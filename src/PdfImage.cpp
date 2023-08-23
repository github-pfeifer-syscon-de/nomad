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

#include <iostream>

#include "PdfImage.hpp"
#include "PdfExport.hpp"

PdfImage::PdfImage(std::shared_ptr<PdfExport>& pdfExport)
: m_pdfExport{pdfExport}
{
}

float
PdfImage::getWidth()
{
    return static_cast<float>(HPDF_Image_GetWidth(m_image));
}

float
PdfImage::getHeight()
{
    return static_cast<float>(HPDF_Image_GetHeight(m_image));
}

void
PdfImage::loadPng(const Glib::ustring& filename)
{
    auto sharedPdfExport = m_pdfExport.lock();
    if (sharedPdfExport) {
        HPDF_Doc pdf = sharedPdfExport->getDoc();
        m_image = HPDF_LoadPngImageFromFile(pdf, filename.c_str());
    }
}

HPDF_Image
PdfImage::getPdfImage()
{
    return m_image;
}