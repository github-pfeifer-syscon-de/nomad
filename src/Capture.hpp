/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#pragma once

#include <gdk/gdk.h>

class Capture
{
public:
    Capture() = default;
    explicit Capture(const Capture& orig) = delete;
    virtual ~Capture() = default;

    virtual GdkPixbuf *get_pixbuf(GdkRectangle* rectangle) = 0;

    bool get_take_window_shot();
    void set_take_window_shot(bool take_window_shot);
    bool get_take_area_shot();
    void set_take_area_shot(bool take_area_shot);
    bool get_include_pointer();
    void set_include_pointer(bool include_pointer);
private:
protected:
    bool m_take_window_shot{false};
    bool m_take_area_shot{false};
    bool m_include_pointer{false};

};

