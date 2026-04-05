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
#pragma once

#include <gtkmm.h>
#include <vector>

#include "SaneScanDevice.hpp"
#include "SaneScanPreview.hpp"
#include "ScanDialog.hpp"

class NomadWin;

class SaneScanDialog
: public ScanDialog
{
public:
    SaneScanDialog(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin);
    explicit SaneScanDialog(const SaneScanDialog &other) = delete;
    virtual ~SaneScanDialog();

protected:
    void deviceChanged();
    void setupMode(const std::shared_ptr<SaneStringOption>& modeOption);
    void setupAdjustment(const std::shared_ptr<SaneIntOption>& modeOption, Gtk::Scale* scale);
    void setupResolution(const std::shared_ptr<SaneIntOption>& resultion, Gtk::ComboBoxText* resolutionCombo);
    void preview();
    void scan();
    /**
     * @param scan true for scan, false for preview
     */
    void setParameters(bool scan);

private:
    SaneScanPreview* m_saneScanPreview;
    Glib::Dispatcher m_completed;
    std::vector<std::shared_ptr<SaneScanDevice>> m_scanDevices;
    std::shared_ptr<SaneScanDevice> m_scanDevice;
};