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
#include <sstream>      // std::ostringstream
#include <iomanip>

#include "WiaProperty.hpp"
#include "StringUtils.hpp"

// this macro is broken so use functions
//#define WIA_PROP_LIST_VALUE(ppv,index) \ ((index > ((PROPVARIANT*) ppv)->cal.cElems - WIA_LIST_VALUES) || (index < -WIA_LIST_NOM))
//?\ NULL
//: \ (((PROPVARIANT*) ppv)->vt==VT_UI1) ? \ ((PROPVARIANT*) ppv)->caub.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_UI2) ? \ ((PROPVARIANT*) ppv)->caui.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_UI4) ? \ ((PROPVARIANT*) ppv)->caul.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_I2) ? \ ((PROPVARIANT*) ppv)->cai.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_I4) ? \ ((PROPVARIANT*) ppv)->cal.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_R4) ? \ ((PROPVARIANT*) ppv)->caflt.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_R8) ? \ ((PROPVARIANT*) ppv)->cadbl.pElems[WIA_LIST_VALUES + index]
//: \ (((PROPVARIANT*) ppv)->vt==VT_BSTR) ? \ (LONG)(((PROPVARIANT*) ppv)->cabstr.pElems[WIA_LIST_VALUES + index])
//: \ NULL

static double
prop_list_value_double(PROPVARIANT* ppv, uint32_t index)
{
    double ret = 0.0;
    if (index < WIA_PROP_LIST_COUNT(ppv)) {
        uint32_t vtAttr = ppv->vt & VT_TYPEMASK;
        switch (vtAttr) {
            case VT_R4:
                ret = (double)ppv->caflt.pElems[WIA_LIST_VALUES + index];
                break;
            case VT_R8:
                ret = (double)ppv->cadbl.pElems[WIA_LIST_VALUES + index];
                break;
            default:
                ret = vtAttr;
                break;
        }
    }
    return ret;
}

static int32_t
prop_list_value_int(PROPVARIANT* ppv, uint32_t index)
{
    int32_t ret = 0;
    if (index < WIA_PROP_LIST_COUNT(ppv)) {
        uint32_t vtAttr = ppv->vt & VT_TYPEMASK;
        switch (vtAttr) {
            case VT_I2:
                ret = static_cast<int32_t>(ppv->cai.pElems[WIA_LIST_VALUES + index]);
                break;
            case VT_I4:
                ret = static_cast<int32_t>(ppv->cal.pElems[WIA_LIST_VALUES + index]);
                break;
            default:
                //ret = vtAttr;
                break;
        }
    }
    return ret;
}

static uint32_t
prop_list_value_uint(PROPVARIANT* ppv, uint32_t index)
{
    uint32_t ret = 0u;
    if (index < WIA_PROP_LIST_COUNT(ppv)) {
        uint32_t vtAttr = ppv->vt & VT_TYPEMASK;
        switch (vtAttr) {
            case VT_UI1:
                ret = static_cast<uint32_t>(ppv->caub.pElems[WIA_LIST_VALUES + index]);
                break;
            case VT_UI2:
                ret = static_cast<uint32_t>(ppv->caui.pElems[WIA_LIST_VALUES + index]);
                break;
            case VT_UI4:
                ret = static_cast<uint32_t>(ppv->caul.pElems[WIA_LIST_VALUES + index]);
                break;
            default:
                //ret = (uint32_t)vtAttr;
                break;
        }
    }
    return ret;
}

static Glib::ustring
prop_list_value_str(PROPVARIANT* ppv, uint32_t index)
{
    Glib::ustring ret;
    if (index < WIA_PROP_LIST_COUNT(ppv)) {
        uint32_t vtAttr = ppv->vt & VT_TYPEMASK;
        switch (vtAttr) {
            case VT_BSTR:
                ret = StringUtils::utf8_encode(ppv->cabstr.pElems[WIA_LIST_VALUES + index]);
                break;
            default:
                //ret = Glib::ustring("%d", vtAttr);
                break;
        }
    }
    return ret;
}

