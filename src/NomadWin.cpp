/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2020 rpf
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>


#include "NomadWin.hpp"
#include "NomadApp.hpp"
#include "Capture.hpp"
#include "TextShape.hpp"
#include "CairoShape.hpp"
#ifdef __WIN32__
#include "WinCapture.hpp"
#include "ScanDlg.hpp"
#else
#include "X11Capture.hpp"
#endif

NomadFileChooser::NomadFileChooser(
        Gtk::Window& win,
        bool save,
        const std::vector<Glib::ustring>& types)
: Gtk::FileChooserDialog(win
                        , ""
                        , save
                        ? Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE
                        : Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN
                        , Gtk::DIALOG_MODAL | Gtk::DIALOG_DESTROY_WITH_PARENT)
{
    add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    add_button(save
                ? "_Save"
                : "_Open", Gtk::RESPONSE_ACCEPT);
    Glib::ustring allTypes;
    for (auto type : types) {
        if (!allTypes.empty()) {
            allTypes += ", ";
        }
        allTypes += type;
    }
    set_title(save
              ? Glib::ustring::sprintf("Save %s-file(s)", allTypes)
              : Glib::ustring::sprintf("Open %s-file(s)", allTypes));

    Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
    filter->set_name("Type");
    for (auto type : types) {
        filter->add_pattern(Glib::ustring::sprintf("*.%s", type));
    }
    set_filter(filter);
}

NomadWin::NomadWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadApp *application)
: Gtk::ApplicationWindow(cobject)       //Calls the base class constructor
, m_application{application}
, m_treeView{nullptr}
, m_preview{nullptr}
, m_buttons{nullptr}
{
    set_title("Nomad");
    auto pix = Gdk::Pixbuf::create_from_resource(m_application->get_resource_base_path() + "/nomad.png");
    set_icon(pix);
    activate_actions();

    builder->get_widget_derived("tree_view", m_treeView, this);
    builder->get_widget_derived("preview", m_preview, this);
    builder->get_widget("buttons", m_buttons);
    create_buttons();

    show_all_children();
}

bool
NomadWin::ask_text(TextInfo& textInfo)
{
	bool ret = false;
	auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(m_application->get_resource_base_path() + "/text-dlg.ui");
		Gtk::Dialog* dlg;
        builder->get_widget("dlg", dlg);
		Gtk::Entry* text;
        builder->get_widget("text", text);
        text->set_text(textInfo.getText());
        Gtk::ColorButton* color;
        builder->get_widget("color", color);
        color->set_color(m_config->getForegroundColor());
        Gtk::FontButton *font;
        builder->get_widget("font", font);
        font->set_font_name(m_config->getTextFont());
	    int result = dlg->run();
		switch (result) {
			case Gtk::RESPONSE_OK:
				textInfo.setText(text->get_text());
                textInfo.setColor(color->get_color());
                textInfo.setFont(font->get_font_name());
                m_config->setForegroundColor(color->get_color());
                m_config->setTextFont(font->get_font_name());
                ret = true;
				break;
			default:
				break;
		}
		delete dlg;
    }
    catch (const Glib::Error &ex) {
        show_error(Glib::ustring::sprintf("Unable to load name-dlg: %s",  ex.what()));
    }
	return ret;
}

bool
NomadWin::ask_size(std::array<int,2>& size, Gdk::Color& background)
{
    bool ret = false;
	auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(m_application->get_resource_base_path() + "/size-dlg.ui");
		Gtk::Dialog* dlg;
        builder->get_widget("dlg", dlg);
		Gtk::SpinButton* width;
        builder->get_widget("width", width);
        width->set_value(size[0]);
		Gtk::SpinButton* height;
        builder->get_widget("height", height);
        height->set_value(size[1]);
        Gtk::ColorButton * color;
        builder->get_widget("color", color);
        color->set_color(background);
	    int result = dlg->run();
		switch (result) {
			case Gtk::RESPONSE_OK:
                size[0] = static_cast<int>(width->get_value());
                size[1] = static_cast<int>(height->get_value());
                background = color->get_color();
                ret = true;
				break;
			default:
				break;
		}
		delete dlg;
    }
    catch (const Glib::Error &ex) {
        show_error(Glib::ustring::sprintf("Unable to load size-dlg: %s",  ex.what()));
    }
    return ret;
}

