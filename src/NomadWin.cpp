/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <ImageFileChooser.hpp>
#include <DisplayImage.hpp>
#include <exception>

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
#include "PenlWindow.hpp"

#undef NOMADWIN_DEBUG

NomadWin::NomadWin(
        BaseObjectType* cobject
        , const Glib::RefPtr<Gtk::Builder>& builder
        , std::shared_ptr<Mode> mode
        , ApplicationSupport& appSupport)
: ImageView(cobject, builder, mode, appSupport, false)
{
    set_title("Nomad");
    auto pix = Gdk::Pixbuf::create_from_resource(m_appSupport.getApplication()->get_resource_base_path() + "/nomad.png");
    set_icon(pix);

    // use our instance of imageview
    Preview* preview;
    builder->get_widget_derived("imageDraw", preview, m_appSupport, this);
    #ifdef NOMADWIN_DEBUG
    std::cout << "NomadWin::createImageWindow preview " << preview << std::endl;
    #endif
    m_content = preview;

    activate_actions();
    //show_all_children();  prefere show_all from app
}

Preview*
NomadWin::getPreview()
{
    Preview* preview = dynamic_cast<Preview*>(m_content);
    if (!preview) {
        std::cout << "m_content could not be cast to preview! " << m_content << std::endl;
        throw std::runtime_error("Not the expected type for m_content!");
    }
    return preview;
}


bool
NomadWin::ask_size(std::array<int,2>& size, Gdk::Color& background)
{
    bool ret = false;
	auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(m_appSupport.getApplication()->get_resource_base_path() + "/size-dlg.ui");
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
        m_appSupport.showError(Glib::ustring::sprintf("Unable to load size-dlg: %s",  ex.what()));
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
            auto displayImage = DisplayImage::create(pixbuf);
            setDisplayImage(displayImage);
        }
        else {
            std::cout << "Capture failed!" << std::endl;
        }
    }
	catch (const Glib::Error &ex) {
		m_appSupport.showError(Glib::ustring::sprintf("Unable to capture %s", ex.what()));
	}
    return false;   // do not repeat
}


Gtk::Menu*
NomadWin::build_popup(int x, int y)
{
    //std::cout << "NomadWin::build_popup" << std::endl;
    Gtk::Menu* menu = ImageView::build_popup(x, y);
    auto mni_text = Gtk::make_managed<Gtk::MenuItem>("_Text", true);
    //auto theme = Gtk::IconTheme::get_default();
    //auto info = theme->lookup_icon("gtk-edit", Gtk::ICON_SIZE_BUTTON);    // new name "GTK_STOCK_EDIT"? not understood
    //auto icon = Gtk::make_managed<Gtk::Image>(info.load_icon());
    //if (icon) {
    //    mni_text->set_image(*icon);
    //}
    //btn_text->set_image_from_icon_name("gtk-edit");
    mni_text->signal_activate().connect([this] () {
        Preview* preview = getPreview();
        TextInfo text;
        text.setColor(m_config->getForegroundColor());
        text.setFont(m_config->getTextFont());
        if (TextShape::ask_text(text, *preview)) {
            m_config->setForegroundColor(text.getColor());
            m_config->setTextFont(text.getFont());
            preview->addText(text);
        }
    });
    menu->append(*mni_text);
    auto mni_arrow = Gtk::make_managed<Gtk::MenuItem>("_Shape", true);
    //mni_arrow->set_image_from_icon_name("gtk-go-up");
    mni_arrow->signal_activate().connect([this] () {
        auto arrow = std::make_shared<CairoShape>("S 24 24 W 2 M 7 7.5 L 12 2.5 L 17 7.5 M 12 21.3 L 12 4.8");
        arrow->setScale(0.1);
        Preview* preview = getPreview();
        preview->add(arrow);
    });
    menu->append(*mni_arrow);
    auto mni_load = Gtk::make_managed<Gtk::MenuItem>("Sv_g", true);
    //btn_load->set_image_from_icon_name("gtk-directory");
    mni_load->signal_activate().connect([this] () {
        try {
            ImageFileChooser file_chooser(*this, false, {"svg"});
            if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                Preview* preview = getPreview();
                preview->loadSvg(file_chooser.get_file());
            }
        }
        catch (const Glib::Error &ex) {
            m_appSupport.showError(Glib::ustring::sprintf("Unable load svg file %s", ex.what()));
        }
    });
    menu->append(*mni_load);
    auto mni_edit = Gtk::make_managed<Gtk::MenuItem>("_Edit", true);
    mni_edit->signal_activate().connect([this,x,y] () {
        try {
            auto preview = getPreview();
            preview->edit(x, y);
        }
        catch (const Glib::Error &ex) {
            m_appSupport.showError(Glib::ustring::sprintf("Unable edit %s", ex.what()));
        }
    });
    menu->append(*mni_edit);

    return menu;
}

void NomadWin::on_hide()
{
    if (m_config) {
        m_config->save_config();
    }
    ImageView::on_hide();   // this will save config
}