void
WiaValue::set(const PROPVARIANT& propvar)
{
    switch (propvar.vt) {
        case VT_I1:
            emplace<int32_t>(static_cast<int32_t>(propvar.cVal));
            break;
        case VT_I2:
            emplace<int32_t>(static_cast<int32_t>(propvar.iVal));
            break;
        case VT_I4:
        case VT_INT:
            emplace<int32_t>(propvar.lVal);
            break;
//        case VT_I8: {     ms specific?
//            int64_t i64 = propvar.hVal;
//            emplace<int64_t>(i64);
//            }
//            break;
        case VT_UI1:
            emplace<uint32_t>(static_cast<uint32_t>(propvar.bVal));
            break;
        case VT_UI2:
            emplace<uint32_t>(static_cast<uint32_t>(propvar.uiVal));
            break;
        case VT_UI4:
        case VT_UINT:
            emplace<uint32_t>(propvar.ulVal);
            break;
//        case VT_UI8: {    ms specific?
//            uint64_t ui64 = propvar.uhVal;
//            emplace<uint64_t>(ui64);
//            }
//            break;
        case VT_R4:
             emplace<double>(static_cast<double>(propvar.fltVal));
             break;
         case VT_R8:
             emplace<double>(propvar.dblVal);
             break;
         case VT_BSTR:
             emplace<Glib::ustring>(StringUtils::utf8_encode(propvar.bstrVal));
             break;
         case VT_BOOL:
             emplace<bool>(VARIANT_TRUE == propvar.boolVal ? true : false);
             break;
         case VT_LPSTR:
             emplace<Glib::ustring>(Glib::ustring(propvar.pszVal));   // ???
             break;
         case VT_LPWSTR:
             emplace<Glib::ustring>(StringUtils::utf8_encode(propvar.pwszVal));
             break;
         default:
             break;
    }
    m_vt = propvar.vt;
    //std::cout << "WiaValue::set vt " << m_vt << " " << WiaProperty::convertVarTypeToString(m_vt)
    //          << " int32_t " << (std::holds_alternative<int32_t>(*this) ? "y" : "n")
    //          << " uint32_t " << (std::holds_alternative<uint32_t>(*this) ? "y" : "n") << std::endl;
}


bool
WiaValue::get(PROPVARIANT& propvar)
{
    bool set{false};
    //std::cout << "WiaValue::get vt " << m_vt << " " << WiaProperty::convertVarTypeToString(m_vt)
    //          << " int32_t " << (std::holds_alternative<int32_t>(*this) ? "y" : "n")
    //          << " uint32_t " << (std::holds_alternative<uint32_t>(*this) ? "y" : "n") << std::endl;
    switch (m_vt) {
            case VT_I1:
            set = std::holds_alternative<int32_t>(*this);
            if (set) {
                propvar.cVal = static_cast<int8_t>(std::get<int32_t>(*this));
            }
            break;
        case VT_I2:
            set = std::holds_alternative<int32_t>(*this);
            if (set) {
                propvar.iVal = static_cast<int16_t>(std::get<int32_t>(*this));
            }
            break;
        case VT_I4:
        case VT_INT:
            set = std::holds_alternative<int32_t>(*this);
            if (set) {
                propvar.lVal = std::get<int32_t>(*this);
            }
            break;
        case VT_UI1:
            set = std::holds_alternative<uint32_t>(*this);
            if (set) {
                propvar.bVal = static_cast<uint8_t>(std::get<uint32_t>(*this));
            }
            break;
        case VT_UI2:
            set = std::holds_alternative<uint32_t>(*this);
            if (set) {
                propvar.uiVal = static_cast<uint16_t>(std::get<uint32_t>(*this));
            }
            break;
        case VT_UI4:
        case VT_UINT:
            set = std::holds_alternative<uint32_t>(*this);
            if (set) {
                propvar.ulVal = std::get<uint32_t>(*this);
            }
            break;
    }
    if (set) {
        propvar.vt = m_vt;
    }
    return set;
    // at the moment other types are unused ...
    return false;
}

