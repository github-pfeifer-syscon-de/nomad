/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
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

#include "StringUtils.hpp"

void
StringUtils::split(const Glib::ustring &line, char delim, std::vector<Glib::ustring> &ret)
{
    size_t pos = 0;
    while (pos < line.length()) {
        size_t next = line.find(delim, pos);
        if (next != std::string::npos) {
            std::string fld = line.substr(pos, next - pos);
            ret.push_back(fld);
            ++next;
        }
        else {
            if (pos < line.length()) {
                size_t end = line.length();
                if (line.at(end-1) == '\n') {
                    --end;
                }
                if (end - pos > 0) {
                    std::string fld = line.substr(pos, end - pos);
                    ret.push_back(fld);
                }
            }
            break;
        }
        pos = next;
    }
}

Glib::ustring
StringUtils::replaceAll(const Glib::ustring& text, const Glib::ustring& replace, const Glib::ustring& with)
{
    Glib::ustring ret = text;
    size_t pos = 0;
    while ((pos = ret.find(replace, pos)) != Glib::ustring::npos) {
         ret.replace(pos, replace.length(), with);
         pos += with.length();
    }
    return ret;
}

#ifdef __WIN32__
#include <windows.h>
#include <wchar.h>
// Convert a wide Unicode string to an UTF8 string
std::string
StringUtils::utf8_encode(const std::wstring &wstr)
{
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
#endif