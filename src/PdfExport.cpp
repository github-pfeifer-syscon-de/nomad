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
#include <setjmp.h>
#include <hpdf.h>

#include "PdfExport.hpp"
#include "PdfFont.hpp"

jmp_buf env;

static void
#ifdef HPDF_DLL
  __stdcall
#endif
error_handler(HPDF_STATUS   error_no,
                HPDF_STATUS   detail_no,
                void         *user_data)
{
    printf ("PdfOut ERROR: error_no=%04X, detail_no=%u\n", (HPDF_UINT)error_no,
                (HPDF_UINT)detail_no);
    longjmp(env, 1);
}

PdfExport::PdfExport()
{
    m_pdf = HPDF_New(error_handler, NULL);
    if (!m_pdf) {
        printf ("error: cannot create PdfDoc object\n");
        throw std::runtime_error("PdfExport::PdfExport");
    }

    /* error-handler */
    if (setjmp(env)) {
        HPDF_Free (m_pdf);
        throw std::runtime_error("PdfExport::PdfExport");
    }

    HPDF_SetCompressionMode(m_pdf, HPDF_COMP_ALL);
}

PdfExport::~PdfExport()
{
    if (m_pdf) {
        HPDF_Free(m_pdf);
        m_pdf = nullptr;
    }
}

PdfFormat
PdfExport::getFormat()
{
    return m_format;
}

void
PdfExport::setFormat(PdfFormat pdfFormat)
{
    m_format = pdfFormat;
}

HPDF_Doc
PdfExport::getDoc()
{
    return m_pdf;
}

void
PdfExport::save(const Glib::ustring& filename)
{
    /* save the document to a file */
    HPDF_SaveToFile (m_pdf, filename.c_str());
}

std::shared_ptr<PdfFont>
PdfExport::createFont(const Glib::ustring& fontName)
{
    return std::make_shared<PdfFont>(this, fontName);
}


float
PdfExport::mm2dot(float mm)
{
    //  72 dot per inch
    return mm / 25.4f * 72.f;
}

