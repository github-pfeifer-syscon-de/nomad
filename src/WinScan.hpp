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

#include <vector>
#include <memory>
#include <windows.h>
#include <wia.h>

#include "CWiaDataCallback.hpp"

class WinScan;

class WiaDevice {
public:
    WiaDevice(WinScan* winScan, const BSTR& devId);
    explicit WiaDevice(const WiaDevice& orig) = delete;
    virtual ~WiaDevice();

    bool scan(CWiaDataCallback *pCallback);
protected:
    HRESULT createWiaDevice( IWiaDevMgr *pWiaDevMgr, BSTR bstrDeviceID );
    HRESULT enumerateItems( IWiaItem *pWiaItem, CWiaDataCallback *pCallback );
    HRESULT transferWiaItem( IWiaItem *pWiaItem, bool trnsfFile, CWiaDataCallback *pCallback);

private:
    WinScan* m_winScan;
    IWiaItem* m_pWiaDevice;
};

class WinScan {
public:
    WinScan();
    explicit WinScan(const WinScan& orig) = delete;
    WinScan& operator=(const WinScan& other) = delete;
    virtual ~WinScan();
    HRESULT enumerateWiaDevices();
    HRESULT readSomeWiaProperties(IWiaPropertyStorage *pWiaPropertyStorage);

    IWiaDevMgr* getWiaDevMgr() {
        return m_pWiaDevMgr;
    }
    std::vector<std::shared_ptr<WiaDevice>> getDevices() {
        return m_devices;
    }
private:
    std::vector<std::shared_ptr<WiaDevice>> m_devices;
    IWiaDevMgr* m_pWiaDevMgr{nullptr};

};

