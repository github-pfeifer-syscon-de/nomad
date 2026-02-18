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

#pragma once

#include <glibmm.h>
#include <typeinfo> 
#include <windows.h>
#include <variant>
#include <limits>

// use local wia.h as workaround, if the msys2 version has been filled in, use <wia.h>
#include "wia.h"



class WiaValue2
{
public:
    WiaValue2();
    WiaValue2(uint32_t vtAttr, int32_t i);
    WiaValue2(uint32_t vtAttr, uint32_t u);
    WiaValue2(uint32_t vtAttr, BSTR str);
    WiaValue2(uint32_t vtAttr, double val);

    WiaValue2(const WiaValue2& other) = default;
    WiaValue2& operator= (const WiaValue2& other) = default;
    ~WiaValue2() = default;
    
    BSTR toBstr(const char* str);
    void set(const PROPVARIANT& propvar);
    const PROPVARIANT& get();
    VARTYPE getVt();
    template <typename T>
    T get() 
    {
        if (std::is_same<T, int32_t>::value) {
            switch (m_propvar.vt) {
            case VT_I1:
                return static_cast<int32_t>(m_propvar.iVal);       // how to place ?
                break;
            case VT_I2:
                return static_cast<int32_t>(m_propvar.iVal);
                break;
            case VT_I4:
                return static_cast<int32_t>(m_propvar.intVal);
                break;
            case VT_UI1:     // here we are a sloppy, but we don't expect values that large, has to change if values are > int_max
                return static_cast<int32_t>(m_propvar.uiVal);       // how to place ?
                break;
            case VT_UI2:
                return static_cast<int32_t>(m_propvar.uiVal);
                break;
            case VT_UI4:
                if (m_propvar.uintVal > static_cast<uint32_t>(std::numeric_limits<int32_t>::max())) {
                    std::cout <<  "WiaValue2::get values exceeds int limit! " << " vt " << m_propvar.vt << std::endl;
                    return static_cast<uint32_t>(std::numeric_limits<int32_t>::max());  // maybe this limits the damage?
                }
                return static_cast<int32_t>(m_propvar.uintVal);
                break;
            default:
                std::cout <<  "WiaValue2::get mismatch type " << typeid(T).name() << " vt " << m_propvar.vt << std::endl;
                break;
            }
        }
        else {
            std::cout <<  "WiaValue2::get looking for unexpected type " << typeid(T).name() << std::endl;
        }
        return 0;
    }
    
private:
    PROPVARIANT m_propvar{};
};

class WiaAttribute
{
public:
    WiaAttribute(IWiaPropertyStorage *pWiaPropertyStorage, uint32_t propid);
    ~WiaAttribute();
    
    Glib::ustring info();
    bool isList();
    bool isRange();
protected:
    static Glib::ustring getFlags(ULONG flags);

private:
    HRESULT m_hr;
    PROPVARIANT m_propAttribute;
    static constexpr auto c_nPropertyAttributeCount{1};
    ULONG m_flags;    
    std::vector<int32_t> m_values;
};

class WiaProperty
{
public:
    WiaProperty(STATPROPSTG& wiaPropertyStorage);
    WiaProperty(const WiaProperty& orig) = default;
    virtual ~WiaProperty() = default;

    Glib::ustring info(IWiaPropertyStorage *pWiaPropertyStorage);
    PROPID getPropertyId();
    WiaValue2 getValue(IWiaPropertyStorage *pWiaPropertyStorage);
    std::vector<WiaValue2> getRange(IWiaPropertyStorage *pWiaPropertyStorage);

    static Glib::ustring convertVarTypeToString(VARTYPE vt);
    static std::string dump(const guint8 *data, gsize size);
protected:
    Glib::ustring convertValueToString( const PROPVARIANT &propvar);

private:
    Glib::ustring m_name;
    PROPID m_propid;
    WiaValue2 m_value;
};