VARTYPE
WiaValue::getVt()
{
    return m_vt;
}

void
WiaValue::set(int32_t val)
{
    m_vt = VT_I4;
    emplace<int32_t>(val);
}

WiaProperty::WiaProperty(STATPROPSTG& nextWiaPropertyStorage)
{
    if( NULL != nextWiaPropertyStorage.lpwstrName ) {
        m_name = StringUtils::utf8_encode(nextWiaPropertyStorage.lpwstrName);
    }
    m_propid = nextWiaPropertyStorage.propid;
}

PROPID
WiaProperty::getPropertyId()
{
    return m_propid;
}

// get list/range depending on flags
std::vector<WiaValue>
WiaProperty::getRange(IWiaPropertyStorage *pWiaPropertyStorage)
{
    PROPSPEC propspec;
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = m_propid;

    PROPVARIANT propAttribute;
    PropVariantInit(&propAttribute);
    auto c_nPropertyAttributeCount = 1;
    ULONG flags = 0;
    HRESULT hr = pWiaPropertyStorage->GetPropertyAttributes(c_nPropertyAttributeCount, &propspec, &flags, &propAttribute);
    std::vector<WiaValue> ret;
    if (SUCCEEDED(hr)) {
        //ret += Glib::ustring::sprintf("   attribute %s flags %s ", convertVarTypeToString(propAttribute.vt & VT_TYPEMASK), getFlags(flags));
        // only these cases are useful for us
        if ((flags & WIA_PROP_LIST) || (flags & WIA_PROP_RANGE)) {
            //ret += Glib::ustring::sprintf("[%d]", WIA_PROP_LIST_COUNT(&propAttribute));
            for (uint32_t i = 0; i < WIA_PROP_LIST_COUNT(&propAttribute); ++i) {
                uint32_t vtAttr = propAttribute.vt & VT_TYPEMASK;
                switch (vtAttr) {
                    case VT_I2:
                    case VT_I4:
                        {
                            int32_t ival = prop_list_value_int(&propAttribute, i);
                            WiaValue val;
                            val.emplace<int32_t>(ival);
                            ret.push_back(val);
                        }
                        break;
                    case VT_UI1:
                    case VT_UI2:
                    case VT_UI4:
                        {
                            uint32_t uval = prop_list_value_uint(&propAttribute, i);
                            WiaValue val;
                            val.emplace<uint32_t>(uval);
                            ret.push_back(val);
                        }
                        break;
                    case VT_BSTR:
                        {
                        auto ustr = prop_list_value_str(&propAttribute, i);
                        WiaValue val;
                        val.emplace<Glib::ustring>(ustr);
                        ret.push_back(val);
                        }
                        break;
                    case VT_R4:
                    case VT_R8:
                        {
                        auto dval = prop_list_value_double(&propAttribute, i);
                        WiaValue val;
                        val.emplace<double>(dval);
                        ret.push_back(val);
                        }
                        break;
//                    default:
//                        ret += Glib::ustring::sprintf(" ?%u,", vtAttr);
//                        break;
                }
            }
        }
        //
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray(c_nPropertyAttributeCount, &propAttribute);
    }
    return ret;
}


// this will also return unsigned values (which may fail if the sign bit will be used)
WiaValue
WiaProperty::getValue(IWiaPropertyStorage *pWiaPropertyStorage)
{
    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    PROPSPEC propspec;
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = m_propid;
    WiaValue value;

    auto c_nPropertyCount = 1;
    HRESULT hr = pWiaPropertyStorage->ReadMultiple(c_nPropertyCount, &propspec, &propvar);
    if (SUCCEEDED(hr)) {
        value.set(propvar);
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray(c_nPropertyCount, &propvar);
    }
    else {
        std::cout << "Error " << hr << " getValue ReadMultiple " << std::endl;
    }
    return value;
}

