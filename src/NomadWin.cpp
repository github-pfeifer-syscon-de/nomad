/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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
#ifdef __WIN32__
#include "WinCapture.hpp"
#else
#include "X11Capture.hpp"
#endif
/*
 * slightly customized file chooser
 */
class NomadFileChooser : public Gtk::FileChooserDialog {
public:
    NomadFileChooser(Gtk::Window& win, bool save, const Glib::ustring& type)
    : Gtk::FileChooserDialog(win
                            , save
                            ? Glib::ustring::sprintf("Save %s-file", type)
                            : Glib::ustring::sprintf("Open %s-file", type)
                            , save
                            ? Gtk::FileChooserAction::FILE_CHOOSER_ACTION_SAVE
                            : Gtk::FileChooserAction::FILE_CHOOSER_ACTION_OPEN
                            , Gtk::DIALOG_MODAL | Gtk::DIALOG_DESTROY_WITH_PARENT)
    {
        add_button("_Cancel", Gtk::RESPONSE_CANCEL);
        add_button(save
                    ? "_Save"
                    : "_Open", Gtk::RESPONSE_ACCEPT);

        Glib::RefPtr<Gtk::FileFilter> filter = Gtk::FileFilter::create();
        filter->set_name("Type");
        //filter->add_mime_type("text/plain");
        filter->add_pattern(Glib::ustring::sprintf("*.%s", type));
        set_filter(filter);
    }

    virtual ~NomadFileChooser() = default;
protected:
private:
};

NomadWin::NomadWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadApp *application)
: Gtk::ApplicationWindow(cobject)       //Calls the base class constructor
, m_application{application}
, m_treeView{nullptr}
, m_preview{nullptr}
{
    set_title("Nomad");
    auto pix = Gdk::Pixbuf::create_from_resource(m_application->get_resource_base_path() + "/nomad.png");
    set_icon(pix);
    activate_actions();

    builder->get_widget_derived("tree_view", m_treeView, this);
    builder->get_widget_derived("preview", m_preview, this);

    show_all_children();
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
        GdkRectangle re(0,0, 1920,1080);
        capture.set_take_window_shot(true);
        GdkPixbuf* buf = capture.get_pixbuf(nullptr);    // &re
        if (buf) {
            Glib::RefPtr<Gdk::Pixbuf> pixbuf = Glib::wrap(buf);
            m_preview->setPixbuf(pixbuf);
			NomadFileChooser file_chooser(*this, true, "png");
			if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                pixbuf->save(file_chooser.get_filename(), "png");
			}
        }
        else {
            std::cout << "Capture failed!" << std::endl;
        }
    }
	catch (const Glib::Error &ex) {
		show_error(Glib::ustring::sprintf("Unable to capture and save file %s", ex.what()));
	}
    return false;   // do not repeat
}

void
NomadWin::activate_actions()
{
    auto capture10_action = Gio::SimpleAction::create("capture10");
    capture10_action->signal_activate().connect (
        [this] (const Glib::VariantBase& value)  {
			try {
                if (m_timer.connected()) {
                    m_timer.disconnect(); // kill previous
                }
                m_timer = Glib::signal_timeout().connect_seconds(
                    sigc::mem_fun(*this, &NomadWin::timeout), 10);
			}
			catch (const Glib::Error &ex) {
				show_error(Glib::ustring::sprintf("Unable to start timer %s", ex.what()));
			}
		});
    add_action(capture10_action);
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

