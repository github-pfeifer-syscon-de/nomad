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
#include <StringUtils.hpp>

#include "WiaDataCallback.hpp"

// remove if these are provided by wiadef.h
//
// IWiaTransferCallback Message types
//

#define WIA_TRANSFER_MSG_STATUS           0x00001
#define WIA_TRANSFER_MSG_END_OF_STREAM    0x00002
#define WIA_TRANSFER_MSG_END_OF_TRANSFER  0x00003
#define WIA_TRANSFER_MSG_DEVICE_STATUS    0x00005
#define WIA_TRANSFER_MSG_NEW_PAGE         0x00006

WiaDataCallback::WiaDataCallback(Glib::Dispatcher& dispatcher)
: m_cRef(1)
, m_dispatcher{dispatcher}
{
}

WiaDataCallback::~WiaDataCallback()
{
    //
    // Free the item buffer
    //
    //if (m_pBuffer) {
    //    LocalFree( m_pBuffer );
    //    m_pBuffer = NULL;
    //}
    //m_nBufferLength = 0;
    //m_nBytesTransfered = 0;
}

HRESULT CALLBACK
WiaDataCallback::QueryInterface( REFIID riid, void **ppvObject )
{
    //
    // Validate arguments
    //
    if (NULL == ppvObject) {
        return E_INVALIDARG;
    }

    //
    // Return the appropriate interface
    //
    if (IsEqualIID( riid, IID_IUnknown )) {
        *ppvObject = static_cast<WiaDataCallback *>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaTransferCallback )) {
        *ppvObject = static_cast<WiaDataCallback *>(this);
    }
    else if (IsEqualIID( riid, IID_IStream )) {
        *ppvObject = static_cast<IStream *>(this);
    }
    else {
        *ppvObject = NULL;
        return(E_NOINTERFACE);
    }

    //
    // Increment the reference count before returning the interface.
    //
    reinterpret_cast<IUnknown*>(*ppvObject)->AddRef();
    return S_OK;
}

ULONG CALLBACK
WiaDataCallback::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG CALLBACK
WiaDataCallback::Release()
{
    LONG cRef = InterlockedDecrement(&m_cRef);
    if (0 == cRef) {
        delete this;
    }
    return cRef;
}

HRESULT 
WiaDataCallback::TransferCallback(
        LONG lFlags,
        WiaTransferParams* pWiaTransferParams)
{
    HRESULT hr = S_OK;

    //std::cout << "WiaDataCallback::TransferCallback" 
    //          << " flags " << lFlags 
    //          << " hr " << pWiaTransferParams->hrErrorStatus
    //          << " percent " << pWiaTransferParams->lPercentComplete
    //          << " bytes " << pWiaTransferParams->ulTransferredBytes
    //          << std::endl;
    switch (pWiaTransferParams->lMessage) {
        case WIA_TRANSFER_MSG_STATUS:
            //std::cout << "  WIA_TRANSFER_MSG_STATUS" << std::endl;
            if (!m_pending) {   // don't overrun
                m_dispatcher.emit();
                m_pending = true;
            }
            break;
        case WIA_TRANSFER_MSG_END_OF_STREAM:
            //std::cout << "  WIA_TRANSFER_MSG_END_OF_STREAM" << std::endl;
            break;
        case WIA_TRANSFER_MSG_END_OF_TRANSFER:
            //std::cout << "  WIA_TRANSFER_MSG_END_OF_TRANSFER" << std::endl;
            m_dispatcher.emit();
            //m_stat.close();
            break;
        default:
            std::cout << "  WIA_TRANSFER_MSG unknow " << pWiaTransferParams->lMessage << std::endl;
            break;
    }

    return hr;
}

HRESULT
WiaDataCallback::GetNextStream(
    LONG    lFlags,
    BSTR    bstrItemName,
    BSTR    bstrFullItemName,
    IStream **ppDestination)
{
    std::cout << "WiaDataCallback::GetNextStream" 
            << " flags " << std::hex << lFlags << std::dec
            << " name " << (bstrItemName!=nullptr?StringUtils::utf8_encode(bstrItemName):"null")
            << " full " << (bstrFullItemName!=nullptr?StringUtils::utf8_encode(bstrFullItemName):"null")
            << std::endl;
    HRESULT hr = S_OK;
    //  Return a new stream for this item's data.
    //
    *ppDestination = this;
    //hr = CreateDestinationStream(bstrItemName, ppDestination);
    //memoryStream
    return hr;
}

