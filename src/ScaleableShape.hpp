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

#include <gtkmm.h>

#include <Shape.hpp>

// shape that defines it's size by a scale (relative to output size)
class ScaleableShape
: public Shape
{
public:
    ScaleableShape() = default;
    explicit ScaleableShape(const ScaleableShape& orig) = delete;
    virtual ~ScaleableShape() = default;
    void setScale(double scale);
    double getScale();
    void setRotate(int rotate);
    int getRotate();
    int toRealWidth(int width);
    int toRealHeight(int height);
    void edit(Preview& preview) override;
    static bool askScale(Preview& preview, ScaleableShape* cairoShape);
protected:
    double m_scale{1.0};
    int m_rotate{0};

};

