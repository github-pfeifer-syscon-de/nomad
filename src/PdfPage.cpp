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

#include <StringUtils.hpp>

#include "PdfPage.hpp"
#include "PdfExport.hpp"
#include "PdfFont.hpp"

PdfPage::PdfPage(std::weak_ptr<PdfExport> pdfExport)
: m_pdfExport{pdfExport}
{
    auto sharedPdfExport = pdfExport.lock();
    if (sharedPdfExport) {
        HPDF_Doc pdf = sharedPdfExport->getDoc();
        /* add a new page object. */
        m_page = HPDF_AddPage (pdf);

        float width = 550.0f;
        float height = 650.0f;
        PdfFormat fmt = sharedPdfExport->getFormat();
        switch (fmt) {
            case PdfFormat::A4:
                width = PdfExport::mm2dot(210);
                height = PdfExport::mm2dot(297);
                break;
            default:
            case PdfFormat::Letter:
                width = 550.0f;
                height = 650.0f;
                break;
        }
        HPDF_Page_SetWidth(m_page, width);
        HPDF_Page_SetHeight(m_page, height);

        m_dst = HPDF_Page_CreateDestination(m_page);
        HPDF_Destination_SetXYZ(m_dst, 0, HPDF_Page_GetHeight(m_page), 1);
        HPDF_SetOpenAction(pdf, m_dst);
    }
}

void
PdfPage::setFont(std::shared_ptr<PdfFont>& font, float size)
{
    m_font = font;
    m_fontSize = size;
}

float
PdfPage::getHeight()
{
    return HPDF_Page_GetHeight(m_page);
}

float
PdfPage::getWidth()
{
    return HPDF_Page_GetWidth(m_page);
}

void
PdfPage::drawPng(const Glib::ustring& filename, float x, float y)
{
    auto sharedPdfExport = m_pdfExport.lock();
    if (sharedPdfExport) {
        HPDF_Doc pdf = sharedPdfExport->getDoc();
        HPDF_Image image = HPDF_LoadPngImageFromFile(pdf, filename.c_str());

        /* Draw image to the canvas. */
        HPDF_Page_DrawImage (
                m_page,
                image,
                x, y,
                static_cast<HPDF_REAL>(HPDF_Image_GetWidth(image)),
                static_cast<HPDF_REAL>(HPDF_Image_GetHeight(image)));
    }
}

void
PdfPage::drawText(const Glib::ustring& text, float x, float y)
{
    /* Print the text. */
    HPDF_Page_BeginText (m_page);
    if (m_font) {
        HPDF_Page_SetFontAndSize(m_page, m_font->getPdfFont(), m_fontSize);
        HPDF_Page_SetTextLeading(m_page, m_fontSize * 1.333333f);
    }
    HPDF_Page_MoveTextPos(m_page, x, y);
    std::vector<Glib::ustring> lines;
    StringUtils::split(text, '\n', lines);
    for (auto line : lines) {
        HPDF_Page_ShowTextNextLine (m_page, line.c_str());
    }
    HPDF_Page_EndText (m_page);
}