Glib::ustring
WiaProperty::getFlags(ULONG flags)
{
    Glib::ustring ret;
    if (flags & WIA_PROP_CACHEABLE) {
        ret += " cacheable";
    }
    if (flags & WIA_PROP_FLAG) {
        ret += " flag";
    }
    if (flags & WIA_PROP_LIST) {
        ret += " list";
    }
    if (flags & WIA_PROP_NONE) {
        ret += " none";
    }
    if (flags & WIA_PROP_RANGE) {
        ret += " range";
    }
    if (flags & WIA_PROP_READ) {
        ret += " read";
    }
    if (flags & WIA_PROP_RW) {
        ret += " read/write";
    }
    if (flags & WIA_PROP_SYNC_REQUIRED) {
        ret += " syncrequired";
    }
    if (flags & WIA_PROP_WRITE) {
        ret += " write";
    }
    return ret;
}

Glib::ustring
WiaProperty::decodeAttribute(IWiaPropertyStorage *pWiaPropertyStorage)
{
    PROPSPEC propspec;
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = m_propid;

    PROPVARIANT propAttribute;
    PropVariantInit(&propAttribute);
    auto c_nPropertyAttributeCount = 1;
    ULONG flags = 0;
    HRESULT hr = pWiaPropertyStorage->GetPropertyAttributes(c_nPropertyAttributeCount, &propspec, &flags, &propAttribute);
    Glib::ustring ret;
    if (SUCCEEDED(hr)) {
        ret += Glib::ustring::sprintf("   attribute %s flags %s ", convertVarTypeToString(propAttribute.vt & VT_TYPEMASK), getFlags(flags));
        // only these cases are useful for us
        if ((flags & WIA_PROP_LIST) || (flags & WIA_PROP_RANGE)) {
            ret += Glib::ustring::sprintf("[%d]", WIA_PROP_LIST_COUNT(&propAttribute));
            for (uint32_t i = 0; i < WIA_PROP_LIST_COUNT(&propAttribute); ++i) {
                uint32_t vtAttr = propAttribute.vt & VT_TYPEMASK;
                switch (vtAttr) {
                    case VT_I2:
                    case VT_I4:
                        ret += Glib::ustring::sprintf(" %d,", prop_list_value_int(&propAttribute, i));
                        break;
                    case VT_UI1:
                    case VT_UI2:
                    case VT_UI4:
                        ret += Glib::ustring::sprintf(" %u,", prop_list_value_uint(&propAttribute, i));
                        break;
                    case VT_BSTR:
                        ret += Glib::ustring::sprintf(" %s,", prop_list_value_str(&propAttribute, i));
                        break;
                    case VT_R4:
                    case VT_R8:
                        ret += Glib::ustring::sprintf(" %lf,", prop_list_value_double(&propAttribute, i));
                        break;
                    default:
                        ret += Glib::ustring::sprintf(" ?%u,", vtAttr);
                        break;
                }
            }
        }
        //
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray(c_nPropertyAttributeCount, &propAttribute);
    }
    else {
        ret += Glib::ustring("   attribute error %d", hr);
    }
    return ret;
}

Glib::ustring
WiaProperty::info(IWiaPropertyStorage *pWiaPropertyStorage)
{
    PROPVARIANT propvar;
    PropVariantInit(&propvar);
    PROPSPEC propspec;
    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = m_propid;

    Glib::ustring ret = " Name = " + m_name + "\n";
    auto c_nPropertyCount = 1;
    HRESULT hr = pWiaPropertyStorage->ReadMultiple(c_nPropertyCount, &propspec, &propvar);
    if (SUCCEEDED(hr)) {
        // Display the property value, type, and so on.
        ret += Glib::ustring::sprintf("   PropID = %d VarType = %s\n"
            ,  m_propid, convertVarTypeToString( propvar.vt));
        ret += Glib::ustring::sprintf("   Value = %s\n", convertValueToString(propvar));
        ret += decodeAttribute(pWiaPropertyStorage);
        //
        // Free the returned PROPVARIANTs
        //
        FreePropVariantArray(c_nPropertyCount, &propvar);
    }
    else {
        ret += Glib::ustring::sprintf("Error %d ReadMultiple ", hr);
    }
    return ret;
}

