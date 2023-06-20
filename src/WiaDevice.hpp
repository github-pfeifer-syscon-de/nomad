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
class ScanPreview;

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
    static constexpr auto property_bits = 4104u;
    static constexpr auto property_resolution_x = 6147u;
    static constexpr auto property_resolution_y = 6148u;
    static constexpr auto property_start_x = 6149u;
    static constexpr auto property_start_y = 6150u;
    static constexpr auto property_extend_x = 6151u;
    static constexpr auto property_extend_y = 6152u;
    static constexpr auto property_brightness = 6154u;
    static constexpr auto property_contrast = 6155u;
    static constexpr auto property_threshold = 6159u;
    void readExtends(IWiaPropertyStorage *pWiaPropertyStorage);
    std::map<uint32_t, WiaValue> buildScanProperties(
            bool full
            , int32_t bright
            , int32_t contr
            , int32_t tresh
            , int32_t res
            , bool color
            , double xRelStart
            , double yRelStart
            , double xRelEnd
            , double yRelEnd);  // arkward but wia and gtkmm will never be best friends ;(

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
    std::vector<WiaValue> m_startX;
    std::vector<WiaValue> m_startY;
    std::vector<WiaValue> m_extendX;
    std::vector<WiaValue> m_extendY;
};
