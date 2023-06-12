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
#include <cairomm/cairomm.h>

#include "TextShape.hpp"
#include "Shape.hpp"

TextShape::TextShape()
: Shape()
{
}

void
TextShape::setText(const TextInfo& text)
{
    m_text = text;
}

bool
TextShape::render(
        const Cairo::RefPtr<Cairo::Context>& cairoCtx,
        int width,
        int height)
{
    int x = toRealX(width);
    int y = toRealY(height);
    cairoCtx->move_to(x, y);
    //std::cout << "Text"
    //          << " width " <<  extends.width
    //          << " height " <<  extends.height
    //          << std::endl;
    cairoCtx->set_source_rgb(
            m_text.getColor().get_red_p(),
            m_text.getColor().get_green_p(),
            m_text.getColor().get_blue_p());
    auto layout = Pango::Layout::create(cairoCtx);
    auto descr = Pango::FontDescription(m_text.getFont());
    // make size depend on output
    descr.set_size(static_cast<int>(getScale() * static_cast<double>(width) / 400.0 * static_cast<double>(descr.get_size())));
    layout->set_font_description(descr);
    layout->set_text(m_text.getText());
    //auto font =
    //    Cairo::ToyFontFace::create(m_text.getFont(),
    //                           m_text.getSlant(),
    //                           m_text.getWeight());
    //cairoCtx->set_font_face(font);
    //cairoCtx->show_text(m_text.getText());
    layout->show_in_cairo_context(cairoCtx);
    auto extend = layout->get_ink_extents();
    m_extends.height = extend.get_width();
    m_extends.width = extend.get_height();
    return true;
}

Gdk::Rectangle
TextShape::getBounds(
        int width,
        int height)
{
    Gdk::Rectangle next;
    // this assumes text was rendered with actual values ...
    next.set_x(toRealX(width));
    next.set_y(toRealY(height));
    next.set_width(static_cast<int>(m_extends.width));
    next.set_height(static_cast<int>(m_extends.height));
    return next;
}