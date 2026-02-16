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

#include <iostream>

#include "WiaDevice.hpp"
#include "WiaScan.hpp"
#include "StringUtils.hpp"
#include "WiaProperty.hpp"

WiaDevice::WiaDevice(WiaScan* winScan, const BSTR& devId, const BSTR& devName, const BSTR& devDescr)
: m_winScan{winScan}
, m_pWiaDevice{nullptr}
, m_devId(StringUtils::utf8_encode(devId))
, m_devName{StringUtils::utf8_encode(devName)}
, m_devDescr{StringUtils::utf8_encode(devDescr)}
{
    //std::cout << "WiaDevice::WiaDevice WIA_DIP_DEV_ID: " << m_devId
    //      << " WIA_DIP_DEV_NAME: " << m_devName
    //      << " WIA_DIP_DEV_DESC: " << m_devDescr << std::endl;
    HRESULT hr = createWiaDevice(winScan->getWiaDevMgr(), devId);
    if (SUCCEEDED(hr)) {
        //std::cout << "WiaDevice::WiaDevice succeeded" << std::endl;
    }
    else {
        std::string message = std::system_category().message(hr);
        std::cout << "WiaDevice::WiaDevice  Error " << hr
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

Glib::ustring
WiaDevice::getDeviceId()
{
    return m_devId;
}

Glib::ustring
WiaDevice::getDeviceName()
{
    return m_devName;
}

IWiaItem2*
WiaDevice::getWiaItem() {
    return m_pWiaDevice;
}

bool
WiaDevice::scan(WiaDataCallback *pCallback, std::map<uint32_t, WiaValue> properties)
{
    if (m_pWiaDevice) {
        IWiaItem2 *pChildWiaItem = nullptr;
        HRESULT hr = findItem(m_pWiaDevice, WiaItemTypeImage, &pChildWiaItem);
        if (SUCCEEDED(hr)) {
            if (pChildWiaItem) {
                hr = transferWiaItem(pChildWiaItem, false, pCallback, properties);
                pChildWiaItem->Release();
                pChildWiaItem = nullptr;
                return SUCCEEDED(hr);
            }
            else {
                std::cout << "Could not identify image item!" << std::endl;
            }
        }
        else {
            std::string message = std::system_category().message(hr);
            std::cout << "Error " << hr
                      << " enumerateItems " << message << std::endl;
        }
    }
    return false;
}


HRESULT  //Vista or later:
WiaDevice::createWiaDevice( IWiaDevMgr2 *pWiaDevMgr, BSTR bstrDeviceID )
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
    HRESULT hr = pWiaDevMgr->CreateDevice(0, bstrDeviceID, &m_pWiaDevice);

    //
    // Return the result of creating the device
    //
    return hr;
}

std::list<std::shared_ptr<WiaProperty>>
WiaDevice::getProperties()
{
    if (m_properties.empty()) {
        IWiaItem2 *pChildWiaItem = nullptr;
        HRESULT hr = findItem(m_pWiaDevice, WiaItemTypeImage, &pChildWiaItem);
        if (SUCCEEDED(hr)) {
            if (pChildWiaItem) {
                IWiaPropertyStorage *pWiaPropertyStorage{};
                HRESULT hr = pChildWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
                if (SUCCEEDED(hr)) {
                    getProperties(pWiaPropertyStorage, "image");
                }
                else {
                    std::cout <<  "Item not convertible IID_IWiaPropertyStorage" << std::endl;
                }
                pChildWiaItem->Release();
                pChildWiaItem = nullptr;
            }
            else {
                std::cout << "Could not identify image item!" << std::endl;
            }
        }
        else {
            std::cout << "Error enumerate hr " << hr << "!" << std::endl;
        }
    }
    return m_properties;
}

void
WiaDevice::getProperties(IWiaPropertyStorage *pWiaPropertyStorage, const std::string& section)
{
    std::cout << "WiaDevice::getProperties pWiaPropertyStorage " << pWiaPropertyStorage << std::endl;
    ULONG ulNumProps{};
    HRESULT hr = pWiaPropertyStorage->GetCount(&ulNumProps);
    if (hr != S_OK) {
        std::string message = std::system_category().message(hr);
        std::cout << "WiaDevice::getPropertiesulNumProps hr " << hr 
                  << " message " << message << std::endl;
    }
    IEnumSTATPROPSTG *penum = NULL;
    hr = pWiaPropertyStorage->Enum(&penum);
    if (SUCCEEDED(hr)) {
        while (S_OK == hr) {
            //
            // Get the next device's property storage interface pointer
            //
            STATPROPSTG nextWiaPropertyStorage{};
            hr = penum->Next(1, &nextWiaPropertyStorage, NULL);
            //std::cout << "WiaDevice::getProperties Next hr " << hr
            //          << " nextWiaPropertyStorage " << (nextWiaPropertyStorage.lpwstrName != nullptr ? StringUtils::utf8_encode(nextWiaPropertyStorage.lpwstrName) : "null") << std::endl;

            // pWiaEnumDevInfo->Next will return S_FALSE when the list is
            // exhausted, so check for S_OK before using the returned
            // value.
            if (hr == S_OK) {
                auto property = std::make_shared<WiaProperty>(nextWiaPropertyStorage);
                m_properties.push_back(property);
                if (!section.empty()) {
                    std::string info = property->info(pWiaPropertyStorage);
                    std::cout << section << " ---------------------------------------" << std::endl
                              << info << std::endl;
                }
            }
        }
        //
        // If the result of the enumeration is S_FALSE (which is normal), change it to S_OK.
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
        std::string message = std::system_category().message(hr);
        std::cout << "WiaDevice::getProperties pWiaPropertyStorage->Enum hr " << hr 
                  << " message " << message << std::endl;
    }

}

HRESULT
WiaDevice::transferWiaItem( IWiaItem2 *pWiaItem, bool trnsfFile, WiaDataCallback *pCallback, std::map<uint32_t, WiaValue> properties)
{
    //
    // Validate arguments
    //
    if (NULL == pWiaItem) {
        return E_INVALIDARG;
    }    
    // Get the IWiaTransfer interface
    //
    IWiaTransfer *pWiaTransfer = NULL;
    HRESULT hr = pWiaItem->QueryInterface(IID_IWiaTransfer,(void**)&pWiaTransfer);
    if (SUCCEEDED(hr)) {
        // Download   
        hr = pWiaTransfer->Download(0,pCallback);
        
        // Release the IWiaTransfer 
        pWiaTransfer->Release();
        pWiaTransfer = NULL;
    }
    return hr;    
}
//#else
//HRESULT
//WiaDevice::transferWiaItem( IWiaItem *pWiaItem, bool trnsfFile, WiaDataCallback *pCallback, std::map<uint32_t, WiaValue> properties)
//{
//    //
//    // Validate arguments
//    //
//    if (NULL == pWiaItem) {
//        return E_INVALIDARG;
//    }
//
//    //
//    // Get the IWiaPropertyStorage interface so you can set required properties.
//    //
//    IWiaPropertyStorage *pWiaPropertyStorage = NULL;
//    HRESULT hr = pWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
//    if (SUCCEEDED(hr)) {
//        //
//        // Prepare PROPSPECs and PROPVARIANTs for setting the
//        // media type and format
//        //
//        std::vector<PROPSPEC> PropSpec;
//        std::vector<PROPVARIANT> PropVariant;
//
//        //
//        // Use BMP as the output format
//        //
//        GUID guidOutputFormat = trnsfFile ? WiaImgFmt_BMP : WiaImgFmt_MEMORYBMP;    // seems to have not much influence
//
//        //
//        // Initialize the PROPSPECs&PROPVARIANTs
//        //
//        PropSpec.push_back( {
//            .ulKind = PRSPEC_PROPID, 
//            .propid = WIA_IPA_FORMAT});
//        PROPVARIANT propVar;
//        PropVariantInit(&propVar);
//        propVar.vt = VT_CLSID;
//        propVar.puuid = &guidOutputFormat;
//        PropVariant.push_back(propVar);
//
//        PropSpec.push_back( {
//            .ulKind = PRSPEC_PROPID, 
//            .propid = WIA_IPA_TYMED});
//        PropVariantInit(&propVar);
//        propVar.vt = VT_I4;
//        propVar.lVal = trnsfFile ? TYMED_FILE : TYMED_CALLBACK;
//        PropVariant.push_back(propVar);
//
//        for (auto entry : properties) {
//            auto value = entry.second;
//            PropSpec.push_back({
//                .ulKind = PRSPEC_PROPID,
//                .propid = entry.first});
//            PropVariantInit(&propVar);
//            if (value.get(propVar)) {
//                PropVariant.push_back(propVar);
//            }
//            else {
//                std::cout << "The variant " << entry.first << " value type " << value.getVt() << " was unsupported!" << std::endl;
//            }
//        }
//        //
//        // Set the properties
//        //
//        hr = pWiaPropertyStorage->WriteMultiple(static_cast<ULONG>(PropSpec.size()), &PropSpec[0], &PropVariant[0], WIA_IPA_FIRST );
//        if (SUCCEEDED(hr)) {
//            //
//            // Get the IWiaDataTransfer interface
//            //
//            IWiaDataTransfer *pWiaDataTransfer = NULL;
//            hr = pWiaItem->QueryInterface( IID_IWiaDataTransfer, (void**)&pWiaDataTransfer );
//            if (SUCCEEDED(hr)) {
//                //
//                // Create our callback class
//                //
//                if (pCallback) {
//                    //
//                    // Get the IWiaDataCallback interface from our callback class.
//                    //
//                    IWiaDataCallback *pWiaDataCallback = NULL;
//                    hr = pCallback->QueryInterface( IID_IWiaDataCallback, (void**)&pWiaDataCallback );
//                    if (SUCCEEDED(hr)) {
//                        if (!trnsfFile) {
////                            WIA_EXTENDED_TRANSFER_INFO  extendedTransferInfo;
////                            hr = pWiaDataTransfer->idtGetExtendedTransferInfo(&extendedTransferInfo);
////                            if (hr == S_OK) {
////                                // will give maxBuf 4294967295 minBuf 0 optBuf 0 num 1
////                                std::cout << "idtGetExtendedTransferInfo"
////                                          << " maxBuf " << extendedTransferInfo.ulMaxBufferSize
////                                          << " minBuf " << extendedTransferInfo.ulMinBufferSize
////                                          << " optBuf " << extendedTransferInfo.ulOptimalBufferSize
////                                          << " num " << extendedTransferInfo.ulNumBuffers
////                                          << std::endl;
////                            }
////                            else {
////                                std::cout << "idtGetExtendedTransferInfo hr " << hr << std::endl;
////                            }
//
//                            // will transfer in memory
//                            WIA_DATA_TRANSFER_INFO dataTrsfInfo{0};
//                            dataTrsfInfo.ulSize = sizeof(WIA_DATA_TRANSFER_INFO);
//                            dataTrsfInfo.ulBufferSize = 327680;  // (5*65536) with 0 the used buffer is 64k and called multiple times
//                            dataTrsfInfo.ulSection = 0; // supports dedicated transfer pointer
//                            dataTrsfInfo.bDoubleBuffer = FALSE;
//
//                            hr = pWiaDataTransfer->idtGetBandedData( &dataTrsfInfo, pWiaDataCallback );
//                            if (S_OK == hr) {
//                                std::cout << "Transfer ok " << hr << std::endl;
//                            }
//                            else {
//                                // we will always get a error
//                                std::string message = std::system_category().message(hr);
//                                std::cout << "Transfer error " << std::hex << hr << std::dec
//                                          << " msg " << message << std::endl;
//                            }
//                        }
//                        else {
//                            // will transfer file
//                            //
//                            // Perform the transfer using default settings
//                            //
//                            STGMEDIUM stgMedium = {0};
//                            stgMedium.tymed = TYMED_FILE;
//                            hr = pWiaDataTransfer->idtGetData( &stgMedium, pWiaDataCallback );
//                            if (S_OK == hr)
//                            {
//                                //
//                                // Print the filename (note that this filename is always
//                                // a WCHAR string, not TCHAR).
//                                //
//                                std::cout << "Transferred filename: " << StringUtils::utf8_encode(stgMedium.lpszFileName) << std::endl;
//
//                                //
//                                // Release any memory associated with the stgmedium
//                                // This will delete the file stgMedium.lpszFileName.
//                                //
//                                ReleaseStgMedium( &stgMedium );
//                            }
//                        }
//
//                        //
//                        // Release the callback interface
//                        //
//                        pWiaDataCallback->Release();
//                        pWiaDataCallback = NULL;
//                    }
//
//                    //
//                    // Release our callback.  It should now delete itself.
//                    //
//                    //pCallback->Release();
//                    //pCallback = NULL;
//                }
//
//                //
//                // Release the IWiaDataTransfer
//                //
//                pWiaDataTransfer->Release();
//                pWiaDataTransfer = NULL;
//            }
//        }
//
//        //
//        // Release the IWiaPropertyStorage
//        //
//        pWiaPropertyStorage->Release();
//        pWiaPropertyStorage = NULL;
//    }
//    else {
//        std::cout << "error pWiaItem not a IID_IWiaPropertyStorage!" << std::endl;
//    }
//
//    return hr;
//}
//#endif



// named EnumerateItems in example
// this will set pRetChildItem if found
HRESULT
WiaDevice::findItem(IWiaItem2 *pWiaItem, LONG typeMask, IWiaItem2** pRetChildWiaItem)
{
    //std::cout << "WiaDevice::findItem pWiaItem (Device) " << pWiaItem << std::endl;
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
        //          << ((lItemType & WiaItemTypeHasAttachments) != 0 ? " attach" : "") << std::endl;
        if (lItemType & WiaItemTypeFolder || lItemType & WiaItemTypeHasAttachments) {
            //
            // Get the child item enumerator for this item.
            //     
            IEnumWiaItem2 *pEnumWiaItem = NULL; //vista and later
            hr = pWiaItem->EnumChildItems(nullptr, &pEnumWiaItem);
            if (SUCCEEDED(hr)) {
                // Loop until you get an error or pEnumWiaItem->Next returns S_FALSE to signal the end of the list.
                while (S_OK == hr) {
                    //
                    // Get the next child item.
                    //
                    IWiaItem2 *pChildWiaItem = NULL; //vista and later
                    hr = pEnumWiaItem->Next(1, &pChildWiaItem, NULL);
                    //std::cout << "WiaDevice::findItem hr " << hr
                    //          << " pChildWiaItem " << pChildWiaItem << std::endl;
                    //
                    if (S_OK == hr) {
                        LONG lchldItemType{};
                        hr = pChildWiaItem->GetItemType(&lchldItemType);
                        //std::cout << "   " << std::hex << lchldItemType 
                        //  << " typeMask " << typeMask << std::dec
                        //  << ((lchldItemType & typeMask) != 0 ? " match" : "")
                        //  << std::endl;
                        if (hr == S_OK && lchldItemType & typeMask) {
                            *pRetChildWiaItem = pChildWiaItem;
                            break;
                        }
                        //
                        // Recurse into this item.
                        //
                        hr = findItem(pChildWiaItem, typeMask, pRetChildWiaItem);
                        //
                        // Release this item.
                        //
                        pChildWiaItem->Release();
                        pChildWiaItem = NULL;
                        if (*pRetChildWiaItem) {
                            break;
                        }
                    }
                }
                // If the result of the enumeration is S_FALSE (which is normal), change it to S_OK.
                if (S_FALSE == hr) {
                    hr = S_OK;
                }
                // Release the enumerator.
                pEnumWiaItem->Release();
                pEnumWiaItem = NULL;
            }
        }
    }
    return hr;
}

