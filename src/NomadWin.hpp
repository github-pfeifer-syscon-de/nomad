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

#pragma once

#include <gtkmm.h>

#include "NomadTreeView.hpp"

class NomadApp;

class NomadWin : public Gtk::ApplicationWindow {
public:
    NomadWin(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refBuilder, NomadApp *appl);
    virtual ~NomadWin() = default;

    void on_hide() override;
    void show_error(Glib::ustring msg);
    void eval(Glib::ustring text, Gtk::TextIter& end);
    void apply_font(bool defaultFont);
protected:
private:
    void build_menu();
    void activate_actions();

    NomadApp *m_application;
    NomadTreeView* m_treeView;
};

