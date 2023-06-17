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


#include "WiaProperty.hpp"
#include "StringUtils.hpp"

WiaProperty::WiaProperty(STATPROPSTG& nextWiaPropertyStorage)
{
    if( NULL != nextWiaPropertyStorage.lpwstrName ) {
        m_name = StringUtils::utf8_encode(nextWiaPropertyStorage.lpwstrName);
    }
    m_propid = nextWiaPropertyStorage.propid;
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
            ,  m_propid,  convertVarTypeToString( propvar.vt));
        ret += Glib::ustring::sprintf("   Value = %s\n", convertValueToString(propvar));
        ULONG flags;
        PROPVARIANT propAttribute;
        PropVariantInit(&propAttribute);
        auto c_nPropertyAttributeCount = 1;
        hr = pWiaPropertyStorage->GetPropertyAttributes(c_nPropertyAttributeCount, &propspec, &flags, &propAttribute);
        if (SUCCEEDED(hr)) {
            ret += Glib::ustring::sprintf("   attribute flags %s ", getFlags(flags));
            // only these cases are useful for us
            if (flags & WIA_PROP_LIST) {
                //if (propvarattr.vt == 4099) {
                //ret += "list " + convertValueToString(propvarattr);
                //}
                //else {
                //    std::cout << "unused variant type list " << propvarattr.vt << std::endl;
                //}
            }
            if (flags & WIA_PROP_RANGE) {
                //if (propvarattr.vt == 4099) {
                //ret += "range " + convertValueToString(propvarattr);
                //}
                //else {
                //    std::cout << "unused variant type range " << propvarattr.vt << std::endl;
                //}
            }
            //
            // Free the returned PROPVARIANTs
            //
            FreePropVariantArray(c_nPropertyAttributeCount, &propAttribute);
        }
        else {
            ret += Glib::ustring("   attribute error %d", hr);
        }
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
    case VT_I8:
        ret = Glib::ustring::sprintf( "%ld", propvar.hVal );
        break;
    case VT_UI2:
        ret = Glib::ustring::sprintf( "%04u", (uint32_t)propvar.uiVal );
        break;
    case VT_UI4:
    case VT_UINT:
        ret = Glib::ustring::sprintf( "%u", propvar.ulVal );
        break;
    case VT_UI8:
        ret = Glib::ustring::sprintf( "%lu", propvar.uhVal );
        break;
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
        if (propvar.vt & VT_VECTOR) {
            switch (propvar.vt & VT_TYPEMASK) {
                case VT_I4:
                    LPSAFEARRAY *pparray;
                    PROPVARIANT *pvarVal;
                    if (propvar.parray) {
                        ret = Glib::ustring::sprintf("Vector elements %d", propvar.parray->cbElements);
                    }
                    else {
                        ret = Glib::ustring::sprintf("Vector ?");
                    }
                    break;
                default:
                    ret = Glib::ustring::sprintf("Vector unhandled vt 0x%0x", propvar.vt );
                    break;
            }
        }
        else if (propvar.vt & VT_ARRAY) {
            switch (propvar.vt & VT_TYPEMASK) {
                case VT_I4:
                    // propvar.parray->pvData
                    if (propvar.parray) {
                        ret = Glib::ustring::sprintf("Array elements %d", propvar.parray->cbElements);
                    }
                    else {
                        ret = Glib::ustring::sprintf("Array ?");
                    }
                    break;
                default:
                    ret = Glib::ustring::sprintf("Array unhandled vt 0x%0x", propvar.vt );
                    break;
            }
        }
        else {
            ret = Glib::ustring::sprintf("Composite unhandled vt 0x%0x", propvar.vt );
        }
        break;
    }
    return ret;
}
