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

#include <glibmm.h>
#include <windows.h>
#include <wia.h>
#include <list>
#include <map>
#include <memory>

#include "WiaDataCallback.hpp"

class WiaValue;
class WiaProperty;
class WiaScan;

class WiaDevice
{
public:
    WiaDevice(WiaScan* winScan, const BSTR& devId, const BSTR& devName, const BSTR& devDescr);
    explicit WiaDevice(const WiaDevice& orig) = delete;
    virtual ~WiaDevice();

    bool scan(WiaDataCallback *pCallback, std::map<uint32_t, WiaValue> properties);
    std::list<std::shared_ptr<WiaProperty>> getProperties();
    Glib::ustring getDeviceId();
    Glib::ustring getDeviceName();
    HRESULT findItem(IWiaItem *pWiaItem, LONG typeMask, IWiaItem** pRetChildWiaItem);
    IWiaItem* getWiaItem();
protected:
    HRESULT createWiaDevice( IWiaDevMgr *pWiaDevMgr, BSTR bstrDeviceID );
    HRESULT transferWiaItem(IWiaItem *pWiaItem, bool trnsfFile, WiaDataCallback *pCallback, std::map<uint32_t, WiaValue> properties);
    void getProperties(IWiaPropertyStorage *pWiaPropertyStorage, const Glib::ustring& section);
private:
    WiaScan* m_winScan;
    IWiaItem* m_pWiaDevice;
    Glib::ustring m_devId;
    Glib::ustring m_devName;
    Glib::ustring m_devDescr;
    std::list<std::shared_ptr<WiaProperty>> m_properties;
};
