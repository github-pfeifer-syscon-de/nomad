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
#include <psc_format.hpp>

#include "SaneScanDialog.hpp"
#include "NomadWin.hpp"
#include "SaneScanDevice.hpp"


SaneScanDialog::SaneScanDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: ScanDialog(cobject, builder, nomadWin)
{
    m_preview->signal_clicked().connect(sigc::mem_fun(*this, &SaneScanDialog::preview));
    m_scan->signal_clicked().connect(sigc::mem_fun(*this, &SaneScanDialog::scan));
    builder->get_widget_derived("previewImages", m_saneScanPreview, m_completed);
    m_scanPreview = m_saneScanPreview;

    auto config = nomadWin->getConfig();
    SANE_Status sane_status = SaneScanDevice::load_sanelib(config->getSanePath());
    bool first{true};
    if (sane_status == SANE_STATUS_GOOD) {
        try {
            m_scanDevices = SaneScanDevice::devices();
            for (auto& saneDevice : m_scanDevices) {
                m_device->append(saneDevice->getId(), saneDevice->getName());
                if (first) {
                    m_device->set_active_id(saneDevice->getId());
                    first = false;
                }
            }
        }
        catch (const SaneException& exc) {
            m_nomadWin->show_error(psc::fmt::format("Error {} sane list", exc.what()));
        }
    }
    else {
        m_nomadWin->show_error(psc::fmt::format("Error {} sane init", static_cast<int>(sane_status)));
        return;
    }
    m_device->signal_changed().connect(
            sigc::mem_fun(*this, &SaneScanDialog::deviceChanged));

    if (!first) {
        deviceChanged();
    }
}

SaneScanDialog::~SaneScanDialog()
{
    m_scanDevices.clear();  // close devices
    std::cout << "Sane exit " << std::endl;
    SaneScanDevice::unload_sanelib();
}


void
SaneScanDialog::setupMode(
        const std::shared_ptr<SaneStringOption>& modeOption
      , const Glib::ustring& confMode)
{
    if (modeOption) {
        auto optConstr= modeOption->getConstraints();
        auto strConstr = std::dynamic_pointer_cast<SaneConstraintDiscreteString>(optConstr);
        bool hasColor{};
        bool hasGray{};
        if (strConstr) {
            for (auto optValue : strConstr->getValues()) {
                //std::cout << "Scanning mode options " << optValue << std::endl;
                if (optValue == SaneStringOption::MODE_COLOR) {
                    hasColor = true;
                }
                else if (optValue == SaneStringOption::MODE_GRAY) {
                    hasGray = true;
                }
            }
            m_radioColor->set_sensitive(hasColor);
            m_radioGray->set_sensitive(hasGray);
        }
        else {
            std::cout << "SaneScanDialog::deviceChanged unexpected constraint expected SaneConstraintDiscreteString" << std::endl;
        }
        std::string mode;
        if (confMode.empty()) {
             mode = modeOption->getStrValue();
        }
        else {
            mode = confMode;
        }
        //std::cout << "Scanning mode \"" << mode << "\"" << std::endl;
        if (hasColor && mode == SaneStringOption::MODE_COLOR) {
            m_radioColor->set_active(true);
        }
        else if (hasGray && mode == SaneStringOption::MODE_GRAY) {
            m_radioGray->set_active(true);
        }
    }
}

void
SaneScanDialog::setupAdjustment(
        const std::shared_ptr<SaneIntOption>& modeOption
      , Gtk::Scale* scale
      , int configValue)
{
    if (modeOption) {
        auto constrRange = std::dynamic_pointer_cast<SaneConstraintRange>(modeOption->getConstraints());
        if (constrRange) {
            auto adjust = Gtk::Adjustment::create(
                modeOption->getIntValue()
                ,constrRange->getMin()
                ,constrRange->getMax()
                ,constrRange->getQuant());
            scale->set_adjustment(adjust);
            if (configValue >= constrRange->getMin()
             && configValue <= constrRange->getMax()) {
                scale->set_value(configValue);
            }
        }
        else {
            std::cout << "SaneScanDialog::deviceChanged option constr not range" << std::endl;
        }
    }
    else {
        std::cout << "SaneScanDialog::deviceChanged no option " << std::endl;
    }
}

void
SaneScanDialog::setupResolution(const std::shared_ptr<SaneIntOption>& resultion, Gtk::ComboBoxText* resolutionCombo)
{
    resolutionCombo->remove_all();
    if (resultion) {
        auto constrDiscInt = std::dynamic_pointer_cast<SaneConstraintDiscreteInt>(resultion->getConstraints());
        if (constrDiscInt) {
            auto values = constrDiscInt->getValues();
            for (auto n : values) {
                auto id = psc::fmt::format("{}", n);
                resolutionCombo->append(id, id);
            }
            auto value = resultion->getIntValue();
            auto id = psc::fmt::format("{}", value);
            resolutionCombo->set_active_id(id);
        }
        else {
            std::cout << "SaneScanDialog::setupResolution option constr not discr int" << std::endl;
        }
    }
    else {
        std::cout << "SaneScanDialog::deviceChanged no option " << std::endl;
    }
}

