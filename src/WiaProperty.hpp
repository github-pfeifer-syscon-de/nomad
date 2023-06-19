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
#include <variant>

// incomplete implementation of variant value holder
class WiaValue
: public std::variant<int32_t, int64_t, uint32_t, uint64_t, double, Glib::ustring, bool>
{
public:
    WiaValue() = default;
    WiaValue(const WiaValue& other) = default;
    WiaValue& operator= (const WiaValue& other) = default;
    ~WiaValue() = default;

    void set(const PROPVARIANT& propvar);
    bool get(PROPVARIANT& propvar);
    VARTYPE getVt();
    void set(int32_t val);
private:
    VARTYPE m_vt{0};
};

class WiaProperty
{
public:
    WiaProperty(STATPROPSTG& wiaPropertyStorage);
    WiaProperty(const WiaProperty& orig) = default;
    virtual ~WiaProperty() = default;

    Glib::ustring info(IWiaPropertyStorage *pWiaPropertyStorage);
    PROPID getPropertyId();
    WiaValue getValue(IWiaPropertyStorage *pWiaPropertyStorage);
    std::vector<WiaValue> getRange(IWiaPropertyStorage *pWiaPropertyStorage);

    static Glib::ustring convertVarTypeToString(VARTYPE vt);
    static std::string dump(const guint8 *data, gsize size);
protected:
    Glib::ustring convertValueToString( const PROPVARIANT &propvar);
    Glib::ustring getFlags(ULONG flags);
    Glib::ustring decodeAttribute(IWiaPropertyStorage *pWiaPropertyStorage);

private:
    Glib::ustring m_name;
    PROPID m_propid;
};
