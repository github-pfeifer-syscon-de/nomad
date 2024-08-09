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
#include <locale>

#include "CairoShape.hpp"
#include "LocaleContext.hpp"
#include "StringUtils.hpp"

CairoShape::CairoShape(const Glib::ustring& path)
: m_path{}
, m_color("#fff")
{
    Glib::ustring npath = StringUtils::replaceAll(path, "\n", " ");
    npath = StringUtils::replaceAll(npath, "\t", " ");
    std::vector<Glib::ustring> parts;
    StringUtils::split(npath, ' ', parts);
    std::shared_ptr<DrawCommand> command;
    LocaleContext ctx(LC_NUMERIC);
    for (auto part : parts) {
        auto c = part[0];
        if (g_unichar_isalpha(c)) {
            if (command) {
                m_path.push_back(std::move(command));
            }
            command = std::make_shared<DrawCommand>(c);
        }
        else if (g_unichar_isdigit(c)) {
            double val = ctx.parseDouble(LocaleContext::en_US, part);
            if (command) {
                if (command->isXSet()) {
                    command->setY(val);
                }
                else {
                    command->setX(val);
                }
            }
        }
    }
    if (command) {
        m_path.push_back(std::move(command));
    }
}

double
CairoShape::relWidth(int width)
{
    double rwidth = getScale() * static_cast<double>(width);
    return rwidth;
}

double
CairoShape::relHeight(int height)
{
    double rheight = getScale() * static_cast<double>(height);
    return rheight;
}

bool
CairoShape::render(
        const Cairo::RefPtr<Cairo::Context>& cairoCtx,
        int outpWidth,
        int outpHeight)
{
    cairoCtx->save();
    cairoCtx->set_source_rgb(m_color.get_red_p(), m_color.get_green_p(), m_color.get_blue_p());
    double rwidth{1.0};
    double rheight{1.0};
    int xorg = toRealX(outpWidth);
    int yorg = toRealY(outpHeight);
    cairoCtx->translate(xorg, yorg);
    if (m_rotate != 0) {
        cairoCtx->rotate_degrees(static_cast<double>(m_rotate));
        //Cairo::Matrix matrix;
        //matrix.rotate(static_cast<double>(m_rotate) * M_PI / 180.0);
        //cairoCtx->set_matrix(matrix);
    }
    for (auto cmd : m_path) {
        //std::cout << "Cmd " << cmd->getCmd() << " x " << cmd->getX() << " y " << cmd->getY() << std::endl;
        if (cmd->getCmd() == L'S') {
            if (cmd->getX() != 0.0) {
                rwidth = relWidth(outpWidth) / cmd->getX();
            }
            else {
                std::cout << "CairoShape::render no width!" << std::endl;
            }
            if (cmd->getY() != 0.0) {
                rheight = relHeight(outpHeight) / cmd->getY();
            }
            else {
                std::cout << "CairoShape::render no height!" << std::endl;
            }
            //std::cout << "Size " << " x " << rwidth << " y " << rheight << " scale " << getScale()  << std::endl;
        }
        else if (cmd->getCmd() == L'W') {
            double w = cmd->getX() * rwidth;
            //std::cout << "Width " << " w " << w << std::endl;
            cairoCtx->set_line_width(w);
            cairoCtx->set_line_cap(Cairo::LineCap::LINE_CAP_ROUND);
            cairoCtx->set_line_join(Cairo::LineJoin::LINE_JOIN_ROUND);
        }
        else  if (cmd->getCmd() == L'M') {
            double x = /*xorg + */cmd->getX() * rwidth;
            double y = /*yorg + */cmd->getY() * rheight;
            //std::cout << "Move " << " x " << x << " y " << y << std::endl;
            cairoCtx->move_to(x, y);
        }
        else  if (cmd->getCmd() == L'L') {
            double x = /*xorg + */cmd->getX() * rwidth;
            double y = /*yorg + */cmd->getY() * rheight;
            //std::cout << "Line " << " x " << x << " y " << y << std::endl;
            cairoCtx->line_to(x, y);
        }
    }
    cairoCtx->stroke();
    cairoCtx->restore();
    return true;
}

Gdk::Rectangle
CairoShape::getBounds(
        int outpWidth,
        int outpHeight)
{
    Gdk::Rectangle r;
    r.set_x(toRealX(outpWidth));
    r.set_y(toRealY(outpHeight));
    r.set_width(static_cast<int>(relWidth(outpWidth)));
    r.set_height(static_cast<int>(relHeight(outpHeight)));
    return r;
}