bool
NomadWin::timeout()
{
    try {
#ifdef __WIN32__
        WinCapture capture;
#else
        X11Capture capture;
#endif
        //GdkRectangle re(0,0, 1920,1080);
        capture.set_take_window_shot(true);
        GdkPixbuf* buf = capture.get_pixbuf(nullptr);
        if (buf) {
            Glib::RefPtr<Gdk::Pixbuf> pixbuf = Glib::wrap(buf);
            m_preview->setPixbuf(pixbuf);
        }
        else {
            std::cout << "Capture failed!" << std::endl;
        }
    }
	catch (const Glib::Error &ex) {
		show_error(Glib::ustring::sprintf("Unable to capture %s", ex.what()));
	}
    return false;   // do not repeat
}

void
NomadWin::create_buttons()
{
    auto btn_text = Gtk::make_managed<Gtk::Button>();
    //auto theme = Gtk::IconTheme::get_default();
    //auto info = theme->lookup_icon("gtk-edit", Gtk::ICON_SIZE_BUTTON);    // new name "GTK_STOCK_EDIT"? not understood
    //auto icon = Gtk::make_managed<Gtk::Image>(info.load_icon());
    //btn_text->set_image(*icon);
    btn_text->set_image_from_icon_name("gtk-edit");
    btn_text->signal_clicked().connect([this] () {
        TextInfo text;
        if (ask_text(text)) {
            m_preview->addText(text);
        }
    });
    m_buttons->add(*btn_text);
    auto btn_arrow = Gtk::make_managed<Gtk::Button>();
    btn_arrow->set_image_from_icon_name("gtk-go-up");
    btn_arrow->signal_clicked().connect([this] () {
        auto arrow = std::make_shared<CairoShape>("S 24 24 W 2 M 7 7.5 L 12 2.5 L 17 7.5 M 12 21.3 L 12 4.8");
        arrow->setScale(0.1);
        m_preview->add(arrow);
    });
    m_buttons->add(*btn_arrow);
    auto btn_load = Gtk::make_managed<Gtk::Button>();
    btn_load->set_image_from_icon_name("gtk-directory");
    btn_load->signal_clicked().connect([this] () {
        try {
            NomadFileChooser file_chooser(*this, false, {"svg"});
            if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                //std::string home = Glib::get_home_dir();
                //Glib::ustring fullPath = Glib::canonicalize_filename("Downloads/arrow-up-svgrepo-com.svg", home.c_str());
                //Glib::filename_from_utf8(fullPath);
                m_preview->loadSvg(file_chooser.get_file());
            }
        }
        catch (const Glib::Error &ex) {
            show_error(Glib::ustring::sprintf("Unable load file %s", ex.what()));
        }
    });
    m_buttons->add(*btn_load);
}

