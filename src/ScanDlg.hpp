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

#pragma once

#include <gtkmm.h>
#include <wia.h>

#include "ScanPreview.hpp"

class WiaDevice;
class WiaScan;

class ScanDlg
: public Gtk::Dialog
{
public:
    ScanDlg(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* m_nomadWin);
    explicit ScanDlg(const ScanDlg& orig) = delete;
    virtual ~ScanDlg() = default;
protected:
    std::map<uint32_t, WiaValue> getProperties(bool full);
    Glib::ustring getDeviceId();
    std::shared_ptr<WiaDevice> getActiveDevice();
    void setupScale(
        const std::shared_ptr<WiaDevice>& activeDev
        , IWiaPropertyStorage *pWiaPropertyStorage
        , uint32_t propertyId
        , Gtk::Scale* scale);
    void setupSpinner(
        const std::shared_ptr<WiaDevice>& activeDev
        , IWiaPropertyStorage *pWiaPropertyStorage
        , uint32_t propertyId
        , Gtk::SpinButton* scale);
protected:
    void deviceChanged();
private:
    NomadWin* m_nomadWin;
    ScanPreview* m_scanPreview;
    Gtk::FlowBox* m_flow;
    Gtk::Toolbar* m_toolbar;
    Gtk::ComboBoxText *m_device;
    Gtk::RadioButton::Group  m_action_color_depth;
    Gtk::RadioButton* m_radioColor;
    Gtk::RadioButton* m_radioGray;
    Gtk::RadioButton* m_radioBW;
    Gtk::Scale* m_brightness;
    Gtk::Scale* m_contrast;
    Gtk::Scale* m_threshold;
    Gtk::SpinButton* m_resolution;
    std::shared_ptr<WiaScan> m_wiaScan;
};

