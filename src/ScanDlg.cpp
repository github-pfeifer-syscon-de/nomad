/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2023 rpf
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
#include <exception>

#include "ScanDlg.hpp"

ScanDlg::ScanDlg(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Gtk::Application *application)
: Gtk::Dialog(cobject)
{
    auto icon = Gdk::Pixbuf::create_from_resource(application->get_resource_base_path() + "/nomad.png");
    set_icon(icon);

    Gtk::Button* scan = nullptr;
    builder->get_widget<Gtk::Button>("scan", scan);
    if (scan) {
    scan->signal_clicked().connect(
        [this] () {
            try {
                m_scanPreview->scan();
            }
            catch (const Glib::Exception &ex) {
                std::cerr << "Unable to activate scan " << ex.what() << std::endl;
            }
        });
    }
    builder->get_widget("flow", m_flow);
    builder->get_widget_derived("preview", m_scanPreview);
}

