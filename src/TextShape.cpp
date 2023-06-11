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
    Cairo::TextExtents extends;
    cairoCtx->get_text_extents(m_text.getText(), extends);
    //std::cout << "Text"
    //          << " width " <<  extends.width
    //          << " height " <<  extends.height
    //          << std::endl;
    cairoCtx->set_source_rgb(
            m_text.getColor().get_red_p(),
            m_text.getColor().get_green_p(),
            m_text.getColor().get_blue_p());
    auto font =
        Cairo::ToyFontFace::create(m_text.getFont(),
                               m_text.getSlant(),
                               m_text.getWeight());
    cairoCtx->set_font_face(font);
    cairoCtx->set_font_size(getScale() * width / 400.0 * m_text.getSize());
    cairoCtx->show_text(m_text.getText());
    //cairoCtx->stroke();
    return true;
}