HRESULT  __stdcall
WiaDataCallback::Read(void* ptr, ULONG size, ULONG* read)
{
    std::cout << "WiaDataCallback::Read" << std::endl;
    return S_OK;
}
HRESULT  __stdcall
WiaDataCallback::Write(const void* ptr, ULONG size, ULONG* written)
{
    //std::cout << "WiaDataCallback::Write" 
    //          << " count " << m_cnt
    //          << " size " << size << std::endl;     
    if (m_cnt == 0) {
        //m_stat.open("wia_data.dmp");        
        if (size >= 0xel) {
            auto fileheader = reinterpret_cast<const BITMAPFILEHEADER* >(ptr);
            m_dataSize = fileheader->bfSize;
            m_pixelOffs = fileheader->bfOffBits;
            std::cout << "WiaDataCallback::Write"
                      << " dataSize " << m_dataSize
                      << " pixelOffs " << m_pixelOffs << std::endl;            
        }
    }
    else if (m_cnt == 1) {
        auto bitmapHeader = reinterpret_cast<const BITMAPINFOHEADER* >(ptr);
        m_width = bitmapHeader->biWidth ;
        m_height = std::abs(bitmapHeader->biHeight);
        m_bits = bitmapHeader->biBitCount;
        m_bytePerPixel = (m_bits / 8);
        m_bytesPerRow = (( m_width * m_bits + 31) / 32) * sizeof(uint32_t);   // rows are dword aligned 
        std::cout << "WiaDataCallback::Write"
                  << " headerSize " << bitmapHeader->biSize
                  << " width " << bitmapHeader->biWidth 
                  << " height " << bitmapHeader->biHeight
                  << " plan " << bitmapHeader->biPlanes
                  << " bits " << bitmapHeader->biBitCount
                  << " comp " << bitmapHeader->biCompression
                  << " imageSize " << bitmapHeader->biSizeImage 
                  << " bytePerPixel " << m_bytePerPixel 
                  << " srcRowBytes " << m_bytesPerRow << std::endl;                
        //std::cout << StringUtils::hexdump(static_cast<const char*>(ptr), size) << std::endl;                        
    }
    else {  // the data offs is where its expected
        //std::cout << "WiaDataCallback::Write"
        //          << " queue offs " << m_offs << std::endl;
        m_data.push(std::move(std::make_shared<WiaData>(ptr, size)));        
    }
    ++m_cnt;
    m_offs += size;
    //if (m_stat.is_open()) {
    //    m_stat.write(static_cast<const char*>(ptr), size);
    //}
    
    *written = size; 
    return S_OK;
}

HRESULT  __stdcall
WiaDataCallback::Seek(LARGE_INTEGER pos, DWORD dir, ULARGE_INTEGER* set)
{
    if (pos.QuadPart != 0) {    // used to skip to end which is usually a Nop
        std::cout << "WiaDataCallback::Seek"
                  << " pos " << pos.QuadPart
                  << " offs " << dir << std::endl;  
    
        //std::ios_base::seekdir seek{std::ios_base::seekdir::_S_end};
        //if (STREAM_SEEK::STREAM_SEEK_SET == dir) {
        //    seek = std::ios_base::seekdir::_S_cur;
        //}
        //else if (STREAM_SEEK::STREAM_SEEK_CUR == dir) {
        //    seek = std::ios_base::seekdir::_S_cur;
        //}
        //else if (STREAM_SEEK::STREAM_SEEK_END == dir) {
        //    seek = std::ios_base::seekdir::_S_end;
        //}
        //if (m_stat.is_open()) {
        //    m_stat.seekp(pos.QuadPart, seek);
        //}    
    //*set = pos;
    }
    return S_OK;        
}
HRESULT  __stdcall
WiaDataCallback::SetSize(ULARGE_INTEGER size)
{
    std::cout << "WiaDataCallback::SetSize "  << std::endl;        
    return S_OK;                
}
HRESULT  __stdcall
WiaDataCallback::CopyTo(IStream* strm, ULARGE_INTEGER pos, ULARGE_INTEGER* set, ULARGE_INTEGER* large)
{
    std::cout << "WiaDataCallback::CopyTo " << std::endl;        
    return S_OK;                        
}
HRESULT 
WiaDataCallback::Commit(DWORD flags)
{
    std::cout << "WiaDataCallback::Commit flags " << std::hex << flags << std::dec << std::endl;        
    return S_OK;                                
}
HRESULT  __stdcall
WiaDataCallback::Revert()
{
    std::cout << "WiaDataCallback::Revert "  << std::endl;        
    return S_OK;                                        
}
HRESULT  __stdcall
WiaDataCallback::LockRegion(ULARGE_INTEGER pos, ULARGE_INTEGER len, DWORD flag)
{
    std::cout << "WiaDataCallback::LockRegion "  << std::endl;        
    return S_OK;                                               
}
HRESULT  __stdcall
WiaDataCallback::UnlockRegion(ULARGE_INTEGER pos, ULARGE_INTEGER len, DWORD flag)
{
    std::cout << "WiaDataCallback::UnlockRegion "  << std::endl;        
    return S_OK;                                                       
}
HRESULT  __stdcall
WiaDataCallback::Stat(STATSTG* stag, DWORD flag)  
{
    //std::cout << "WiaDataCallback::Stat "
    //          << " type " << stag->type 
    //          << " flag " << std::hex << flag << std::dec << std::endl;        
    return S_OK;                                                       
}
HRESULT  __stdcall
WiaDataCallback::Clone(IStream** stmr)
{
    std::cout << "WiaDataCallback::Clone "  << std::endl;        
    return S_OK;                                                       
}
