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


NomadWin::NomadWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadApp *application)
: Gtk::ApplicationWindow(cobject)       //Calls the base class constructor
, m_application{application}
, m_treeView{nullptr}
{
    set_title("Nomad");
    auto pix = Gdk::Pixbuf::create_from_resource(m_application->get_resource_base_path() + "/nomad.png");
    set_icon(pix);
    activate_actions();

    builder->get_widget_derived("tree_view", m_treeView, this);

    X11Capture be;
    GdkRectangle re(0,0, 1920,1080);
    GdkPixbuf* buf = be.get_pixbuf(nullptr);    // &re
    if (buf) {
        Glib::RefPtr<Gdk::Pixbuf> pbuf = Glib::wrap(buf);
        pbuf->save("/home/rpf/screen.png", "png");
    }

    show_all_children();
}



void
NomadWin::activate_actions()
{


//    auto save_action = Gio::SimpleAction::create("save");
//    save_action->signal_activate().connect (
//        [this]  (const Glib::VariantBase& value)
//		{
//			try {
//				CalcFileChooser file_chooser(this, true);
//				if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
//					Glib::ustring text = m_textView->get_buffer()->get_text();
//					Glib::file_set_contents(file_chooser.get_filename(), text);
//				}
//			}
//			catch (const Glib::Error &ex) {
//				show_error(Glib::ustring::sprintf("Unable to save file %s", ex.what()));
//			}
//		});
//    add_action (save_action);

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

