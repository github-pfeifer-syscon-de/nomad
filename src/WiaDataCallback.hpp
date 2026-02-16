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
#include <fstream>

#include <windows.h>
// use local wia.h as workaround if the msys2 version is filled in, use <wia.h>
#include "wia.h"

struct WiaRawHeader {
DWORD Tag;         // must contain 'WRAW' (single byte ASCII characters)
DWORD Version;        // must contain 0x00010000
DWORD HeaderSize;       // contains amount of valid bytes in header
DWORD XRes;              // X (horizontal) resolution, in DPI
DWORD YRes;              // Y (vertical) resolution, in DPI
DWORD XExtent;           // image width, in pixels
DWORD YExtent;           // image height, in pixels
DWORD BytesPerLine;      // used only for uncompressed image data, 0 (unknown) for compressed data 
DWORD BitsPerPixel;      // number of bits per pixel (all channels)
DWORD ChannelsPerPixel;  // number of color channels (samples) within a pixel
DWORD DataType;    // current WIA_IPA_DATATYPE value describing the image
BYTE  BitsPerChannel[8]; // up to 8 channels per pixel, use as many as needed  
DWORD Compression;       // current WIA_IPA_COMPRESSION value
DWORD PhotometricInterp; // current WIA_IPS_PHOTOMETRIC_INTERP value
DWORD LineOrder;         // image line order as a WIA_LINE_ORDER value
DWORD RawDataOffset;     // offset position (in bytes, starting from 0) for the raw image data
DWORD RawDataSize;       // size of raw image data, in bytes
DWORD PaletteOffset;     // offset position (in bytes, starting from 0) for the palette (0 if none)
DWORD PaletteSize;       // size, in bytes, of color palette table (0 if no palette is required) 
};
//
// The application must instantiate the CDataCallback object using
// the "new" operator, and call QueryInterface to retrieve the
// IWiaDataCallback interface.
//
// In this example, using in-memory transfer, the application then
// calls the IWiaDataTransfer::idtGetBandedData method and passes
// it the IWiaDataCallback interface pointer.
//
// If the application performs a file transfer using
// IWiaDataTransfer::idtGetData, only status messages are sent,
// and the data is transferred in a file.
//
class WiaDataCallback
: public IWiaTransferCallback
, public IStream
{
private:
    LONG  m_cRef;               // Object reference count
    //PBYTE m_pBuffer;            // Data buffer
    //LONG  m_nBufferLength;      // Length of buffer
    //LONG  m_nBytesTransfered;   // Total number of bytes transferred
    //GUID  m_guidFormat;         // Data format
    //LONG  m_percentComplete;
    std::ofstream  m_stat; 
    uint32_t m_cnt{};
    Glib::Dispatcher &m_dispatcher;
public:

    //
    // Constructor and destructor
    //
    WiaDataCallback(Glib::Dispatcher& dispatcher);
    virtual ~WiaDataCallback();

    //
    // IUnknown methods
    //
    HRESULT CALLBACK QueryInterface( REFIID riid, void **ppvObject );

    ULONG CALLBACK AddRef();
    ULONG CALLBACK Release();

    //
    // The IWiaDataTransfer::idtGetBandedData method periodically
    // calls the IWiaDataCallback::BandedDataCallback method with
    // status messages. It sends the callback method a data header
    // message followed by one or more data messages to transfer
    // data. It concludes by sending a termination message.
    //
    HRESULT __stdcall TransferCallback(
        LONG lFlags,
        WiaTransferParams* p_wiaTransferParams) override;
    HRESULT __stdcall GetNextStream(
        LONG    lFlags,
        BSTR    bstrItemName,
        BSTR    bstrFullItemName,
        IStream **ppDestination) override;
    

    HRESULT __stdcall Read(void*, ULONG, ULONG*) override;
    HRESULT __stdcall Write(const void*, ULONG, ULONG*) override;
    HRESULT __stdcall Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER*) override;
    HRESULT __stdcall SetSize(ULARGE_INTEGER) override;
    HRESULT __stdcall CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*, ULARGE_INTEGER*) override;
    HRESULT __stdcall Commit(DWORD) override;
    HRESULT __stdcall Revert() override;
    HRESULT __stdcall LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
    HRESULT __stdcall UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD) override;
    HRESULT __stdcall Stat(STATSTG*, DWORD) override;
    HRESULT __stdcall Clone(IStream**) override;   
};
