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

#pragma once

#include <glibmm.h>
#include <windows.h>
#include <wia.h>

class WiaProperty
{
public:
    WiaProperty(STATPROPSTG& wiaPropertyStorage);
    WiaProperty(const WiaProperty& orig) = default;
    virtual ~WiaProperty() = default;

    Glib::ustring info(IWiaPropertyStorage *pWiaPropertyStorage);
protected:
    Glib::ustring convertVarTypeToString(VARTYPE vt);
    Glib::ustring convertValueToString( const PROPVARIANT &propvar);
private:
    Glib::ustring m_name;
    PROPID m_propid;
};
