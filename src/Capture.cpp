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

#include "Capture.hpp"

bool
Capture::get_take_window_shot()
{
    return m_take_window_shot;
}

void
Capture::set_take_window_shot(bool take_window_shot)
{
    m_take_window_shot = take_window_shot;
}

bool
Capture::get_take_area_shot()
{
    return m_take_area_shot;
}

void
Capture::set_take_area_shot(bool take_area_shot)
{
    m_take_area_shot = take_area_shot;
}

bool
Capture::get_include_pointer()
{
    return m_include_pointer;
}

void
Capture::set_include_pointer(bool include_pointer)
{
    m_include_pointer = include_pointer;
}