/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
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
#include <windows.h>

#include "WinCapture.hpp"



// adapted from https://learn.microsoft.com/en-us/windows/win32/gdi/capturing-an-image?redirectedfrom=MSDN
//
//   FUNCTION: CaptureAnImage(HWND hWnd)
//
//   PURPOSE: Captures a screenshot into a window ,and then saves it in a .bmp file.
//
//   COMMENTS:
//
//      Note: This function attempts to create a file called captureqwsx.bmp
//

static
GdkPixbuf* CaptureAnImage()
{
    HDC hdcScreen;
    HDC hdcMemDC = NULL;
    HBITMAP hbmScreen = NULL;
    BITMAP bmpScreen;
    DWORD dwBytesWritten = 0;
    DWORD dwSizeofDIB = 0;
    HANDLE hFile = NULL;
    char* lpbitmap = NULL;
    HANDLE hDIB = NULL;
    DWORD dwBmpSize = 0;
    DWORD dwRowStride = 0;
    GdkPixbuf* pixbuf = nullptr;
    gpointer buf = nullptr;
    GBytes* data = nullptr;

    // Retrieve the handle to a display device context for the client
    // area of the window.
    hdcScreen = GetDC(NULL);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    hdcMemDC = CreateCompatibleDC(hdcScreen);

    if (!hdcMemDC)
    {
        std::cout << "CreateCompatibleDC has failed" << std::endl;
        //MessageBox(nullptr, "CreateCompatibleDC has failed", "Failed", MB_OK);
        goto done;
    }


    // Create a compatible bitmap from the Window DC.
    hbmScreen = CreateCompatibleBitmap(hdcScreen, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));

    if (!hbmScreen)
    {
        std::cout << "CreateCompatibleBitmap Failed" << std::endl;
        //MessageBox(nullptr, "CreateCompatibleBitmap Failed", "Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if (!BitBlt(hdcMemDC,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
        hdcScreen,
        0, 0,
        SRCCOPY))
    {
        std::cout << "BitBlt has failed" << std::endl;
        //MessageBox(nullptr, "BitBlt has failed", "Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // align to dword boundary
    dwRowStride = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4;
    dwBmpSize = dwRowStride * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    if (!hDIB) {
        goto done;
    }
    lpbitmap = (char*)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer
    // that's pointed to by lpbitmap.
    GetDIBits(hdcScreen, hbmScreen, 0,
        (UINT)bmpScreen.bmHeight,
        lpbitmap,
        (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // the data has a different organisation r<->b and top<->bottom
    #ifdef WINCAP_DEBUG
    std::cout << "g_malloc " << dwBmpSize
              << " width " << bmpScreen.bmWidth
              << " height " << bmpScreen.bmHeight
              << " stride " << dwRowStride << std::endl;
    #endif
    buf = g_malloc(dwBmpSize);
    if (!buf) {
        GlobalUnlock(hDIB);
        GlobalFree(hDIB);
        goto done;
    }
    data = g_bytes_new_take(buf, dwBmpSize);
    for (int y = 0; y < bmpScreen.bmHeight; ++y) {
        DWORD yd = (bmpScreen.bmHeight-1) - y;
        #ifdef WINCAP_DEBUG
        std::cout << "row " << y << " yd " << yd << std::endl;
        #endif
        guchar* rows = (guchar*)lpbitmap + y * dwRowStride;
        guchar* rowd = (guchar*)buf + yd * dwRowStride;
        for (guint32 x = 0; x < dwRowStride; x += 4) {
            //  windows   A BGR
        	//  pixbuf    A 31..24  R 23..16  G 15..8   B 7..0
            rowd[x+0] = rows[x+2];
            rowd[x+1] = rows[x+1];
            rowd[x+2] = rows[x+0];
            rowd[x+3] = rows[x+3];
        }
    }
    pixbuf = gdk_pixbuf_new_from_bytes(data
            , GDK_COLORSPACE_RGB
            , true
            , 8
            , bmpScreen.bmWidth
            , bmpScreen.bmHeight
            , dwRowStride);

    // A file is created, this is where we will save the screen capture.
    //hFile = CreateFile("captureqwsx.bmp",
    //    GENERIC_WRITE,
    //    0,
    //    NULL,
    //    CREATE_ALWAYS,
    //    FILE_ATTRIBUTE_NORMAL, NULL);

    // Add the size of the headers to the size of the bitmap to get the total file size.
    //dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    //bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    //bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    //bmfHeader.bfType = 0x4D42; // BM.

    //WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    //WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    //WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    //CloseHandle(hFile);

    // Clean up.
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);

    return pixbuf;
}

GdkPixbuf *
WinCapture::get_pixbuf(GdkRectangle* rectangle)
{
    GdkPixbuf *pixbuf = CaptureAnImage();
    return pixbuf;
}
