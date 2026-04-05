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
#include <ImageFileChooser.hpp>
#include <StringUtils.hpp>
#include <cstdio>
#include "nomad_config.h"
#ifdef USE_PDF
#include <PdfExport.hpp>
#include <PdfPage.hpp>
#include <PdfFont.hpp>
#include <PdfImage.hpp>
#endif

#include "NomadWin.hpp"
#include "ScanDialog.hpp"
#include "SaneScanDialog.hpp"


namespace std
{
    using ::close;      // see <cstdio> move these to a namespace
}


ScanDialog::ScanDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::Dialog(cobject)
, m_nomadWin{nomadWin}
{
    auto icon = Gdk::Pixbuf::create_from_resource(m_nomadWin->get_application()->get_resource_base_path() + "/nomad.png");
    set_icon(icon);

    builder->get_widget("list", m_list);
    builder->get_widget("device", m_device);
    builder->get_widget("radioColor", m_radioColor);
    builder->get_widget("radioGray", m_radioGray);
    m_radioGray->join_group(*m_radioColor);
    m_radioColor->set_active(true);
    builder->get_widget("scaleBrightness", m_brightness);
    builder->get_widget("scaleContrast", m_contrast);
    builder->get_widget("resolution", m_resolution);

    builder->get_widget("preview", m_preview);
    builder->get_widget("scan", m_scan);

    builder->get_widget<Gtk::Button>("save", m_save);
    if (m_save) {
        m_save->set_tooltip_text("Save");
        m_save->signal_clicked().connect(
        [this] () {
            try {
                std::vector<std::string> types;
                types.push_back("png");
                #ifdef USE_PDF
                types.push_back("pdf");
                #endif
                ImageFileChooser file_chooser(*m_nomadWin, true, types);
                if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                    if (!saveImage(file_chooser.get_filename())) {
                        m_nomadWin->show_error(Glib::ustring::sprintf("Unable to save file %s", file_chooser.get_filename()));
                    }
                }
            }
            catch (const std::exception &ex) {
                std::cerr << "Unable to activate save " << ex.what() << std::endl;
            }
        });
    }

}

bool
ScanDialog::saveImage(const Glib::ustring& file)
{
    bool ret = false;
    if (StringUtils::endsWith(file, ".png")) {
        ret = m_scanPreview->savePng(file);
    }
#ifdef USE_PDF
    else if(StringUtils::endsWith(file, ".pdf")) {
        exportPdf(file);
        ret = true;
    }
#endif
    else {
        std::cout << "Cannot handle " << file << " expecting e.g. .png!" << std::endl;
    }
    return ret;
}

#ifdef USE_PDF
void
ScanDialog::exportPdf(const Glib::ustring& file)
{
    auto pdfExport = std::make_shared<psc::pdf::PdfExport>();
    auto helv = pdfExport->createFont(psc::pdf::PdfExport::BASE14FONT_HELVECTICA);
    //page->drawText("PngDemo", 220, page->getHeight() - 70);
    //page->setFont(helv, 12);

    //std::string tempName;
    //int h = Glib::file_open_tmp(tempName, "temp");
    //close(h);
    //savePng(tempName);
    for (auto pngImg : m_pages) {
        auto page = pdfExport->createPage();
        page->setFont(helv);
        auto img = std::make_shared<psc::pdf::PdfImage>(pdfExport);
        img->loadPng(pngImg->get_path());
        std::cout << " width " << img->getWidth()
                  << " height " << img->getHeight()
                  << " PngImage " << pngImg->get_path()
                  << std::endl;
        float pageAvailWidth = page->getWidth() - 100.0f;
        float pageAvailHeight = page->getHeight() - 100.0f;
        float imageWidth = img->getWidth();
        float imageHeight = img->getHeight();
        float relWidth = pageAvailWidth / imageWidth;
        float relHeight = pageAvailHeight / imageHeight;
        float scale = std::min(relWidth, relHeight);
        float scaledWidth = imageWidth * scale;
        float scaledHeight = imageHeight * scale;
        page->drawImage(img,
                50.0f, 50.0f,
                scaledWidth, scaledHeight);
    }
            //page->getWidth() - 100.0f, page->getHeight() - 100.0f);
//    page->drawPng("res/basn0g01.png", 100, page->getHeight() - 150);
//    page->drawText("1bit grayscale.\nbasn0g01.png", 100, page->getHeight() - 150);
//    page->drawPng("res/basn0g02.png", 200, page->getHeight() - 150);
//    page->drawText("2bit grayscale.\nbasn0g02.png", 200, page->getHeight() - 150);
//    page->drawPng("res/basn0g04.png", 300, page->getHeight() - 150);
//    page->drawText("4bit grayscale.\nbasn0g04.png", 300, page->getHeight() - 150);
//    page->drawPng("res/basn0g08.png", 400, page->getHeight() - 150);
//    page->drawText("8bit grayscale.\nbasn0g08.png", 400, page->getHeight() - 150);
    pdfExport->save(file);

    cleanup();
    //auto tfile = Gio::File::create_for_path(tempName);
    //tfile->remove();
}
#endif


void
ScanDialog::cleanup()
{
    while (!m_pages.empty()) {
        auto file = m_pages.back();
        file->remove();
        m_pages.pop_back();
    }
}

void
ScanDialog::appendScanPage()
{
    if (m_lastListRow == nullptr) {
        m_lastListRow = Gtk::make_managed<Gtk::ListBoxRow>();
        m_list->append(*m_lastListRow);
    }
    else {
        if (!m_pages.empty()) {
            auto file = m_pages.back();
            file->remove();
            m_pages.pop_back();
        }
        m_lastListRow->remove();    // clear previous
    }
    auto pixbuf = m_scanPreview->getPixbuf();
    if (pixbuf) {
        float width = static_cast<float>(pixbuf->get_width());
        float height = static_cast<float>(pixbuf->get_height());
        float scale = 64.0f / std::max(width, height);
        auto preview = pixbuf->scale_simple(static_cast<int>(scale * width), static_cast<int>(scale * height), Gdk::InterpType::INTERP_BILINEAR);
        auto image = Gtk::make_managed<Gtk::Image>(preview);
        m_lastListRow->add(*image);

        std::string tempName;
        int h = Glib::file_open_tmp(tempName, "savedPng");
        std::close(h);
        m_scanPreview->savePng(tempName);
        m_pages.push_back(Gio::File::create_for_path(tempName));
    }
    m_lastListRow->show_all();
}