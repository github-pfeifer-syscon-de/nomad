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

#include <iostream>
#include <system_error>

#include "WiaScan.hpp"
#include "StringUtils.hpp"


static  HRESULT
createWiaDeviceManager( IWiaDevMgr **ppWiaDevMgr ) //XP or earlier
{
    //
    // Validate arguments
    //
    if (NULL == ppWiaDevMgr) {
        return E_INVALIDARG;
    }

    //
    // Initialize out variables
    //
    *ppWiaDevMgr = NULL;

    //
    // Create an instance of the device manager
    //

    //XP or earlier:
    HRESULT hr = CoCreateInstance(CLSID_WiaDevMgr, NULL, CLSCTX_LOCAL_SERVER, IID_IWiaDevMgr, (void**) ppWiaDevMgr);

    //
    // Return the result of creating the device manager
    //
    return hr;
}


HRESULT
WiaScan::readSomeWiaProperties(IWiaPropertyStorage *pWiaPropertyStorage)
{
    //
    // Validate arguments
    //
    if (NULL == pWiaPropertyStorage) {
        return E_INVALIDARG;
    }

    //
    // Declare PROPSPECs and PROPVARIANTs, and initialize them to zero.
    //
    PROPSPEC PropSpec[3] = {0};
    PROPVARIANT PropVar[3] = {0};

    //
    // How many properties are you querying for?
    //
    const ULONG c_nPropertyCount = sizeof (PropSpec) / sizeof (PropSpec[0]);

    //
    // Define which properties you want to read:
    // Device ID.  This is what you would use to create
    // the device.
    //
    PropSpec[0].ulKind = PRSPEC_PROPID;
    PropSpec[0].propid = WIA_DIP_DEV_ID;

    //
    // Device Name
    //
    PropSpec[1].ulKind = PRSPEC_PROPID;
    PropSpec[1].propid = WIA_DIP_DEV_NAME;

    //
    // Device description
    //
    PropSpec[2].ulKind = PRSPEC_PROPID;
    PropSpec[2].propid = WIA_DIP_DEV_DESC;

    //
    // Ask for the property values
    //
    HRESULT hr = pWiaPropertyStorage->ReadMultiple(c_nPropertyCount, PropSpec, PropVar);
    if (SUCCEEDED(hr)) {
        //
        // IWiaPropertyStorage::ReadMultiple will return S_FALSE if some
        // properties could not be read, so you have to check the return
        // types for each requested item.
        //

        BSTR devId;
        BSTR devName;
        BSTR devDescr;
        //
        // Check the return type for the device ID
        //
        if (VT_BSTR == PropVar[0].vt) {
            //
            // Do something with the device ID
            //
            std::cout << "WIA_DIP_DEV_ID: " << StringUtils::utf8_encode(PropVar[0].bstrVal) << std::endl;

            devId = PropVar[0].bstrVal;
        }

        //
        // Check the return type for the device name
        //
        if (VT_BSTR == PropVar[1].vt) {
            //
            // Do something with the device name
            //
            //std::cout << "WIA_DIP_DEV_NAME: " << StringUtils::utf8_encode(PropVar[1].bstrVal) << std::endl;
            devName= PropVar[1].bstrVal;
        }

        //
        // Check the return type for the device description
        //
        if (VT_BSTR == PropVar[2].vt) {
            //
            // Do something with the device description
            //
            //std::cout << "WIA_DIP_DEV_DESC: " << StringUtils::utf8_encode(PropVar[2].bstrVal) << std::endl;
            devDescr = PropVar[2].bstrVal;
        }
        auto dev = std::make_shared<WiaDevice>(this, devId, devName, devDescr);
        m_devices.push_back(dev);

        //
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray(c_nPropertyCount, PropVar);
    }

    //
    // Return the result of reading the properties
    //
    return hr;
}


HRESULT
WiaScan::enumerateWiaDevices() //XP or earlier
{
    //
    // Validate arguments
    //
    if (NULL == m_pWiaDevMgr) {
        return E_INVALIDARG;
    }

    //
    // Get a device enumerator interface
    //
    IEnumWIA_DEV_INFO *pWiaEnumDevInfo = NULL;
    HRESULT hr = m_pWiaDevMgr->EnumDeviceInfo(WIA_DEVINFO_ENUM_LOCAL, &pWiaEnumDevInfo);
    if (SUCCEEDED(hr)) {
        //
        // Loop until you get an error or pWiaEnumDevInfo->Next returns
        // S_FALSE to signal the end of the list.
        //
        while (S_OK == hr) {
            //
            // Get the next device's property storage interface pointer
            //
            IWiaPropertyStorage *pWiaPropertyStorage = NULL;
            hr = pWiaEnumDevInfo->Next(1, &pWiaPropertyStorage, NULL);

            //
            // pWiaEnumDevInfo->Next will return S_FALSE when the list is
            // exhausted, so check for S_OK before using the returned
            // value.
            //
            if (hr == S_OK) {
                //
                // Do something with the device's IWiaPropertyStorage*
                //
                readSomeWiaProperties(pWiaPropertyStorage);


                //
                // Release the device's IWiaPropertyStorage*
                //
                pWiaPropertyStorage->Release();
                pWiaPropertyStorage = NULL;
            }
        }

        //
        // If the result of the enumeration is S_FALSE (which
        // is normal), change it to S_OK.
        //
        if (S_FALSE == hr) {
            hr = S_OK;
        }

        //
        // Release the enumerator
        //
        pWiaEnumDevInfo->Release();
        pWiaEnumDevInfo = NULL;
    }

    //
    // Return the result of the enumeration
    //
    return hr;
}

WiaDevice::WiaDevice(WiaScan* winScan, const BSTR& devId, const BSTR& devName, const BSTR& devDescr)
: m_winScan{winScan}
, m_pWiaDevice{nullptr}
, m_devName{StringUtils::utf8_encode(devName)}
, m_devDescr{StringUtils::utf8_encode(devDescr)}
{
    HRESULT hr = createWiaDevice(winScan->getWiaDevMgr(), devId);
    if (SUCCEEDED(hr)) {

    }
    else {
        std::string message = std::system_category().message(hr);
        std::cout << "Error " << hr
                  << " createWiaDevice " << message << std::endl;
    }
}

WiaDevice::~WiaDevice()
{
    if (m_pWiaDevice) {
       m_pWiaDevice->Release();
       m_pWiaDevice = nullptr;
    }
}

bool
WiaDevice::scan(WiaDataCallback *pCallback)
{

    if (m_pWiaDevice) {
        HRESULT hr = enumerateItems(m_pWiaDevice, pCallback);
        if (SUCCEEDED(hr)) {
            return true;
        }
        else {
            std::string message = std::system_category().message(hr);
            std::cout << "Error " << hr
                      << " enumerateItems " << message << std::endl;
        }
    }
    return false;
}

WiaScan::WiaScan()
{
    HRESULT hr = createWiaDeviceManager(&m_pWiaDevMgr);
    if (SUCCEEDED(hr)) {
        hr = enumerateWiaDevices();
        if (hr != S_OK) {
            std::string message = std::system_category().message(hr);
            std::cout << "Error " << hr
                      << " EnumerateWiaDevices " << message << std::endl;
        }
    }
    else {
        std::string message = std::system_category().message(hr);
        std::cout << "Error " << hr
                  << " CreateWiaDeviceManager " << message << std::endl;
    }
}

WiaScan::~WiaScan()
{
    if (m_pWiaDevMgr) {
        m_pWiaDevMgr->Release();
        m_pWiaDevMgr = nullptr;
    }
}


