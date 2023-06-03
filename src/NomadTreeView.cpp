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

#include "Preview.hpp"
#include "NomadTreeView.hpp"
#include "NomadWin.hpp"

NomadTreeView::NomadTreeView(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::TreeView(cobject)
, m_nomadWin{nomadWin}
{
    //create_column_name("Name", m_evalContext->m_variable_columns.m_name);
    //create_column_value("Value", m_evalContext->m_variable_columns.m_value);

    //set_model(m_evalContext->get_list());
	//auto format_proxy = m_evalContext->property_output_format_id();
	//format_proxy.signal_changed().connect(
	//[this] {
		// to rerender, wont work
		//set_model(m_evalContext->get_list());
		//auto col = m_evalContext->m_variable_columns.m_value;
	    //auto cell = get_column(col.index());
	    //cell->property_visible() = false;
	    //cell->property_visible() = true;	// render column with changed properties
	//});
}


void
NomadTreeView::create_column_name(Glib::ustring name, Gtk::TreeModelColumn<Glib::ustring>& col)
{
    append_column_editable(name, col);
    Gtk::CellRendererText* cell = (Gtk::CellRendererText*)get_column_cell_renderer(col.index());
    cell->property_editable() = true;
    cell->signal_edited().connect(
		[this, col](Glib::ustring path, Glib::ustring data) {
			// lamda allows to avoid function hoping
//			Gtk::TreePath tPath(path);
//			auto model = get_model();
//			Gtk::TreeIter iter = model->get_iter(tPath);
//			//std::cout << "Data " << data << std::endl;
//			auto datas = StrUtil::strip(data);	// no ws
//			//std::cout << "Datas " << datas << std::endl;
//			Gtk::TreeModel::Row row = *iter;
//			Glib::ustring varName = row[col];
//			//std::cout << "Got varName" << varName << std::endl;
//			if (datas == "") {
//				m_evalContext->remove(varName);
//			}
//			else {
//				if (!m_evalContext->find(datas, &row)) {
//					m_evalContext->rename(varName, datas);
//				}
//				else {
//					m_nomadWin->show_error(Glib::ustring::sprintf("Name %s already used.", data));
//				}
//			}
		});
}

// as we want a dynamic format, do our own conversion
void
NomadTreeView::value_cell_data_func(Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter, int model_column)
{
	Gtk::CellRendererText* pTextRenderer = dynamic_cast<Gtk::CellRendererText*> (cell);
	if (!pTextRenderer) {
		g_warning("gtkmm: TextView: append_column_numeric() was used with a non-numeric type.");
	}
	else {
		if (iter) {
			//Get the value from the model.
//			Gtk::TreeModel::Row row = *iter;
//			double value;
//			row.get_value(model_column, value);
//
//			//Convert it to a string representation:
//			auto output_format = m_evalContext->get_output_format();
//			auto str = output_format->format(value);
//			pTextRenderer->property_text() = str;
		}
	}
}


void
NomadTreeView::create_column_value(Glib::ustring name, Gtk::TreeModelColumn<double>& col)
{
	// use a default text column as we provide our own conversion from numeric values
    append_column(name, col);	// the return value is usable if you want to count column's, and is no index!
	auto treeColumn = get_column(col.index());
    auto cellRenderer = (Gtk::CellRendererText*)get_column_cell_renderer(col.index());

	Gtk::TreeViewColumn::SlotTreeCellData slot =
		sigc::bind(
			sigc::mem_fun(*this, &NomadTreeView::value_cell_data_func), col.index());
	treeColumn->set_cell_data_func(*cellRenderer, slot);

    cellRenderer->property_editable() = true;
    cellRenderer->signal_edited().connect(
		[this](Glib::ustring path, Glib::ustring data) {	// lamda allows to avoid function hoping
//			Gtk::TreePath tPath(path);
//			auto model = get_model();
//			Gtk::TreeIter iter = model->get_iter(tPath);
//			std::string::size_type end;
//			double val;
//			if (m_evalContext->get_output_format()->parse(data, val, &end)) {	// uniform number conversion handling
//				Gtk::TreeModel::Row row = *iter;
//				Glib::ustring varName = row[m_evalContext->m_variable_columns.m_name];
//				m_evalContext->set_value(varName, val);
//			}
//			else {
//				m_nomadWin->show_error(Glib::ustring::sprintf("not a valid number %s", data));
//			}
		});
}