void
NomadWin::activate_actions()
{
    auto capture_action = Gio::SimpleAction::create("capture");
    auto config = getConfig();
    capture_action->signal_activate().connect(
        [this,config] (const Glib::VariantBase& value)  {
            try {
                if (m_timer.connected()) {
                    m_timer.disconnect(); // kill previous
                }
                m_timer = Glib::signal_timeout().connect_seconds(
                    sigc::mem_fun(*this, &NomadWin::timeout), config->getDelay());
            }
            catch (const Glib::Error &ex) {
                m_appSupport.showError(Glib::ustring::sprintf("Unable to start timer %s", ex.what()));
            }
		});
    add_action(capture_action);

    #ifdef __WIN32__
    auto scan_action = Gio::SimpleAction::create("scan");
    scan_action->signal_activate().connect(
        [this] (const Glib::VariantBase& value)  {
            auto builder = Gtk::Builder::create();
            try {
                builder->add_from_resource(m_appSupport.getApplication()->get_resource_base_path() + "/scan-dlg.ui");
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
    auto captureWindow_action = Gio::SimpleAction::create_bool("captureWindow", config->isCaptureWindow());
    captureWindow_action->signal_change_state().connect (
        [this,captureWindow_action,config] (const Glib::VariantBase& value)  {
            auto boolValue = Glib::VariantBase::cast_dynamic<Glib::Variant<bool>>(value);
            //std::cout << "NomadWin::activate_actions "
            //          << " type " << (boolValue ? (boolValue.get() ? "y" : "n") : "?") << std::endl;
            if (boolValue) {
                config->setCaptureWindow(boolValue.get());
                //std::cout << "change_state" << std::endl;
                captureWindow_action->set_state(boolValue);
            }
            else {
                std::cout << "NomadWin::activate_actions cannot extract value from capture window action!" << std::endl;
            }
        });
    add_action(captureWindow_action);
    // the last finesse (using create_radio_integer) doesn't work (all will be disabled)
    auto delay_action = Gio::SimpleAction::create_radio_string("delay", Glib::ustring::sprintf("%d", config->getDelay()));
    delay_action->signal_change_state().connect (
        [this,delay_action,config] (const Glib::VariantBase& value)  {
            if (value) {
                auto strValue = Glib::VariantBase::cast_dynamic<Glib::Variant<Glib::ustring>>(value);
                int intValue = std::stoi(strValue.get());
                config->setDelay(intValue);
                delay_action->set_state(value);
            }
            else {
                std::cout << "NomadWin::activate_actions cannot extract delay !" << std::endl;
            }
		});
    add_action(delay_action);

    auto new_action = Gio::SimpleAction::create("new");
    new_action->signal_activate().connect (
        [this,config] (const Glib::VariantBase& value)  {
            try {
                std::array<int,2> size{800, 600};
                auto background = config->getBackgroundColor();
                if (ask_size(size, background)) {
                    Preview* preview = getPreview();
                    preview->create(size, background);
                }
            }
            catch (const Glib::Error &ex) {
                m_appSupport.showError(Glib::ustring::sprintf("Unable save file %s", ex.what()));
            }
        });
    add_action(new_action);

    auto save_action = Gio::SimpleAction::create("save");
    save_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
            try {
                ImageFileChooser file_chooser(*this, true, {"png", "jpg"});
                if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                    Preview* preview = getPreview();
                    if (!preview->saveImage(file_chooser.get_filename())) {
                        m_appSupport.showError(Glib::ustring::sprintf("Unable to save file %s", file_chooser.get_filename()));
                    }
                }
            }
            catch (const Glib::Error &ex) {
                m_appSupport.showError(Glib::ustring::sprintf("Unable save file %s", ex.what()));
            }
        });
    add_action(save_action);
    auto load_action = Gio::SimpleAction::create("load");
    load_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
            try {
                ImageFileChooser file_chooser(*this, false, {"png", "jpg"});
                if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                    try {
                        Preview* preview = getPreview();
                        preview->loadImage(file_chooser.get_file());
                    }
                    catch (const Glib::Error &ex) {
                        m_appSupport.showError(Glib::ustring::sprintf("Error %s loading %s", ex.what(), file_chooser.get_filename()));
                    }
                }
            }
            catch (const Glib::Error &ex) {
                m_appSupport.showError(Glib::ustring::sprintf("Unable load file %s", ex.what()));
            }
        });
    add_action(load_action);

    auto draw_action = Gio::SimpleAction::create("draw");
    draw_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
            auto builder = Gtk::Builder::create();
            try {
                builder->add_from_resource(m_appSupport.getApplication()->get_resource_base_path() + "/penl-win.ui");
                PenlWindow* window = nullptr;
                builder->get_widget_derived("penlWin", window, m_appSupport.getApplication());
                window->set_transient_for(*this);
                window->set_modal(true);
                window->show_all();
            }
            catch (const Glib::Error &ex) {
                m_appSupport.showError(Glib::ustring::sprintf("Unable open draw %s", ex.what()));
            }
        });
    add_action(draw_action);

}

std::shared_ptr<Config> NomadWin::getConfig()
{
    if (!m_config) {
       m_config = std::make_shared<Config>(m_appSupport.getConfig());
    }
    return m_config;
}