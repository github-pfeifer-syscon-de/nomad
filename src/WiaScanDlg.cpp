/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <unistd.h>
#include <StringUtils.hpp>

#include "nomad_config.h"
#include "WiaScanDlg.hpp"
#include "WiaScan.hpp"
#include "WiaProperty.hpp"
#include "NomadWin.hpp"

WiaScanDlg::WiaScanDlg(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: ScanDialog(cobject, builder, nomadWin)
, m_completed()
{
    m_completed.connect(sigc::mem_fun(*this, &WiaScanDlg::scanCompleted));

    if (m_preview) {
        m_preview->set_tooltip_text("Preview");
        m_preview->signal_clicked().connect(
        [this] () {
            try {
                auto properties = getProperties(false);
                auto devId = getDeviceId();
                if (!devId.empty()) {
                    m_wiaScanPreview->setShowMask(true);
                    m_wiaScanPreview->scan(devId, properties);
                    m_lastListRow = nullptr;
                }
            }
            catch (const Glib::Exception &ex) {
                std::cerr << "Unable to activate scan " << ex.what() << std::endl;
            }
        });
    }
    if (m_scan) {
        m_scan->set_tooltip_text("Scan");
        m_scan->signal_clicked().connect(
        [this] () {
            try {
                auto properties = getProperties(true);
                auto devId = getDeviceId();
                if (!devId.empty()) {
                    m_wiaScanPreview->setShowMask(false);
                    m_wiaScanPreview->scan(devId, properties);
                }
            }
            catch (const Glib::Exception &ex) {
                std::cerr << "Unable to activate scan " << ex.what() << std::endl;
            }
        });
    }
    builder->get_widget_derived("previewImages", m_wiaScanPreview, m_completed);
    m_scanPreview = m_wiaScanPreview;

    // Keep some infos on foreground
    m_wiaScan = std::make_shared<WiaScan>();
    auto devs = m_wiaScan->getDevices();
    //std::cout << "Devs " << devs.size() << std::endl;
    bool first{true};
    for (auto dev : devs) {
        m_device->append(dev->getDeviceId(), dev->getDeviceName());
        if (first) {
            m_device->set_active_id(dev->getDeviceId());
        }
        first = false;
    }
    m_device->signal_changed().connect(
            sigc::mem_fun(*this, &WiaScanDlg::deviceChanged));

    deviceChanged();
}

WiaScanDlg::~WiaScanDlg()
{
    cleanup();
}

void
WiaScanDlg::deviceChanged()
{
    std::shared_ptr<WiaDevice> activeDev = getActiveDevice();
    if (activeDev) {
        IWiaItem2 *pChildWiaItem = nullptr;
        HRESULT hr = activeDev->findItem(activeDev->getWiaItem(), WiaItemTypeImage, &pChildWiaItem);
        if (SUCCEEDED(hr)) {
            IWiaPropertyStorage *pWiaPropertyStorage = NULL;
            hr = pChildWiaItem->QueryInterface( IID_IWiaPropertyStorage, (void**)&pWiaPropertyStorage );
            if (SUCCEEDED(hr) && activeDev) {
                setupScale(activeDev, pWiaPropertyStorage, WiaDevice::PropertyBrightness, m_brightness);
                setupScale(activeDev, pWiaPropertyStorage, WiaDevice::PropertyContrast, m_contrast);
                //setupScale(activeDev, pWiaPropertyStorage, WiaDevice::PropertyThreshold, m_threshold);
                setupCombo(activeDev, pWiaPropertyStorage, WiaDevice::PropertyResolutionX, m_resolution);
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
WiaScanDlg::getDeviceId()
{
    return m_device->get_active_id();
}

void 
WiaScanDlg::setupCombo(
        const std::shared_ptr<WiaDevice>& activeDev
        , IWiaPropertyStorage *pWiaPropertyStorage
        , uint32_t propertyId
        , Gtk::ComboBoxText* combo)
{
    if (combo && activeDev) {
        for (auto property : activeDev->getProperties()) {
            if (property->getPropertyId() == propertyId) {
                auto values = property->getRange(pWiaPropertyStorage);
                if (values.size() >= 2) {
                    auto val = property->getValue(pWiaPropertyStorage);
                    int min = values[1].get<int32_t>();
                    int max = values[0].get<int32_t>();
                    min = std::max(min, 150);    // lowest values e.g.1 will be useless
                    while (min <= max) {
                        std::string id = psc::fmt::format("{}", min);
                        min *= 2;
                        combo->append(id, id);
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
WiaScanDlg::setupSpinner(
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
                    int min = values[1].get<int32_t>();
                    int max = values[0].get<int32_t>();
                    min = std::max(min, 50);    // lowest values e.g.1 will be useless
                    spinner->set_range(min, max);
                    spinner->set_value(val.get<int32_t>());
                    spinner->set_increments(50, 100);
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
WiaScanDlg::setupScale(
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
                    int min = values[1].get<int32_t>();
                    int max = values[0].get<int32_t>();

                    if (max < 255) {    // as it seems the range is interpreted differently for brightness&contrast
                        min = -max;
                    }
                    scale->set_range(min, max);
                    scale->set_value(val.get<int32_t>());
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
WiaScanDlg::getActiveDevice()
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

std::map<uint32_t, int32_t>
WiaScanDlg::getProperties(bool full)
{
    std::shared_ptr<WiaDevice> activeDevice = getActiveDevice();
    if (activeDevice) {
        auto bright = static_cast<int32_t>(m_brightness->get_value());
        auto contr = static_cast<int32_t>(m_contrast->get_value());
        //auto tresh = static_cast<int32_t>(m_threshold->get_value());
        auto tresh = 128;
        auto resId = m_resolution->get_active_id();        
        auto res = std::stoi(resId);
        int32_t bits = 8;
        if (m_radioColor->get_active()) {
            bits = 24;
        }
        else if(m_radioGray->get_active()) {
            bits = 8;
        }
        //else if(m_radioBW->get_active()) {
        //    bits = 1;
        //}
        return activeDevice->buildScanProperties(
                full
                , bright
                , contr
                , tresh
                , res
                , bits
                , m_wiaScanPreview->getXStart()
                , m_wiaScanPreview->getYStart()
                , m_wiaScanPreview->getXEnd()
                , m_wiaScanPreview->getYEnd());
    }
    return std::map<uint32_t, int32_t>();
}

void
WiaScanDlg::scanCompleted()
{
    std::cout << "ScanDlg::scanCompleted" << std::endl;
    bool result = m_wiaScanPreview->getResult();
    bool preview = m_wiaScanPreview->getShowMask();
    if (!result) {
        Gtk::MessageDialog messagedialog(*m_nomadWin, "Error scanning!", FALSE, Gtk::MessageType::MESSAGE_WARNING);

        messagedialog.run();
        messagedialog.hide();
    }
    if (!preview) {
        appendScanPage();
    }
    m_wiaScanPreview->cleanup();   // release resources
}