void
SaneScanDialog::deviceChanged()
{
    try {
        if (m_scanDevice) {
            m_scanDevice->close();
        }
        auto activeId = m_device->get_active_id();
        if (!activeId.empty()) {
            std::cout << "sane active id " << activeId << std::endl;
            for (auto& saneDev : m_scanDevices) {
                if (saneDev->getId() == activeId) {
                    m_scanDevice = saneDev;
                    break;
                }
            }
            if (m_scanDevice) {
                auto modeOption = m_scanDevice->getStrOption(SaneScanDevice::MODE_OPTION);
                setupMode(modeOption, m_nomadWin->getConfig()->getScanMode());
                auto brightness = m_scanDevice->getIntOption(SaneScanDevice::BRIGHTNESS_OPTION);
                setupAdjustment(brightness, m_brightness, m_nomadWin->getConfig()->getScanBrightness());
                auto contrast = m_scanDevice->getOption<SaneIntOption>(SaneScanDevice::CONTRAST_OPTION);
                setupAdjustment(contrast, m_contrast, m_nomadWin->getConfig()->getScanContrast());
                auto resolution = m_scanDevice->getOption<SaneIntOption>(SaneScanDevice::RESOLUTION_OPTION);
                setupResolution(resolution, m_resolution);
                //auto options = scanDevice->getOptions();
            }
            else {
                std::cout << "No device found for " << activeId << "!" << std::endl;
            }
        }
    }
    catch (const SaneException& exc) {
        m_nomadWin->show_error(psc::fmt::format("Error {} sane options", exc.what()));
    }
}

void
SaneScanDialog::setParameters(bool scan)
{
    auto previewOpt = m_scanDevice->getBoolOption(SaneScanDevice::PREVIEW_OPTION);
    if (previewOpt) {
        previewOpt->setBoolValue(!scan);
    }
    auto resOpt =  m_scanDevice->getIntOption(SaneScanDevice::RESOLUTION_OPTION);
    SANE_Int res{150};
    if (scan) {
        auto ress= m_resolution->get_active_id();
        res = std::stoi(ress);
    }
    else {
        auto resConstr = resOpt->getConstraints();
        auto resDiscInt = std::dynamic_pointer_cast<SaneConstraintDiscreteInt>(resConstr);
        if (resDiscInt) {
            res = resDiscInt->getValues()[0];
        }
    }
    resOpt->setIntValue(res);
    auto tx = m_scanDevice->getFixedOption(SaneScanDevice::TOPLEFTX_OPTION);
    auto ty = m_scanDevice->getFixedOption(SaneScanDevice::TOPLEFTY_OPTION);
    auto bx = m_scanDevice->getFixedOption(SaneScanDevice::BOTTOMRIGHTX_OPTION);
    auto by = m_scanDevice->getFixedOption(SaneScanDevice::BOTTOMRIGHTY_OPTION);
    auto xconstr = std::dynamic_pointer_cast<SaneConstraintRange>(tx->getConstraints());
    auto yconstr = std::dynamic_pointer_cast<SaneConstraintRange>(ty->getConstraints());
    if (!xconstr || !yconstr) {
        // convert percent into coords
        std::cout << "SaneScanDialog::setParameters the top/bottom coord constraints are not of expected type SaneConstraintRange" << std::endl;
        return;
    }
    if (scan && m_saneScanPreview->getShowMask()) {
        auto xs = xconstr->getMin() + m_saneScanPreview->getXStart() * xconstr->getMax();
        auto ys = yconstr->getMin() + m_saneScanPreview->getYStart() * yconstr->getMax();
        auto xe = xconstr->getMin() + m_saneScanPreview->getXEnd() * xconstr->getMax();
        auto ye = yconstr->getMin() + m_saneScanPreview->getYEnd() * yconstr->getMax();
        tx->setFixValue(xs);
        ty->setFixValue(ys);
        bx->setFixValue(xe);
        by->setFixValue(ye);
    }
    else {
        tx->setFixValue(xconstr->getMin());
        ty->setFixValue(yconstr->getMin());
        bx->setFixValue(xconstr->getMax());
        by->setFixValue(yconstr->getMax());
    }
    bool color = m_radioColor->get_active();
    //bool gray = m_radioGray->get_active();
    std::string mode = SaneStringOption::MODE_GRAY;
    if (color) {
        mode = SaneStringOption::MODE_COLOR;
    }
    m_nomadWin->getConfig()->setScanMode(mode);
    auto modeOpt = m_scanDevice->getStrOption(SaneScanDevice::MODE_OPTION);
    modeOpt->setStrValue(mode);
    auto brightVal = static_cast<SANE_Int>(m_brightness->get_value());
    m_nomadWin->getConfig()->setScanBrightness(brightVal);
    auto brightOpt = m_scanDevice->getIntOption(SaneScanDevice::BRIGHTNESS_OPTION);
    brightOpt->setIntValue(brightVal);
    auto contrastVal = static_cast<SANE_Int>(m_contrast->get_value());
    m_nomadWin->getConfig()->setScanContrast(contrastVal);
    auto contrOpt = m_scanDevice->getIntOption(SaneScanDevice::CONTRAST_OPTION);
    contrOpt->setIntValue(contrastVal);
    m_nomadWin->getConfig()->saveConfig();
}

void
SaneScanDialog::preview()
{
    try {
        if (m_scanDevice) {
            setParameters(false);
            m_scanDevice->transfer(m_saneScanPreview);
            m_saneScanPreview->queue_draw();
            m_saneScanPreview->setShowMask(true);
        }
        else {
            m_nomadWin->show_error("Preview no device found");
        }
    }
    catch (const SaneException& exc) {
        m_nomadWin->show_error(psc::fmt::format("Error {} sane preview", exc.what()));
    }
}

void
SaneScanDialog::scan()
{
    try {
        if (m_scanDevice) {
            setParameters(true);
            m_saneScanPreview->setShowMask(false);
            m_scanDevice->transfer(m_saneScanPreview);
            m_saneScanPreview->queue_draw();
            appendScanPage();
        }
        else {
            m_nomadWin->show_error("Scan no device found");
        }
    }
    catch (const SaneException& exc) {
        m_nomadWin->show_error(psc::fmt::format("Error {} sane scan", exc.what()));
    }
}