// see https://learn.microsoft.com/en-us/windows/win32/stg/enumall-sample
Glib::ustring
WiaProperty::convertVarTypeToString(VARTYPE vt)
{

    // Create a string using the basic type.
    Glib::ustring ret;
    switch (vt & VT_TYPEMASK)
    {
    case VT_EMPTY:
        ret = "VT_EMPTY";
        break;
    case VT_NULL:
        ret  ="VT_NULL";
        break;
    case VT_I2:
        ret = "VT_I2";
        break;
    case VT_I4:
        ret = "VT_I4";
        break;
    case VT_I8:
        ret = "VT_I8";
        break;
    case VT_UI2:
        ret = "VT_UI2";
        break;
    case VT_UI4:
        ret = "VT_UI4";
        break;
    case VT_UI8:
        ret = "VT_UI8";
        break;
    case VT_R4:
        ret = "VT_R4";
        break;
    case VT_R8:
        ret = "VT_R8";
        break;
    case VT_CY:
        ret = "VT_CY";
        break;
    case VT_DATE:
        ret = "VT_DATE";
        break;
    case VT_BSTR:
        ret = "VT_BSTR";
        break;
    case VT_ERROR:
        ret = "VT_ERROR";
        break;
    case VT_BOOL:
        ret = "VT_BOOL";
        break;
    case VT_VARIANT:
        ret = "VT_VARIANT";
        break;
    case VT_DECIMAL:
        ret = "VT_DECIMAL";
        break;
    case VT_I1:
        ret = "VT_I1";
        break;
    case VT_UI1:
        ret = "VT_UI1";
        break;
    case VT_INT:
        ret = "VT_INT";
        break;
    case VT_UINT:
        ret = "VT_UINT";
        break;
    case VT_VOID:
        ret = "VT_VOID";
        break;
    case VT_SAFEARRAY:
        ret = "VT_SAFEARRAY";
        break;
    case VT_USERDEFINED:
        ret = "VT_USERDEFINED";
        break;
    case VT_LPSTR:
        ret = "VT_LPSTR";
        break;
    case VT_LPWSTR:
        ret = "VT_LPWSTR";
        break;
    case VT_RECORD:
        ret = "VT_RECORD";
        break;
    case VT_FILETIME:
        ret = "VT_FILETIME";
        break;
    case VT_BLOB:
        ret = "VT_BLOB";
        break;
    case VT_STREAM:
        ret = "VT_STREAM";
        break;
    case VT_STORAGE:
        ret = "VT_STORAGE";
        break;
    case VT_STREAMED_OBJECT:
        ret = "VT_STREAMED_OBJECT";
        break;
    case VT_STORED_OBJECT:
        ret = "VT_BLOB_OBJECT";
        break;
    case VT_CF:
        ret = "VT_CF";
        break;
    case VT_CLSID:
        ret = "VT_CLSID";
        break;
    default:
        ret = Glib::ustring::sprintf("Unknown (%d)", vt & VT_TYPEMASK );
        break;
    }

    // Add the type modifiers, if present.
    if (vt & VT_VECTOR) {
        ret += " | VT_VECTOR";
    }
    if (vt & VT_ARRAY) {
        ret += " | VT_ARRAY";
    }
    if (vt & VT_RESERVED) {
        ret += " | VT_RESERVED";
    }
    return ret;
}