void
NomadWin::activate_actions()
{
    m_config = std::make_shared<Config>();
    signal_hide().connect(
        [this] {
            m_config->save_config();
        });
    auto capture_action = Gio::SimpleAction::create("capture");
    capture_action->signal_activate().connect(
        [this] (const Glib::VariantBase& value)  {
            try {
                if (m_timer.connected()) {
                    m_timer.disconnect(); // kill previous
                }
                m_timer = Glib::signal_timeout().connect_seconds(
                    sigc::mem_fun(*this, &NomadWin::timeout), m_config->getDelay());
            }
            catch (const Glib::Error &ex) {
                show_error(Glib::ustring::sprintf("Unable to start timer %s", ex.what()));
            }
		});
    add_action(capture_action);
    #ifdef __WIN32__
    auto scan_action = Gio::SimpleAction::create("scan");
    scan_action->signal_activate().connect(
        [this] (const Glib::VariantBase& value)  {
            auto builder = Gtk::Builder::create();
               try {
                   builder->add_from_resource(m_application->get_resource_base_path() + "/scan-dlg.ui");
                   ScanDlg* dialog = nullptr;
                   builder->get_widget_derived("scan-dlg", dialog, this);
                   dialog->show_all();
                   dialog->run();
                   dialog->hide();
               }
               catch (const Glib::Error &ex) {
                   std::cerr << "Unable to load scan-dialog: " << ex.what() << std::endl;
               }
		});
    add_action(scan_action);
    #endif
    auto captureWindow_action = Gio::SimpleAction::create_bool("captureWindow", m_config->isCaptureWindow());
    captureWindow_action->signal_change_state().connect (
        [this,captureWindow_action] (const Glib::VariantBase& value)  {
            auto boolValue = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(value);
            //std::cout << "NomadWin::activate_actions "
            //          << " type " << (boolValue ? (boolValue.get() ? "y" : "n") : "?") << std::endl;
            if (boolValue) {
                m_config->setCaptureWindow(boolValue.get());
                //std::cout << "change_state" << std::endl;
                captureWindow_action->set_state(boolValue);
            }
            else {
                std::cout << "NomadWin::activate_actions cannot extract value from capture window action!" << std::endl;
            }
        });
    add_action(captureWindow_action);
    // the last finesse (using create_radio_integer) doesn't work (all will be disabled)
    auto delay_action = Gio::SimpleAction::create_radio_string("delay", Glib::ustring::sprintf("%d", m_config->getDelay()));
    delay_action->signal_change_state().connect (
        [this,delay_action] (const Glib::VariantBase& value)  {
            if (value) {
                auto strValue = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(value);
                int intValue = std::stoi(strValue.get());
                m_config->setDelay(intValue);
                delay_action->set_state(value);
            }
            else {
                std::cout << "NomadWin::activate_actions cannot extract delay !" << std::endl;
            }
		});
    add_action(delay_action);

    auto new_action = Gio::SimpleAction::create("new");
    new_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
            try {
                std::array<int,2> size{800, 600};
                auto background = m_config->getBackgroundColor();
                if (ask_size(size, background)) {
                    m_preview->create(size, background);
                }
            }
            catch (const Glib::Error &ex) {
                show_error(Glib::ustring::sprintf("Unable save file %s", ex.what()));
            }
        });
    add_action(new_action);

    auto save_action = Gio::SimpleAction::create("save");
    save_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
            try {
                if (m_preview->getPixbuf()) {
                    NomadFileChooser file_chooser(*this, true, {"png", "jpg"});
                    if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                        if (!m_preview->saveImage(file_chooser.get_filename())) {
                            show_error(Glib::ustring::sprintf("Unable to save file %s", file_chooser.get_filename()));
                        }
                    }
                }
            }
            catch (const Glib::Error &ex) {
                show_error(Glib::ustring::sprintf("Unable save file %s", ex.what()));
            }
        });
    add_action(save_action);
    auto load_action = Gio::SimpleAction::create("load");
    load_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
            try {
                NomadFileChooser file_chooser(*this, false, {"png", "jpg"});
                if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                    try {
                        m_preview->loadImage(file_chooser.get_file());
                    }
                    catch (const Glib::Error &ex) {
                        show_error(Glib::ustring::sprintf("Error %s loading %s", ex.what(), file_chooser.get_filename()));
                    }
                }
            }
            catch (const Glib::Error &ex) {
                show_error(Glib::ustring::sprintf("Unable load file %s", ex.what()));
            }
        });
    add_action(load_action);
}


void
NomadWin::on_hide()
{
    //save_config();
    Gtk::Window::on_hide();
}

void
NomadWin::show_error(Glib::ustring msg)
{
    // this shoud automatically give some context
    g_warning("show_error %s", msg.c_str());
    Gtk::MessageDialog messagedialog(*this, msg, FALSE, Gtk::MessageType::MESSAGE_WARNING);

    messagedialog.run();
    messagedialog.hide();
}

