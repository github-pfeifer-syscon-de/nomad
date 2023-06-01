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

class NomadWin;

class NomadTreeView : public Gtk::TreeView {
public:
    NomadTreeView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* calcppWin);
    virtual ~NomadTreeView() = default;

    void value_cell_data_func(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter, int model_column);

private:

    void create_column_name(Glib::ustring name, Gtk::TreeModelColumn<Glib::ustring>& col);

    void create_column_value(Glib::ustring name, Gtk::TreeModelColumn<double>& col);

    NomadWin* m_nomadWin;
};

