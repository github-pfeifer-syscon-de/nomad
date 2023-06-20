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

#pragma once

#include <gtkmm.h>
#include <hpdf.h>
#include <memory>

enum class PdfFormat
{
    A4,
    Letter
};

class PdfPage;
class PdfFont;

class PdfExport
{
public:
    PdfExport();
    explicit PdfExport(const PdfExport& orig) = delete;
    virtual ~PdfExport();

    void save(const Glib::ustring& filename);
    HPDF_Doc getDoc();
    static float mm2dot(float mm);
    PdfFormat getFormat();
    void setFormat(PdfFormat pdfFormat);
    std::shared_ptr<PdfFont> createFont(const Glib::ustring& fontName);
protected:

private:
    HPDF_Doc m_pdf{nullptr};
    PdfFormat m_format{PdfFormat::A4};
};