Glib::ustring
WiaProperty::convertValueToString( const PROPVARIANT &propvar)
{

    // Based on the type, put the value into ret as a string.
    Glib::ustring ret;
    switch (propvar.vt)    // VT_TYPEMASK is left out to get not confused by complex types
    {
    case VT_EMPTY:
        ret = "";
        break;
    case VT_NULL:
        ret = "";
        break;
    case VT_I2:
        ret = Glib::ustring::sprintf( "%04d", (int32_t)propvar.iVal );
        break;
    case VT_I4:
    case VT_INT:
        ret = Glib::ustring::sprintf( "%d", propvar.lVal );
        break;
//    case VT_I8:       // ms specific? dont crash sprintf!
//        ret = Glib::ustring::sprintf( "%ld", propvar.hVal );
//        break;
    case VT_UI2:
        ret = Glib::ustring::sprintf( "%04u", (uint32_t)propvar.uiVal );
        break;
    case VT_UI4:
    case VT_UINT:
        ret = Glib::ustring::sprintf( "%u", propvar.ulVal );
        break;
//    case VT_UI8:      // ms specific? dont crash sprintf!
//        ret = Glib::ustring::sprintf( "%lu", propvar.uhVal );
//        break;
    case VT_R4:
        ret = Glib::ustring::sprintf( "%f", propvar.fltVal );
        break;
    case VT_R8:
        ret = Glib::ustring::sprintf( "%lf", propvar.dblVal );
        break;
    case VT_BSTR:
        ret = Glib::ustring::sprintf( "\"%s\"",
                     StringUtils::utf8_encode(propvar.bstrVal) );
        break;
    case VT_ERROR:
        ret = Glib::ustring::sprintf( "0x%08X", propvar.scode );
        break;
    case VT_BOOL:
        ret = VARIANT_TRUE == propvar.boolVal ? "True" : "False";
        break;
    case VT_I1:
        ret = Glib::ustring::sprintf( "%02d", (int32_t)propvar.cVal );
        break;
    case VT_UI1:
        ret = Glib::ustring::sprintf( "%02u", (uint32_t)propvar.bVal );
        break;
    case VT_VOID:
        ret = "";
        break;
    case VT_LPSTR:
        ret = Glib::ustring::sprintf( "\"%s\"", propvar.pszVal );
        break;
    case VT_LPWSTR:
        ret = Glib::ustring::sprintf( "\"%s\"", StringUtils::utf8_encode(propvar.pwszVal) );
        break;
    case VT_FILETIME:
        ret = Glib::ustring::sprintf( "%08x:%08x",
                     propvar.filetime.dwHighDateTime,
                     propvar.filetime.dwLowDateTime );
        break;
    case VT_CLSID:
        WCHAR pwszValue[64];
        pwszValue[0] = L'\0';
        StringFromGUID2( *propvar.puuid, pwszValue, sizeof(pwszValue)/sizeof(pwszValue[0]) );
        ret = StringUtils::utf8_encode(pwszValue);
        break;
    default:
        ret = "?";
        break;
    }
    return ret;
}

std::string
WiaProperty::dump(const guint8 *data, gsize size)
{
    std::ostringstream out;
    gsize offset = 0u;
    while (offset < size) {
        if (offset > 0u) {
            out << std::endl;
        }
        out << std::hex << std::setw(4) << std::setfill('0') << offset << ":";
        for (gsize i = 0; i < std::min(size-offset, (gsize)16u); ++i)  {
            out << std::setw(2) << std::setfill('0') << (int)data[offset+i] << " ";
        }
        out << std::dec << std::setw(1) << " ";
        for (gsize i = 0; i < std::min(size-offset, (gsize)16u); ++i)  {
            if (data[offset+i] >= 32 && data[offset+i] < 127) {
                out << data[offset+i];
            }
            else {
                out << ".";
            }
        }
        offset += 16u;
    }
    return out.str();
}

