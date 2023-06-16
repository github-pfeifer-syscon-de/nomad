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

#include "WinScan.hpp"
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


WiaProperty::WiaProperty(STATPROPSTG& nextWiaPropertyStorage)
{
    if( NULL != nextWiaPropertyStorage.lpwstrName ) {
        m_name = StringUtils::utf8_encode(nextWiaPropertyStorage.lpwstrName);
    }
    m_propid = nextWiaPropertyStorage.propid;
}

Glib::ustring
WiaProperty::info(IWiaPropertyStorage *pWiaPropertyStorage)
{
    PROPVARIANT propvar;
    PropVariantInit( &propvar );
    PROPSPEC propspec;
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = m_propid;

    Glib::ustring ret = " Name = " + m_name + "\n";
    HRESULT hr = pWiaPropertyStorage->ReadMultiple( 1, &propspec, &propvar );
    if( FAILED(hr) ) {
        ret += Glib::strintf("Error %d ReadMultiple ", hr);
    }
    else {
        // Display the property value, type, and so on.
        ret += Glib::ustring::sprintf("   PropID = %d VarType = %s\n"
            ,  m_propid,  convertVarTypeToString( propvar.vt));

        auto value = convertValueToString(propvar);
        ret += "   Value = " + value;
    }
    return ret;
}

// see https://learn.microsoft.com/en-us/windows/win32/stg/enumall-sample
Glib::ustring
WiaProperty::convertVarTypeToString(VARTYPE vt)
{

    // Create a string using the basic type.
    Glib::ustring ret;
    switch( vt & VT_TYPEMASK )
    {
    case VT_EMPTY:
        ret = "VT_EMPTY";
        break;
    case VT_NULL:
        ret  ="VT_NULL";
        break;
    case VT_I2:
        ret = "VT_I2";
        break;
    case VT_I4:
        ret = "VT_I4";
        break;
    case VT_I8:
        ret = "VT_I8";
        break;
    case VT_UI2:
        ret = "VT_UI2";
        break;
    case VT_UI4:
        ret = "VT_UI4";
        break;
    case VT_UI8:
        ret = "VT_UI8";
        break;
    case VT_R4:
        ret = "VT_R4";
        break;
    case VT_R8:
        ret = "VT_R8";
        break;
    case VT_CY:
        ret = "VT_CY";
        break;
    case VT_DATE:
        ret = "VT_DATE";
        break;
    case VT_BSTR:
        ret = "VT_BSTR";
        break;
    case VT_ERROR:
        ret = "VT_ERROR";
        break;
    case VT_BOOL:
        ret = "VT_BOOL";
        break;
    case VT_VARIANT:
        ret = "VT_VARIANT";
        break;
    case VT_DECIMAL:
        ret = "VT_DECIMAL";
        break;
    case VT_I1:
        ret = "VT_I1";
        break;
    case VT_UI1:
        ret = "VT_UI1";
        break;
    case VT_INT:
        ret = "VT_INT";
        break;
    case VT_UINT:
        ret = "VT_UINT";
        break;
    case VT_VOID:
        ret = "VT_VOID";
        break;
    case VT_SAFEARRAY:
        ret = "VT_SAFEARRAY";
        break;
    case VT_USERDEFINED:
        ret = "VT_USERDEFINED";
        break;
    case VT_LPSTR:
        ret = "VT_LPSTR";
        break;
    case VT_LPWSTR:
        ret = "VT_LPWSTR";
        break;
    case VT_RECORD:
        ret = "VT_RECORD";
        break;
    case VT_FILETIME:
        ret = "VT_FILETIME";
        break;
    case VT_BLOB:
        ret = "VT_BLOB";
        break;
    case VT_STREAM:
        ret = "VT_STREAM";
        break;
    case VT_STORAGE:
        ret = "VT_STORAGE";
        break;
    case VT_STREAMED_OBJECT:
        ret = "VT_STREAMED_OBJECT";
        break;
    case VT_STORED_OBJECT:
        ret = "VT_BLOB_OBJECT";
        break;
    case VT_CF:
        ret = "VT_CF";
        break;
    case VT_CLSID:
        ret = "VT_CLSID";
        break;
    default:
        ret = Glib::ustring::sprintf("Unknown (%d)", vt & VT_TYPEMASK );
        break;
    }


    // Add the type modifiers, if present.
    if( vt & VT_VECTOR ) {
        ret += " | VT_VECTOR";
    }
    if( vt & VT_ARRAY ) {
        ret += " | VT_ARRAY";
    }
    if( vt & VT_RESERVED ) {
        ret += " | VT_RESERVED";
    }
    return ret;
}

