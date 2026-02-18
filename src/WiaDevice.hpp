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

#include <glibmm.h>
#include <windows.h>
#include <list>
#include <map>
#include <memory>

// use local wia.h as workaround if the msys2/wine version has been filled in, use <wia.h>
#include "wia.h"
#include "WiaDataCallback.hpp"

class WiaValue;
class WiaValue2;
class WiaProperty;
class WiaScan;
class ScanPreview;

class WiaDevice
{
public:
    WiaDevice(WiaScan* winScan, const BSTR& devId, const BSTR& devName, const BSTR& devDescr);
    explicit WiaDevice(const WiaDevice& orig) = delete;
    virtual ~WiaDevice();

    bool scan(WiaDataCallback *pCallback, const std::map<uint32_t, int32_t>& properties);
    std::list<std::shared_ptr<WiaProperty>> getProperties();
    Glib::ustring getDeviceId();
    Glib::ustring getDeviceName();
    HRESULT findItem(IWiaItem2 *pWiaItem, LONG typeMask, IWiaItem2** pRetChildWiaItem);
    IWiaItem2* getWiaItem();
    static constexpr auto PropertyResolutionX = 6147u;
    static constexpr auto PropertyResolutionY = 6148u;
    static constexpr auto PropertyStartX = 6149u;
    static constexpr auto PropertyStartY = 6150u;
    static constexpr auto PropertyExtendX = 6151u;
    static constexpr auto PropertyExtendY = 6152u;
    static constexpr auto PropertyBrightness = 6154u;
    static constexpr auto PropertyContrast = 6155u;
    static constexpr auto PropertyThreshold = 6159u;
    void readExtends(IWiaPropertyStorage *pWiaPropertyStorage);
    std::map<uint32_t, int32_t> buildScanProperties(
            bool full
            , int32_t bright
            , int32_t contr
            , int32_t tresh
            , int32_t res
            , int32_t bits
            , double xRelStart
            , double yRelStart
            , double xRelEnd
            , double yRelEnd);  // arkward but wia and gtkmm will never be best friends ;(
    HRESULT setProperties(IWiaItem2 *pWiaItem, bool trnsfFile, const std::map<uint32_t, int32_t>& properties);

protected:
    HRESULT createWiaDevice( IWiaDevMgr2 *pWiaDevMgr, BSTR bstrDeviceID );
    HRESULT transferWiaItem(IWiaItem2 *pWiaItem, bool trnsfFile, WiaDataCallback *pCallback, const std::map<uint32_t, int32_t>& properties);
    void getProperties(IWiaPropertyStorage *pWiaPropertyStorage, const std::string& section);
    int32_t convertX4DPI(int32_t y1, int32_t dpi);
    int32_t convertY4DPI(int32_t y1, int32_t dpi);
private:
    WiaScan* m_winScan;
    IWiaItem2* m_pWiaDevice;
    Glib::ustring m_devId;
    Glib::ustring m_devName;
    Glib::ustring m_devDescr;
    std::list<std::shared_ptr<WiaProperty>> m_properties;
    std::vector<WiaValue2> m_startX;
    std::vector<WiaValue2> m_startY;
    std::vector<WiaValue2> m_extendX;
    std::vector<WiaValue2> m_extendY;
    int32_t horzResolution{300};
    int32_t vertResolution{300};
};
