/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
// use local wia.h as workaround if the msys2 version is filled in, use <wia.h>
#include "wia.h"

#include "WiaScanPreview.hpp"
#include "ScanDialog.hpp"

class WiaDevice;
class WiaScan;

class WiaScanDlg
: public ScanDialog
{
public:
    WiaScanDlg(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* m_nomadWin);
    explicit WiaScanDlg(const WiaScanDlg& orig) = delete;
    virtual ~WiaScanDlg();
    void scanCompleted();
protected:
    std::map<uint32_t, int32_t> getProperties(bool full);
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
    WiaScanPreview* m_wiaScanPreview;
    std::shared_ptr<WiaScan> m_wiaScan;
    Glib::Dispatcher m_completed;
};

