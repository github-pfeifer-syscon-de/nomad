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

#include <gtkmm.h>
#include <hpdf.h>
#include <memory>

class PdfExport;
class PdfFont;
class PdfImage;

class PdfPage
{
public:
    PdfPage(std::weak_ptr<PdfExport> pdfExport);
    explicit PdfPage(const PdfPage& orig) = delete;
    virtual ~PdfPage() = default;

    void drawText(const Glib::ustring& text, float x, float y);
    void drawImage(std::shared_ptr<PdfImage>& image, float x, float y, float w, float h);
    float getHeight();
    float getWidth();
    void setFont(std::shared_ptr<PdfFont>& font, float size);

private:
    std::weak_ptr<PdfExport> m_pdfExport;
    HPDF_Destination m_dst{nullptr};
    HPDF_Page m_page{nullptr};
    std::shared_ptr<PdfFont> m_font;
    float m_fontSize{12.0};
};


