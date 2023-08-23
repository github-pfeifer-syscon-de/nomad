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


#include "Shape.hpp"

void
Shape::setRelPosition(double posX, double posY)
{
    m_relPosX = posX;
    m_relPosY = posY;
}

int
Shape::toRealX(int outpWidth)
{
    return static_cast<int>(m_relPosX * static_cast<double>(outpWidth));
}

int
Shape::toRealY(int outpHeight)
{
    return static_cast<int>(m_relPosY * static_cast<double>(outpHeight));
}
