/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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
class ScanPreview;

class ScanDialog
: public Gtk::Dialog
{
public:
    ScanDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin);
    explicit ScanDialog(const ScanDialog &other) = delete;
    virtual ~ScanDialog() = default;

protected:
    void exportPdf(const Glib::ustring& file);
    bool saveImage(const Glib::ustring& file);
    void cleanup();
    void appendScanPage();

    NomadWin* m_nomadWin;
    ScanPreview* m_scanPreview{};
    Gtk::ListBox* m_list;
    Gtk::Toolbar* m_toolbar;
    Gtk::ComboBoxText *m_device;
    Gtk::RadioButton::Group  m_action_color_depth;
    Gtk::RadioButton* m_radioColor;
    Gtk::RadioButton* m_radioGray;
    //Gtk::RadioButton* m_radioBW;
    Gtk::Scale* m_brightness;
    Gtk::Scale* m_contrast;
    //Gtk::Scale* m_threshold;
    Gtk::ComboBoxText* m_resolution;

    Gtk::ListBoxRow* m_lastListRow{};
    Gtk::Button* m_preview{};
    Gtk::Button* m_scan{};
    Gtk::Button* m_save{};

    std::list<Glib::RefPtr<Gio::File>> m_pages;


};