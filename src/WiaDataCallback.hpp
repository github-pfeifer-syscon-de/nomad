/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include "GenericCallback.hpp"

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
: public IWiaDataCallback
, public GenericCallback
{
private:
    LONG  m_cRef;               // Object reference count
    PBYTE m_pBuffer;            // Data buffer
    LONG  m_nBufferLength;      // Length of buffer
    LONG  m_nBytesTransfered;   // Total number of bytes transferred
    GUID  m_guidFormat;         // Data format
    LONG  m_percentComplete;
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

    HRESULT __stdcall BandedDataCallback(
            LONG lMessage,
            LONG lStatus,
            LONG lPercentComplete,
            LONG lOffset,
            LONG lLength,
            LONG lReserved,
            LONG lResLength,
            BYTE *pbData);
    uint8_t* getDataTransfered(uint32_t *bytesTransfered, int32_t* percentTransfered, uint32_t *transferAlloc) override;
};
