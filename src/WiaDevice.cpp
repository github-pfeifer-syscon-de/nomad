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

#include "WiaDevice.hpp"
#include "StringUtils.hpp"
#include "WiaProperty.hpp"

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
WiaDevice::transferWiaItem( IWiaItem *pWiaItem, bool trnsfFile, WiaDataCallback *pCallback)
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
//        ULONG ulNumProps = 0;
//        HRESULT hr = pWiaPropertyStorage->GetCount(&ulNumProps);
//        if (SUCCEEDED(hr)) {
//            std::cout << "numProps " << ulNumProps << std::endl;
//        }
//        else {
//            std::cout << "Error " << hr << " pWiaPropertyStorage->GetCount" << std::endl;
//        }

        IEnumSTATPROPSTG* penum = nullptr;
        hr = pWiaPropertyStorage->Enum(&penum);
        if (SUCCEEDED(hr)) {
            while (S_OK == hr) {
                //
                // Get the next device's property storage interface pointer
                //
                STATPROPSTG nextWiaPropertyStorage{0};
                hr = penum->Next(1, &nextWiaPropertyStorage, NULL);

                // pWiaEnumDevInfo->Next will return S_FALSE when the list is
                // exhausted, so check for S_OK before using the returned
                // value.
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
WiaDevice::enumerateItems( IWiaItem *pWiaItem, WiaDataCallback *pCallback ) //XP or earlier
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