Glib::ustring
WiaProperty::convertValueToString( const PROPVARIANT &propvar)
{

    // Based on the type, put the value into pwszValue as a string.
    Glib::ustring ret;
    switch( propvar.vt )
    {
    case VT_EMPTY:
        ret = "";
        break;
    case VT_NULL:
        ret = "";
        break;
    case VT_I2:
        ret = Glib::ustring::sprintf( "%04d", (int)propvar.iVal );
        break;
    case VT_I4:
    case VT_INT:
        ret = Glib::ustring::sprintf( "%d", propvar.lVal );
        break;
    case VT_I8:
        ret = Glib::ustring::sprintf( "%ld", propvar.hVal );
        break;
    case VT_UI2:
        ret = Glib::ustring::sprintf( "%04u", propvar.uiVal );
        break;
    case VT_UI4:
    case VT_UINT:
        ret = Glib::ustring::sprintf( "%u", propvar.ulVal );
        break;
    case VT_UI8:
        ret = Glib::ustring::sprintf( "%lu", propvar.uhVal );
        break;
    case VT_R4:
        ret = Glib::ustring::sprintf( "%f", propvar.fltVal );
        break;
    case VT_R8:
        ret = Glib::ustring::sprintf( "%lf", propvar.dblVal );
        break;
    case VT_BSTR:
        ret = Glib::ustring::sprintf( "\"%s\"",
                     StringUtils::utf8_encode(propvar.bstrVal) );
        break;
    case VT_ERROR:
        ret = Glib::ustring::sprintf( "0x%08X", propvar.scode );
        break;
    case VT_BOOL:
        ret = VARIANT_TRUE == propvar.boolVal ? "True" : "False";
        break;
    case VT_I1:
        ret = Glib::ustring::sprintf( "%02d", propvar.cVal );
        break;
    case VT_UI1:
        ret = Glib::ustring::sprintf( "%02u", propvar.bVal );
        break;
    case VT_VOID:
        ret = "";
        break;
    case VT_LPSTR:
        ret = Glib::ustring::sprintf( "\"%s\"", propvar.pszVal );
        break;
    case VT_LPWSTR:
        ret = Glib::ustring::sprintf( "\"%s\"", StringUtils::utf8_encode(propvar.pwszVal) );
        break;
    case VT_FILETIME:
        ret = Glib::ustring::sprintf( "%08x:%08x",
                     propvar.filetime.dwHighDateTime,
                     propvar.filetime.dwLowDateTime );
        break;
    case VT_CLSID:
        WCHAR pwszValue[64];
        pwszValue[0] = L'\0';
        StringFromGUID2( *propvar.puuid, pwszValue, sizeof(pwszValue)/sizeof(pwszValue[0]) );
        ret = StringUtils::utf8_encode(pwszValue);
        break;
    default:
        ret = "...";
        break;
    }
    return ret;
}


HRESULT  //XP or earlier:
WiaDevice::createWiaDevice( IWiaDevMgr *pWiaDevMgr, BSTR bstrDeviceID )
{
    //
    // Validate arguments
    //
    if (NULL == pWiaDevMgr || NULL == bstrDeviceID) {
        return E_INVALIDARG;
    }

    //
    // Initialize out variables
    //
    m_pWiaDevice = NULL;

    //
    // Create the WIA Device
    //
    HRESULT hr = pWiaDevMgr->CreateDevice(bstrDeviceID, &m_pWiaDevice);

    //
    // Return the result of creating the device
    //
    return hr;
}



