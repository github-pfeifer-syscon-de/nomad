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
#include "ScanPreview.hpp"

class SaneScanPreview
: public ScanPreview
, public SaneScanParamNotify
{
public:
    SaneScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Glib::Dispatcher& completed);
    explicit SaneScanPreview(const SaneScanPreview &other) = delete;
    virtual ~SaneScanPreview() = default;
    void setParameter(const SANE_Parameters& parameters) override;
    void append(std::vector<SANE_Byte>& buf, SANE_Int len) override;
protected:

private:
    Glib::Dispatcher& m_completed;
    SANE_Parameters m_parameters;
    size_t m_offs{};
};