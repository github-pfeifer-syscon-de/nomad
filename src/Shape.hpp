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

#pragma once

#include <gtkmm.h>

class Shape {
public:
    Shape() = default;
    explicit Shape(const Shape& orig) = delete;
    virtual ~Shape() = default;

    virtual bool render(
            const Cairo::RefPtr<Cairo::Context>& cairoCtx,
            int width,
            int height) = 0;
    virtual Gdk::Rectangle getBounds(
            int width,
            int height) = 0;
    void setRelPosition(double posX, double posY);     // relative position 0..1
    int toRealX(int width);
    int toRealY(int height);
private:
    double m_relPosX{0.5};
    double m_relPosY{0.5};
};