HRESULT
WiaDevice::transferWiaItem( IWiaItem *pWiaItem, bool trnsfFile, CWiaDataCallback *pCallback)
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem)
    {
        return E_INVALIDARG;
    }

    //
    // Get the IWiaPropertyStorage interface so you can set required properties.
    //
    IWiaPropertyStorage *pWiaPropertyStorage = NULL;
    HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
    if (SUCCEEDED(hr))
    {

        ULONG ulNumProps = 0;
        HRESULT hr = pWiaPropertyStorage->GetCount(&ulNumProps);
        if (SUCCEEDED(hr)) {
            std::cout << "numProps " << ulNumProps << std::endl;
        }
        else {
            std::cout << "Error " << hr << " pWiaPropertyStorage->GetCount" << std::endl;
        }

        IEnumSTATPROPSTG* penum = nullptr;
        hr = pWiaPropertyStorage->Enum(&penum);
        if (SUCCEEDED(hr)) {
            while (S_OK == hr) {
                //
                // Get the next device's property storage interface pointer
                //
                STATPROPSTG nextWiaPropertyStorage{0};
                hr = penum->Next(1, &nextWiaPropertyStorage, NULL);

                //
                // pWiaEnumDevInfo->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
                //
                if (hr == S_OK) {
                    auto property = std::make_shared<WiaProperty>(nextWiaPropertyStorage);
                    m_properties.push_back(property);
                    std::cout << "-------------------------------------------" << std::endl
                              << property->info(pWiaPropertyStorage) << std::endl;
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
            penum->Release();
            penum = NULL;
        }
        else {
            std::cout << "Error " << hr << " pWiaPropertyStorage->Enum" << std::endl;
        }

//        HRESULT hr = pWiaPropertyStorage->ReadPropertyNames();
//        if (SUCCEEDED(hr)) {
//            std::cout << "numProps " << ulNumProps << std::endl;
//        }
//        else {
//            std::cout << "Error " << hr << " pWiaPropertyStorage->ReadPropertyNames" << std::endl;
//        }
        //
        // Prepare PROPSPECs and PROPVARIANTs for setting the
        // media type and format
        //
        PROPSPEC PropSpec[2] = {0};
        PROPVARIANT PropVariant[2] = {0};
        const ULONG c_nPropCount = sizeof(PropVariant)/sizeof(PropVariant[0]);

        //
        // Use BMP as the output format
        //
        GUID guidOutputFormat = trnsfFile ? WiaImgFmt_BMP : WiaImgFmt_MEMORYBMP;    // seems to have not much influence

        //
        // Initialize the PROPSPECs
        //
        PropSpec[0].ulKind = PRSPEC_PROPID;
        PropSpec[0].propid = WIA_IPA_FORMAT;
        PropSpec[1].ulKind = PRSPEC_PROPID;
        PropSpec[1].propid = WIA_IPA_TYMED;

        //
        // Initialize the PROPVARIANTs
        //
        PropVariant[0].vt = VT_CLSID;
        PropVariant[0].puuid = &guidOutputFormat;
        PropVariant[1].vt = VT_I4;
        PropVariant[1].lVal = trnsfFile ? TYMED_FILE : TYMED_CALLBACK;

        //
        // Set the properties
        //
        hr = pWiaPropertyStorage->WriteMultiple( c_nPropCount, PropSpec, PropVariant, WIA_IPA_FIRST );
        if (SUCCEEDED(hr))
        {
            //
            // Get the IWiaDataTransfer interface
            //
            IWiaDataTransfer *pWiaDataTransfer = NULL;
            hr = pWiaItem->QueryInterface( IID_IWiaDataTransfer, (void**)&pWiaDataTransfer );
            if (SUCCEEDED(hr))
            {
                //
                // Create our callback class
                //
                if (pCallback)
                {
                    //
                    // Get the IWiaDataCallback interface from our callback class.
                    //
                    IWiaDataCallback *pWiaDataCallback = NULL;
                    hr = pCallback->QueryInterface( IID_IWiaDataCallback, (void**)&pWiaDataCallback );
                    if (SUCCEEDED(hr))
                    {
                        if (!trnsfFile) {
//                            WIA_EXTENDED_TRANSFER_INFO  extendedTransferInfo;
//                            hr = pWiaDataTransfer->idtGetExtendedTransferInfo(&extendedTransferInfo);
//                            if (hr == S_OK) {
//                                // will give maxBuf 4294967295 minBuf 0 optBuf 0 num 1
//                                std::cout << "idtGetExtendedTransferInfo"
//                                          << " maxBuf " << extendedTransferInfo.ulMaxBufferSize
//                                          << " minBuf " << extendedTransferInfo.ulMinBufferSize
//                                          << " optBuf " << extendedTransferInfo.ulOptimalBufferSize
//                                          << " num " << extendedTransferInfo.ulNumBuffers
//                                          << std::endl;
//                            }
//                            else {
//                                std::cout << "idtGetExtendedTransferInfo hr " << hr << std::endl;
//                            }

                            // will transfer in memory
                            WIA_DATA_TRANSFER_INFO dataTrsfInfo{0};
                            dataTrsfInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
                            dataTrsfInfo.ulBufferSize = 327680;  // (5*65536) with 0 the used buffer is 64k and called multiple times
                            dataTrsfInfo.ulSection = 0; // supports dedicated transfer pointer
                            dataTrsfInfo.bDoubleBuffer = FALSE;

                            hr = pWiaDataTransfer->idtGetBandedData( &dataTrsfInfo, pWiaDataCallback );
                            if (S_OK == hr)
                            {
                                std::cout << "Transfer ok " << hr << std::endl;
                            }
                            else {
                                // we will always get a error
                                std::string message = std::system_category().message(hr);
                                std::cout << "Transfer error " << std::hex << hr << std::dec
                                          << " msg " << message << std::endl;
                            }
                        }
                        else {
                            // will transfer file
                            //
                            // Perform the transfer using default settings
                            //
                            STGMEDIUM stgMedium = {0};
                            stgMedium.tymed = TYMED_FILE;
                            hr = pWiaDataTransfer->idtGetData( &stgMedium, pWiaDataCallback );
                            if (S_OK == hr)
                            {
                                //
                                // Print the filename (note that this filename is always
                                // a WCHAR string, not TCHAR).
                                //
                                std::cout << "Transferred filename: " << StringUtils::utf8_encode(stgMedium.lpszFileName) << std::endl;

                                //
                                // Release any memory associated with the stgmedium
                                // This will delete the file stgMedium.lpszFileName.
                                //
                                ReleaseStgMedium( &stgMedium );
                            }
                        }

                        //
                        // Release the callback interface
                        //
                        pWiaDataCallback->Release();
                        pWiaDataCallback = NULL;
                    }

                    //
                    // Release our callback.  It should now delete itself.
                    //
                    //pCallback->Release();
                    //pCallback = NULL;
                }

                //
                // Release the IWiaDataTransfer
                //
                pWiaDataTransfer->Release();
                pWiaDataTransfer = NULL;
            }
        }

        //
        // Release the IWiaPropertyStorage
        //
        pWiaPropertyStorage->Release();
        pWiaPropertyStorage = NULL;
    }
    else {
        std::cout << "error pWiaItem not a IID_IWiaPropertyStorage!" << std::endl;
    }

    return hr;
}

// this will call transfer if a image item is found
HRESULT
WiaDevice::enumerateItems( IWiaItem *pWiaItem, CWiaDataCallback *pCallback ) //XP or earlier
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem) {
        return E_INVALIDARG;
    }

    //
    // Get the item type for this item.
    //
    LONG lItemType = 0;
    HRESULT hr = pWiaItem->GetItemType(&lItemType);
    if (SUCCEEDED(hr)) {
        //
        // If it is a folder, or it has attachments, enumerate its children.
        //
        //std::cout << "Item type " << std::hex << lItemType << std::dec
        //          << ((lItemType & WiaItemTypeFolder) != 0 ? " folder" : "")
        //          << ((lItemType & WiaItemTypeHasAttachments) != 0 ? " attach" : "")
        //          << std::endl;
        if (lItemType & WiaItemTypeFolder || lItemType & WiaItemTypeHasAttachments) {
            //
            // Get the child item enumerator for this item.
            //
            IEnumWiaItem *pEnumWiaItem = NULL; //XP or earlier
            hr = pWiaItem->EnumChildItems(&pEnumWiaItem);
            if (SUCCEEDED(hr)) {
                //
                // Loop until you get an error or pEnumWiaItem->Next returns
                // S_FALSE to signal the end of the list.
                //
                while (S_OK == hr) {
                    //
                    // Get the next child item.
                    //
                    IWiaItem *pChildWiaItem = NULL; //XP or earlier
                    hr = pEnumWiaItem->Next(1, &pChildWiaItem, NULL);

                    //
                    // pEnumWiaItem->Next will return S_FALSE when the list is
                    // exhausted, so check for S_OK before using the returned
                    // value.
                    //
                    if (S_OK == hr) {
                        LONG lchldItemType = 0;
                        hr = pChildWiaItem->GetItemType(&lchldItemType);
                        //std::cout << "   " << std::hex << chldItemType << std::dec
                        //  << ((chldItemType & WiaItemTypeImage) != 0 ? " image" : "")
                        //  << std::endl;
                        if (hr == S_OK && lchldItemType & WiaItemTypeImage) {
                            hr = transferWiaItem(pChildWiaItem, false, pCallback);
                        }
                        //
                        // Recurse into this item.
                        //
                        hr = enumerateItems(pChildWiaItem, pCallback);

                        //
                        // Release this item.
                        //
                        pChildWiaItem->Release();
                        pChildWiaItem = NULL;
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
                // Release the enumerator.
                //
                pEnumWiaItem->Release();
                pEnumWiaItem = NULL;
            }
        }
    }
    return hr;
}


HRESULT
WinScan::readSomeWiaProperties(IWiaPropertyStorage *pWiaPropertyStorage)
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
WinScan::enumerateWiaDevices() //XP or earlier
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

WiaDevice::WiaDevice(WinScan* winScan, const BSTR& devId, const BSTR& devName, const BSTR& devDescr)
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
WiaDevice::scan(CWiaDataCallback *pCallback)
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

WinScan::WinScan()
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

WinScan::~WinScan()
{
    if (m_pWiaDevMgr) {
        m_pWiaDevMgr->Release();
        m_pWiaDevMgr = nullptr;
    }
}