int32_t
WiaDevice::convertX4DPI(int32_t x1, int32_t dpi)
{
    // need to adapt values as these depend on dpi setting
    double relx = static_cast<double>(x1) / static_cast<double>(horzResolution);
    return static_cast<int32_t>(relx * static_cast<double>(dpi));
}


int32_t
WiaDevice::convertY4DPI(int32_t y1, int32_t dpi)
{
    // need to adapt values as these depend on dpi setting
    double rely = static_cast<double>(y1) / static_cast<double>(vertResolution);
    return static_cast<int32_t>(rely * static_cast<double>(dpi));
}

std::map<uint32_t, WiaValue>
WiaDevice::buildScanProperties(
        bool full
        , int32_t bright
        , int32_t contr
        , int32_t tresh
        , int32_t res
        , int32_t bits
        , double xRelStart
        , double yRelStart
        , double xRelEnd
        , double yRelEnd)
{
    WiaValue wiaBits;
    wiaBits.set(bits);
    WiaValue brightness;
    brightness.set(bright);
    WiaValue contrast;
    contrast.set(contr);
    WiaValue threshold;
    threshold.set(tresh);

    auto dpi = full ? res : 50;
    WiaValue horz;
    horz.set(dpi);
    WiaValue vert;
    vert.set(dpi);
    auto map = std::map<uint32_t, WiaValue>();
    map.insert(std::make_pair(PropertyResolutionX, horz));
    map.insert(std::make_pair(PropertyResolutionY, vert));
    map.insert(std::make_pair(PropertyBits, wiaBits));
    map.insert(std::make_pair(PropertyBrightness, brightness));
    map.insert(std::make_pair(PropertyContrast, contrast));
    map.insert(std::make_pair(PropertyThreshold, threshold));
    WiaValue ext;
    ext.set("");    // see explanation https://learn.microsoft.com/en-us/windows-hardware/drivers/image/wia-raw-data-header
    map.insert(std::make_pair(PropertyExtension, ext)); 
    WiaValue fmt;
    PROPVARIANT varFmt;
    CLSID clsid{};
    CoGetPSClsid(WiaImgFmt_RAWRGB, &clsid);
    varFmt.vt = VT_CLSID;
    varFmt.puuid = &clsid;  
    fmt.set(varFmt); // use raw as always ;)
    map.insert(std::make_pair(PropertyFormat, fmt));
    
    
    if (full) {
        int xMaxExtend = 2000.0;
        if (m_extendX.size() >= 2) {
            xMaxExtend = m_extendX[0].get<int32_t>();
        }
        int yMaxExtend = 3000.0;
        if (m_extendY.size() >= 2) {
            yMaxExtend = m_extendY[0].get<int32_t>();
        }
        //std::cout << "xMax " << xMaxExtend
        //          << " yMax "  << yMaxExtend << std::endl;
        //std::cout << "Start " << m_scanPreview->getXStart()
        //          << " " << m_scanPreview->getYStart()
        //          << " end " << m_scanPreview->getXEnd()
        //          << " " << m_scanPreview->getYEnd() << std::endl;
        int32_t x1 = static_cast<int32_t>(xRelStart * xMaxExtend);
        if (m_startX.size() >= 2) {
            x1 = std::min(x1, m_startX[0].get<int32_t>());
            x1 = std::max(x1, m_startX[1].get<int32_t>());
        }
        WiaValue xStart;
        xStart.set(convertX4DPI(x1, dpi));
        map.insert(std::make_pair(PropertyStartX, xStart));
        int32_t y1 = static_cast<int32_t>(yRelStart * yMaxExtend);
        if (m_startY.size() >= 2) {
            y1 = std::min(y1, m_startY[0].get<int32_t>());
            y1 = std::max(y1, m_startY[1].get<int32_t>());
        }
        WiaValue yStart;
        yStart.set(convertY4DPI(y1, dpi));
        map.insert(std::make_pair(PropertyStartY, yStart));
        WiaValue xExtend;
        int32_t width = static_cast<int32_t>((xRelEnd - xRelStart) * xMaxExtend + 0.99);
        int32_t effWidth = std::min(width, xMaxExtend - x1 - 1);
        xExtend.set(convertX4DPI(effWidth, dpi));
        map.insert(std::make_pair(PropertyExtendX, xExtend));
        WiaValue yExtend;
        int32_t height = static_cast<int32_t>((yRelEnd - yRelStart) * yMaxExtend + 0.99);
        int32_t effHeight = std::min(height, yMaxExtend - y1 - 1);
        yExtend.set(convertY4DPI(effHeight, dpi));
        map.insert(std::make_pair(PropertyExtendY, yExtend));
        //std::cout << "Start " << x1 << " " << y1
        //          << " width " << width
        //          << " height " << height << std::endl;
    }

    return map;
}


void
WiaDevice::readExtends(IWiaPropertyStorage *pWiaPropertyStorage)
{
    for (auto property : getProperties()) {
        if (property->getPropertyId() == PropertyStartX) {
            m_startX = property->getRange(pWiaPropertyStorage);
        }
        if (property->getPropertyId() == PropertyStartY) {
            m_startY = property->getRange(pWiaPropertyStorage);
        }
        if (property->getPropertyId() == PropertyExtendX) {
            m_extendX = property->getRange(pWiaPropertyStorage);
        }
        if (property->getPropertyId() == PropertyExtendY) {
            m_extendY = property->getRange(pWiaPropertyStorage);
        }
        if (property->getPropertyId() == PropertyResolutionX) {
            horzResolution = property->getValue(pWiaPropertyStorage).get<int32_t>();
        }
        if (property->getPropertyId() == PropertyResolutionY) {
            vertResolution = property->getValue(pWiaPropertyStorage).get<int32_t>();
        }
    }
}

