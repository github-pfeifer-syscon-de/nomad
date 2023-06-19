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
#include <exception>

#include "ScanDlg.hpp"
#include "WiaScan.hpp"
#include "WiaProperty.hpp"
#include "NomadWin.hpp"

ScanDlg::ScanDlg(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::Dialog(cobject)
, m_nomadWin{nomadWin}
{
    auto icon = Gdk::Pixbuf::create_from_resource(m_nomadWin->get_application()->get_resource_base_path() + "/nomad.png");
    set_icon(icon);

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
                    m_scanPreview->scan(devId, properties);
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
        save->set_tooltip_text("Save (Png)");
        save->signal_clicked().connect(
        [this] () {
            try {
                NomadFileChooser file_chooser(*m_nomadWin, true, "png");
                if (file_chooser.run() == Gtk::ResponseType::RESPONSE_ACCEPT) {
                    if (!m_scanPreview->saveImage(file_chooser.get_filename())) {
                        m_nomadWin->show_error(Glib::ustring::sprintf("Unable to save file %s", file_chooser.get_filename()));
                    }
                }
            }
            catch (const std::exception &ex) {
                std::cerr << "Unable to activate save " << ex.what() << std::endl;
            }
        });
    }

    builder->get_widget("flow", m_flow);
    builder->get_widget_derived("previewImages", m_scanPreview);
    builder->get_widget("device", m_device);
    // Keep some infos on foreground
    WiaScan winScan;
    auto devs = winScan.getDevices();
    std::cout << "Devs " << devs.size() << std::endl;
    bool first{true};
    std::shared_ptr<WiaDevice> activeDev;
    for (auto dev : devs) {
        m_device->append(dev->getDeviceId(), dev->getDeviceName());
        if (first) {
            m_device->set_active_id(dev->getDeviceId());
            activeDev = dev;
        }
        first = false;
    }
    //m_action_color_depth = Gio::SimpleAction::create_radio_string("actionColorDepth", "color");
    builder->get_widget("radioColor", m_radioColor);
    //m_radioColor->join_group(m_action_color_depth);
    builder->get_widget("radioGray", m_radioGray);
    m_radioGray->join_group(*m_radioColor);
    m_radioColor->set_active(true);
    builder->get_widget("scaleBrightness", m_brightness);
    builder->get_widget("scaleContrast", m_contrast);
    builder->get_widget("scaleThreshold", m_threshold);
    builder->get_widget("resolution", m_resolution);

    IWiaItem *pChildWiaItem = nullptr;
    HRESULT hr = activeDev->findItem(activeDev->getWiaItem(), WiaItemTypeImage, &pChildWiaItem);
    if (SUCCEEDED(hr)) {
        IWiaPropertyStorage *pWiaPropertyStorage = NULL;
        hr = pChildWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
        if (SUCCEEDED(hr)) {
            setupScale(activeDev, pWiaPropertyStorage, property_brightness, m_brightness);
            setupScale(activeDev, pWiaPropertyStorage, property_contrast, m_contrast);
            setupScale(activeDev, pWiaPropertyStorage, property_threshold, m_threshold);
            setupSpinner(activeDev, pWiaPropertyStorage, property_resolution_x, m_resolution);
            // also read extends
            for (auto property : activeDev->getProperties()) {
                if (property->getPropertyId() == property_start_x) {
                    m_startX = property->getRange(pWiaPropertyStorage);
                }
                if (property->getPropertyId() == property_start_y) {
                    m_startY = property->getRange(pWiaPropertyStorage);
                }
                if (property->getPropertyId() == property_extend_x) {
                    m_extendX = property->getRange(pWiaPropertyStorage);
                }
                if (property->getPropertyId() == property_extend_y) {
                    m_extendY = property->getRange(pWiaPropertyStorage);
                }
            }
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

                        if (max < 255) {    // as it seems the range is interpreted differently?
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


std::map<uint32_t, WiaValue>
ScanDlg::getProperties(bool full)
{
    int colors = 8;
    if (m_radioColor->get_active()) {
        colors = 24;
    }
    WiaValue bits;
    bits.set(colors);
    WiaValue brightness;
    auto bright = static_cast<int32_t>(m_brightness->get_value());
    brightness.set(bright);
    WiaValue contrast;
    auto contr = static_cast<int32_t>(m_contrast->get_value());
    contrast.set(contr);
    WiaValue threshold;
    auto tresh = static_cast<int32_t>(m_threshold->get_value());
    threshold.set(tresh);

    auto dpi = full ? m_resolution->get_value_as_int() : 50;
    WiaValue horz;
    horz.set(dpi);
    WiaValue vert;
    vert.set(dpi);
    auto map = std::map<uint32_t, WiaValue>();
    map.insert(std::make_pair(property_resolution_x, horz));
    map.insert(std::make_pair(property_resolution_y, vert));
    map.insert(std::make_pair(property_bits, bits));
    map.insert(std::make_pair(property_brightness, brightness));
    map.insert(std::make_pair(property_contrast, contrast));
    map.insert(std::make_pair(property_threshold, threshold));
    if (full) {
        int xMaxExtend = 2000.0;
        if (m_extendX.size() >= 2) {
            xMaxExtend = std::get<int32_t>(m_extendX[0]);
        }
        int yMaxExtend = 3000.0;
        if (m_extendY.size() >= 2) {
            yMaxExtend = std::get<int32_t>(m_extendY[0]);
        }
        //std::cout << "xMax " << xMaxExtend
        //          << " yMax "  << yMaxExtend << std::endl;
        //std::cout << "Start " << m_scanPreview->getXStart()
        //          << " " << m_scanPreview->getYStart()
        //          << " end " << m_scanPreview->getXEnd()
        //          << " " << m_scanPreview->getYEnd() << std::endl;
        int x1 = static_cast<int32_t>(m_scanPreview->getXStart() * xMaxExtend);
        if (m_startX.size() >= 2) {
            WiaValue xStart;
            xStart.set(std::min(x1, std::get<int32_t>(m_startX[0])));
            map.insert(std::make_pair(property_start_x, xStart));
        }
        int y1 = static_cast<int32_t>(m_scanPreview->getYStart() * yMaxExtend);
        if (m_startY.size() >= 2) {
            WiaValue yStart;
            yStart.set(std::min(y1, std::get<int32_t>(m_startY[0])));
            map.insert(std::make_pair(property_start_y, yStart));
        }
        WiaValue xExtend;
        int width = static_cast<int32_t>((m_scanPreview->getXEnd() - m_scanPreview->getXStart()) * xMaxExtend);
        xExtend.set(std::min(width, xMaxExtend));
        map.insert(std::make_pair(property_extend_x, xExtend));
        WiaValue yExtend;
        int height = static_cast<int32_t>((m_scanPreview->getYEnd() - m_scanPreview->getYStart()) * yMaxExtend);
        yExtend.set(std::min(height, yMaxExtend));
        map.insert(std::make_pair(property_extend_y, yExtend));
        //std::cout << "Start " << x1 << " " << y1
        //          << " width " << width
        //          << " height " << height << std::endl;
    }

    return map;
}