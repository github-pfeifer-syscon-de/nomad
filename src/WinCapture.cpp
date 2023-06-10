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
//   PURPOSE: Captures a screenshot into a bitmap and covert it into a pixbuf.
//
//   COMMENTS:
//
//      Note: will capture screen if hWnd is NULL
//

static
GdkPixbuf* CaptureAnImage(HWND hWnd)
{
    LONG width;
    LONG height;
    GdkPixbuf* pixbuf = nullptr;
    HDC hdcWindow = GetDC(hWnd);
    if (hWnd == nullptr) {
        width = GetSystemMetrics(SM_CXSCREEN);
        height = GetSystemMetrics(SM_CYSCREEN);
    }
    else {
        RECT rect;
        // get some extra, why ?
        GetWindowRect(hWnd, &rect);
        width = rect.right - rect.left;
        height = rect.bottom - rect.top;
    }

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    HDC hdcMemDC = CreateCompatibleDC(hdcWindow);
    if (hdcMemDC) {
        // Create a compatible bitmap from the Window DC.
        HBITMAP hbmWindow = CreateCompatibleBitmap(hdcWindow, width, height);
        if (hbmWindow) {
            // Select the compatible bitmap into the compatible memory DC.
            SelectObject(hdcMemDC, hbmWindow);
            // Bit block transfer into our compatible memory DC.
            if (BitBlt(hdcMemDC,
                0, 0,
                width, height,
                hdcWindow,
                0, 0,
                SRCCOPY)) {

                // Get the BITMAP from the HBITMAP.
                BITMAP bmpWindow;
                GetObject(hbmWindow, sizeof(BITMAP), &bmpWindow);

                BITMAPINFOHEADER bi{
                      .biSize = sizeof(BITMAPINFOHEADER)
                    , .biWidth = bmpWindow.bmWidth
                    , .biHeight = bmpWindow.bmHeight
                    , .biPlanes = 1
                    , .biBitCount = 32
                    , .biCompression = BI_RGB
                    , .biSizeImage = 0
                    , .biXPelsPerMeter = 0
                    , .biYPelsPerMeter = 0
                    , .biClrUsed = 0
                    , .biClrImportant = 0
                };

                // for this case (ABGR) the calculation is useless
                DWORD dwRowStride = ((bmpWindow.bmWidth * bi.biBitCount + 31) / 32) * 4;
                DWORD dwBmpSize = dwRowStride * bmpWindow.bmHeight;

                // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that
                // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc
                // have greater overhead than HeapAlloc.
                HANDLE hDIB = GlobalAlloc(GHND, dwBmpSize);
                if (hDIB) {
                    char* lpbitmap = (char*)GlobalLock(hDIB);
                    // Gets the "bits" from the bitmap, and copies them into a buffer
                    // that's pointed to by lpbitmap.
                    GetDIBits(hdcWindow, hbmWindow, 0,
                        (UINT)bmpWindow.bmHeight,
                        lpbitmap,
                        (BITMAPINFO*)&bi, DIB_RGB_COLORS);

                    // the data has a different organisation r<->b and top<->bottom
                    #ifdef WINCAP_DEBUG
                    std::cout << "g_malloc " << dwBmpSize
                              << " width " << bmpWindow.bmWidth
                              << " height " << bmpWindow.bmHeight
                              << " stride " << dwRowStride << std::endl;
                    #endif
                    gpointer buf = g_malloc(dwBmpSize);
                    if (buf) {
                        GBytes* bytes = g_bytes_new_take(buf, dwBmpSize);
                        for (int y = 0; y < bmpWindow.bmHeight; ++y) {
                            DWORD yd = (bmpWindow.bmHeight-1) - y;
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
                                rowd[x+3] = 0xff;       // rows[x+3] looks strange
                            }
                        }
                        pixbuf = gdk_pixbuf_new_from_bytes(bytes
                                , GDK_COLORSPACE_RGB
                                , true
                                , 8
                                , bmpWindow.bmWidth
                                , bmpWindow.bmHeight
                                , dwRowStride);
                    }
                    else {
                        std::cout << "Allocate pixbuf size " << dwBmpSize << " failed" << std::endl;
                    }

                    // Unlock and Free the DIB from the heap.
                    GlobalUnlock(hDIB);
                    GlobalFree(hDIB);
                }
                else {
                    std::cout << "Error alloc bitmap size " << dwBmpSize << std::endl;
                }
            }
            else {
                std::cout << "BitBlt has failed" << std::endl;
                //MessageBox(nullptr, "BitBlt has failed", "Failed", MB_OK);
            }
            DeleteObject(hbmWindow);
        }
        else {
            std::cout << "CreateCompatibleBitmap Failed" << std::endl;
        }
    }
    else {
        std::cout << "CreateCompatibleDC has failed" << std::endl;
    }

    // Clean up.
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcWindow);

    return pixbuf;
}

GdkPixbuf *
WinCapture::get_pixbuf(GdkRectangle* rectangle)
{
    HWND hwnd = nullptr;
    if (get_take_window_shot()) {
        hwnd = GetForegroundWindow();
    }
    GdkPixbuf *pixbuf = CaptureAnImage(hwnd);
    return pixbuf;
}
