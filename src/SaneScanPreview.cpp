/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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

#include "SaneScanPreview.hpp"

SaneScanPreview::SaneScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Glib::Dispatcher& completed)
: ScanPreview(cobject, builder)
,  m_completed{completed}
{

}

void
SaneScanPreview::setParameter(SANE_Parameters& parameters)
{
    m_offs = 0l;
    m_pixbuf = Gdk::Pixbuf::create(
            Gdk::Colorspace::COLORSPACE_RGB
            , true
            , 8
            , parameters.pixels_per_line
            , parameters.lines);
    m_parameters = parameters;
}

void
SaneScanPreview::append(std::vector<SANE_Byte>& buf, SANE_Int transferBytes)
{
    int yPos = static_cast<int>(m_offs / m_parameters.bytes_per_line);
    if (yPos >= m_pixbuf->get_height()) {
        std::cout << "Calculated pos "<< yPos << " >= " << m_pixbuf->get_height() << " avail ignoring." << std::endl;
        return;
    }
    auto column = 0; // m_offs % m_parameters.bytes_per_line;
    SANE_Byte* pixSrc = buf.data();
    auto pixData = reinterpret_cast<uint32_t*>(m_pixbuf->get_pixels()) + (yPos) * (m_pixbuf->get_rowstride() / sizeof(uint32_t));
    const auto bytePerPixel = m_parameters.format == SANE_FRAME_GRAY ? 1 : 3;
    const auto transferPixels = transferBytes / bytePerPixel;
    for (int x = column; x < column + transferPixels; ++x) {
        uint32_t rgb;                   //   Gdk   RGB?
        if (bytePerPixel >= 3) {        // discard alpha in case it is there...
            rgb = (pixSrc[2] << 16u) | (pixSrc[1] << 8u) |  pixSrc[0];
        }
        else {
            uint8_t gray;
            if (bytePerPixel == 1)  {   // grayscale
                gray = *pixSrc;
            }
            else {                      // b&w
                auto bit = pixSrc[x >> 3u] & (0x80u >> (x & 0x7u));
                gray = bit != 0 ? 0xffu : 0u;
            }
            rgb = (gray << 16u) | (gray << 8u) | gray;
        }
        //  pixbuf    A 31..24  R 23..16  G 15..8   B 7..0
        *pixData = 0xff000000u | rgb;
        pixSrc += bytePerPixel;
        ++pixData; // next pixel
    }

    m_scaled.reset();

    m_offs += transferBytes;
}

