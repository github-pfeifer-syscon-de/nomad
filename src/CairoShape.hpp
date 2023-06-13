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

#include <vector>
#include <glibmm.h>

#include "ScaleableShape.hpp"



class DrawCommand {
public:
    DrawCommand(gunichar cmd)
    : m_cmd{cmd}
    {
    }
    void setX(double x) {
        m_x = x;
        m_xSet = true;
    }
    double getX() {
        return m_x;
    }
    bool isXSet() {
        return m_xSet;
    }
    void setY(double y) {
        m_y = y;
    }
    double getY() {
        return m_y;
    }
    gunichar getCmd() {
        return m_cmd;
    }
    explicit DrawCommand(const DrawCommand& orig) = delete;
    virtual ~DrawCommand() = default;
private:
    gunichar m_cmd;
    bool m_xSet{false};
    double m_x{0.0};
    double m_y{0.0};
};

/*
 * accepts commands like:
 *   S = size x y
 *   W = line width w
 *   M = move x y
 *   L = line to x y
 */
class CairoShape
: public ScaleableShape
{
public:
    CairoShape(const Glib::ustring& path);
    explicit CairoShape(const CairoShape& orig) = delete;
    virtual ~CairoShape() = default;

    bool render(
            const Cairo::RefPtr<Cairo::Context>& cairoCtx,
            int outpWidth,
            int outpHeight) override;
    Gdk::Rectangle getBounds(
            int outpWidth,
            int outpHeight) override;
    void setColor(const Gdk::Color& color) {
        m_color = color;
    }
    Gdk::Color getColor() {
        return m_color;
    }
protected:

private:
    double relWidth(int outpWidth);
    double relHeight(int outpHeight);
    std::vector<std::shared_ptr<DrawCommand>> m_path;
    Gdk::Color m_color;
};

