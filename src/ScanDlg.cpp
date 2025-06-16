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
#include <exception>
#include <vector>
#include <StringUtils.hpp>
#include <unistd.h>
#ifdef USE_PDF
#include <PdfExport.hpp>
#include <PdfPage.hpp>
#include <PdfFont.hpp>
#include <PdfImage.hpp>
#endif

namespace std
{
    using ::close;      // see <cstdio> move these to a namespace
}

#include "config.h"
#include "ScanDlg.hpp"
#include "WiaScan.hpp"
#include "WiaProperty.hpp"
#include "NomadWin.hpp"

ScanDlg::ScanDlg(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::Dialog(cobject)
, m_nomadWin{nomadWin}
, m_completed()
{
    auto icon = Gdk::Pixbuf::create_from_resource(m_nomadWin->get_application()->get_resource_base_path() + "/nomad.png");
    set_icon(icon);
    m_completed.connect(sigc::mem_fun(*this, &ScanDlg::scanCompleted));

    Gtk::Button* preview = nullptr;
    builder->get_widget<Gtk::Button>("preview", preview);
    if (preview) {
        preview->set_tooltip_text("Preview");
        preview->signal_clicked().connect(
        [this] () {
            try {
                auto properties = getProperties(false);
                auto devId = getDeviceId();
                if (!devId.empty()) {
                    m_scanPreview->setShowMask(true);
                    m_scanPreview->scan(devId, properties);
                    m_lastListRow = nullptr;
                }
            }
            catch (const Glib::Exception &ex) {
                std::cerr << "Unable to activate scan " << ex.what() << std::endl;
            }
        });
    }
    Gtk::Button* scan = nullptr;
    builder->get_widget<Gtk::Button>("scan", scan);
    if (scan) {
        scan->set_tooltip_text("Scan");
        scan->signal_clicked().connect(
        [this] () {
            try {
                auto properties = getProperties(true);
                auto devId = getDeviceId();
                if (!devId.empty()) {
                    m_scanPreview->setShowMask(false);
                    m_scanPreview->scan(devId, properties);
                }
            }
            catch (const Glib::Exception &ex) {
                std::cerr << "Unable to activate scan " << ex.what() << std::endl;
            }
        });
    }
    Gtk::Button* save = nullptr;
    builder->get_widget<Gtk::Button>("save", save);
    if (save) {
        save->set_tooltip_text("Save");
        save->signal_clicked().connect(
        [this] () {
            try {
                std::vector<Glib::ustring> types;
                types.push_back("png");
                #ifdef USE_PDF
                types.push_back("pdf");
                #endif
                NomadFileChooser file_chooser(*m_nomadWin, true, types);
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

    builder->get_widget("list", m_list);
    builder->get_widget_derived("previewImages", m_scanPreview, m_completed);
    builder->get_widget("device", m_device);
    // Keep some infos on foreground
    m_wiaScan = std::make_shared<WiaScan>();
    auto devs = m_wiaScan->getDevices();
    std::cout << "Devs " << devs.size() << std::endl;
    bool first{true};
    for (auto dev : devs) {
        m_device->append(dev->getDeviceId(), dev->getDeviceName());
        if (first) {
            m_device->set_active_id(dev->getDeviceId());
        }
        first = false;
    }
    m_device->signal_changed().connect(
            sigc::mem_fun(*this, &ScanDlg::deviceChanged));
    builder->get_widget("radioColor", m_radioColor);
    builder->get_widget("radioGray", m_radioGray);
    m_radioGray->join_group(*m_radioColor);
    builder->get_widget("radioBW", m_radioBW);
    m_radioBW->join_group(*m_radioColor);
    m_radioColor->set_active(true);
    builder->get_widget("scaleBrightness", m_brightness);
    builder->get_widget("scaleContrast", m_contrast);
    builder->get_widget("scaleThreshold", m_threshold);
    builder->get_widget("resolution", m_resolution);

    deviceChanged();
}

ScanDlg::~ScanDlg()
{
    cleanup();
}

void
ScanDlg::cleanup()
{
    while (!m_pages.empty()) {
        auto file = m_pages.back();
        file->remove();
        m_pages.pop_back();
    }
}

bool
ScanDlg::saveImage(const Glib::ustring& file)
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
ScanDlg::exportPdf(const Glib::ustring& file)
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
        auto page = std::make_shared<psc::pdf::PdfPage>(pdfExport);
        page->setFont(helv, 12);
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
ScanDlg::deviceChanged()
{
    std::shared_ptr<WiaDevice> activeDev = getActiveDevice();
    if (activeDev) {
        IWiaItem *pChildWiaItem = nullptr;
        HRESULT hr = activeDev->findItem(activeDev->getWiaItem(), WiaItemTypeImage, &pChildWiaItem);
        if (SUCCEEDED(hr)) {
            IWiaPropertyStorage *pWiaPropertyStorage = NULL;
            hr = pChildWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
            if (SUCCEEDED(hr) && activeDev) {
                setupScale(activeDev, pWiaPropertyStorage, WiaDevice::PropertyBrightness, m_brightness);
                setupScale(activeDev, pWiaPropertyStorage, WiaDevice::PropertyContrast, m_contrast);
                setupScale(activeDev, pWiaPropertyStorage, WiaDevice::PropertyThreshold, m_threshold);
                setupSpinner(activeDev, pWiaPropertyStorage, WiaDevice::PropertyResolutionX, m_resolution);
                // also read extends as we have the storage around
                activeDev->readExtends(pWiaPropertyStorage);
            }
            else {
                std::cout << "ScanDlg::ScanDlg image item not convertible PropertyStorage!" << std::endl;
            }
            pChildWiaItem->Release();
            pChildWiaItem = nullptr;
        }
        else {
            std::cout << "ScanDlg::ScanDlg no image item!" << std::endl;
        }
    }
}

Glib::ustring
ScanDlg::getDeviceId()
{
    return m_device->get_active_id();
}

void
ScanDlg::setupSpinner(
        const std::shared_ptr<WiaDevice>& activeDev
        , IWiaPropertyStorage *pWiaPropertyStorage
        , uint32_t propertyId
        , Gtk::SpinButton* spinner)
{
    if (spinner && activeDev) {
        for (auto property : activeDev->getProperties()) {
            if (property->getPropertyId() == propertyId) {
                auto values = property->getRange(pWiaPropertyStorage);
                if (values.size() >= 2) {
                    auto val = property->getValue(pWiaPropertyStorage);
                    if (std::holds_alternative<int32_t>(val)) {
                        int min = std::get<int32_t>(values[1]);
                        int max = std::get<int32_t>(values[0]);

                        min = std::max(min, 50);    // lowest values e.g.1 will be useless
                        spinner->set_range(min, max);
                        spinner->set_value(std::get<int32_t>(val));
                        spinner->set_increments(50, 100);
                    }
                    else {
                        std::cout << "ScanDlg::setupSpinner wrong type " << val.getVt() << std::endl;
                    }
                }
                break;
            }
        }
    }
    else {
        std::cout << "ScanDlg::setupSpinner component missing!" << std::endl;
    }
}

void
ScanDlg::setupScale(
        const std::shared_ptr<WiaDevice>& activeDev
        , IWiaPropertyStorage *pWiaPropertyStorage
        , uint32_t propertyId
        , Gtk::Scale* scale)
{
    if (scale && activeDev) {
        for (auto property : activeDev->getProperties()) {
            if (property->getPropertyId() == propertyId) {
                auto values = property->getRange(pWiaPropertyStorage);
                if (values.size() >= 2) {
                    auto val = property->getValue(pWiaPropertyStorage);
                    if (std::holds_alternative<int32_t>(val)) {
                        int min = std::get<int32_t>(values[1]);
                        int max = std::get<int32_t>(values[0]);

                        if (max < 255) {    // as it seems the range is interpreted differently for brightness&contrast
                            min = -max;
                        }
                        scale->set_range(min, max);
                        scale->set_value(std::get<int32_t>(val));
                    }
                    else {
                        std::cout << "ScanDlg::setupScale wrong type " << val.getVt() << std::endl;
                    }
                }
                break;
            }
        }
    }
    else {
        std::cout << "ScanDlg::setupScale component missing!" << std::endl;
    }
}


std::shared_ptr<WiaDevice>
ScanDlg::getActiveDevice()
{
    auto devId = getDeviceId();
    if (!devId.empty()) {
        auto devs = m_wiaScan->getDevices();
        for (auto dev : devs) {
            if (devId == dev->getDeviceId()) {
                return dev;
            }
        }
    }
    return nullptr;
}

std::map<uint32_t, WiaValue>
ScanDlg::getProperties(bool full)
{
    std::shared_ptr<WiaDevice> activeDevice = getActiveDevice();
    if (activeDevice) {
        auto bright = static_cast<int32_t>(m_brightness->get_value());
        auto contr = static_cast<int32_t>(m_contrast->get_value());
        auto tresh = static_cast<int32_t>(m_threshold->get_value());
        auto res = m_resolution->get_value_as_int();
        int32_t bits = 8;
        if (m_radioColor->get_active()) {
            bits = 24;
        }
        else if(m_radioGray->get_active()) {
            bits = 8;
        }
        else if(m_radioBW->get_active()) {
            bits = 1;
        }
        return activeDevice->buildScanProperties(
                full
                , bright
                , contr
                , tresh
                , res
                , bits
                , m_scanPreview->getXStart()
                , m_scanPreview->getYStart()
                , m_scanPreview->getXEnd()
                , m_scanPreview->getYEnd());
    }
    return std::map<uint32_t, WiaValue>();
}

void
ScanDlg::scanCompleted()
{
    std::cout << "ScanDlg::scanCompleted" << std::endl;
    bool result = m_scanPreview->getResult();
    bool preview = m_scanPreview->getShowMask();
    if (!result) {
        Gtk::MessageDialog messagedialog(*m_nomadWin, "Error scanning!", FALSE, Gtk::MessageType::MESSAGE_WARNING);

        messagedialog.run();
        messagedialog.hide();
    }
    if (!preview) {
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
    m_scanPreview->cleanup();   // release resources
}
