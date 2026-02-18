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
#include <algorithm>
#include <memory>
#include <oneapi\tbb.h>

#include <windows.h>
// use local wia.h as workaround if the msys2 version is filled in, use <wia.h>
#include "wia.h"

class WiaData
{
public:
    WiaData(size_t size) 
    : m_size{size}
    {
        m_data = new uint8_t[size];
    }
    WiaData(const void* ptr, size_t size) 
    : WiaData(size)
    {
        auto chr = static_cast<const uint8_t*>(ptr);
        std::copy(chr, chr + size, m_data);
    }
    explicit WiaData(const WiaData& orig) = delete;  // don't copy this
    ~WiaData() 
    {
        if (m_data) {
            delete[] m_data;
        }
    }
    size_t getBytesSize() 
    {
        return m_size;
    }
    size_t getUsed() 
    {
        return m_used;
    }
    void setUsed(size_t used) 
    {
        m_used = used;
    }
    uint8_t* getData()
    {
        return m_data;
    }
    
private:
    uint8_t* m_data;
    const size_t m_size;
    size_t m_used{};
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
    uint32_t m_dataSize{};     // full data size
    uint32_t m_pixelOffs{};     // offset pixel data
    uint32_t m_width{};
    uint32_t m_height{};
    uint32_t m_bits{};
    //std::ofstream  m_stat; 
    tbb::concurrent_queue<std::shared_ptr<WiaData>> m_data;
    uint32_t m_cnt{};
    bool m_pending{false};
    Glib::Dispatcher &m_dispatcher;
    uint32_t m_bytePerPixel{1};
    uint32_t m_bytesPerRow{1};
    size_t m_offs{};
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

    uint32_t getWidth()
    {
        return m_width;
    }
    uint32_t getHeight()
    {
        return m_height;
    }
    // bits per pixel e.g. 24 = RGB, 8 = Grapy, 1 = B/W
    uint32_t getBits()
    {
        return m_bits;
    }
    tbb::concurrent_queue<std::shared_ptr<WiaData>>& getQueue()
    {
        return m_data;
    }
    void resetPending()
    {
        m_pending = false;
    }
    uint32_t getBytePerPixel() 
    {
        return m_bytePerPixel;
    }
    uint32_t getBytePerRow() 
    {
        return m_bytesPerRow;
    }
    
    
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
