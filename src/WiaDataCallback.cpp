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

#include "WiaDataCallback.hpp"

WiaDataCallback::WiaDataCallback(Glib::Dispatcher& dispatcher)
: m_cRef(1)
, m_pBuffer{NULL}
, m_nBufferLength{0}
, m_nBytesTransfered{0}
, m_guidFormat{IID_NULL}
, m_percentComplete{0}
, m_dispatcher{dispatcher}
{
}

WiaDataCallback::~WiaDataCallback()
{
    //
    // Free the item buffer
    //
    if (m_pBuffer)
    {
        LocalFree( m_pBuffer );
        m_pBuffer = NULL;
    }
    m_nBufferLength = 0;
    m_nBytesTransfered = 0;
}

HRESULT CALLBACK
WiaDataCallback::QueryInterface( REFIID riid, void **ppvObject )
{
    //
    // Validate arguments
    //
    if (NULL == ppvObject)
    {
        return E_INVALIDARG;
    }

    //
    // Return the appropriate interface
    //
    if (IsEqualIID( riid, IID_IUnknown ))
    {
        *ppvObject = static_cast<WiaDataCallback *>(this);
    }
    else if (IsEqualIID( riid, IID_IWiaDataCallback ))
    {
        *ppvObject = static_cast<WiaDataCallback *>(this);
    }
    else
    {
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
    if (0 == cRef)
    {
        delete this;
    }
    return cRef;
}

HRESULT __stdcall
WiaDataCallback::BandedDataCallback(
        LONG lMessage,
        LONG lStatus,
        LONG lPercentComplete,
        LONG lOffset,
        LONG lLength,
        LONG lReserved,
        LONG lResLength,
        BYTE *pbData)
    {
    UNREFERENCED_PARAMETER(lReserved);
    UNREFERENCED_PARAMETER(lResLength);
    switch (lMessage)
    {
    case IT_MSG_DATA_HEADER:
        {
            //
            // The data header contains the image's final size.
            //
            auto pHeader = reinterpret_cast<PWIA_DATA_CALLBACK_HEADER>(pbData);
            if (pHeader && pHeader->lBufferSize)
            {
                //std::cout << "header buffer " << pHeader->lBufferSize << std::endl;
                //
                // Allocate a block of memory to hold the image
                //
                m_pBuffer = reinterpret_cast<PBYTE>(LocalAlloc(LPTR,pHeader->lBufferSize));
                if (m_pBuffer)
                {
                    //
                    // Save the buffer size.
                    //
                    m_nBufferLength = pHeader->lBufferSize;

                    //
                    // Initialize the bytes transferred count.
                    //
                    m_nBytesTransfered = 0;

                    //
                    // Save the file format.
                    //
                    m_guidFormat = pHeader->guidFormatID;
                }
            }
        }
        break;

    case IT_MSG_DATA:
        {
            //
            // Make sure a block of memory has been created.
            //
            std::cout << "Got data at"
                      << " buf 0x" << std::hex << (void*)m_pBuffer << std::dec
                      << " offs " << lOffset
                      << " len " << lLength
                      << std::endl;
            if (NULL != m_pBuffer)
            {
                //
                // Copy the new band.
                //
                CopyMemory( m_pBuffer + lOffset, pbData, lLength );

                //
                // Increment the byte count.
                //
                m_nBytesTransfered += lLength;

                m_dispatcher.emit();    // keep gui informed
            }
        }
        break;

    case IT_MSG_STATUS:
        {
            //
            // Display transfer phase
            //
            if (lStatus & IT_STATUS_TRANSFER_FROM_DEVICE)
            {
                //std::cout << "Transfer from device" << std::endl;
            }
            else if (lStatus & IT_STATUS_PROCESSING_DATA)
            {
                //std::cout << "Processing Data" << std::endl;
            }
            else if (lStatus & IT_STATUS_TRANSFER_TO_CLIENT)
            {
                //std::cout << "Transfer to Client" << std::endl;
            }

            m_percentComplete = lPercentComplete;

            //
            // Display percent complete
            //
            //std::cout << "lPercentComplete: " << lPercentComplete << std::endl;
        }
        break;
    }

    return S_OK;
}

uint8_t*
WiaDataCallback::getDataTransfered(uint32_t* bytesTransfered, int32_t* percentTransfered)
{
    *bytesTransfered = m_nBytesTransfered;
    *percentTransfered = m_percentComplete;
    return m_pBuffer;
}
