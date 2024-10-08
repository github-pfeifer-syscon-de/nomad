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

#include <cairomm/cairomm.h>
#include <memory>

#include "Shape.hpp"

class Preview;

class TextInfo
{
public:
    TextInfo() = default;
    TextInfo(const TextInfo& other) = default;
    virtual ~TextInfo() = default;

    Glib::ustring getText() {
        return m_text;
    }
    void setText(const Glib::ustring& text) {
        m_text = text;
    }
    Gdk::Color getColor() {
        return m_color;
    }
    void setColor(const Gdk::Color& color) {
        m_color = color;
    }
    Glib::ustring getFont() {
        return m_font;
    }
    void setFont(Glib::ustring font) {
        m_font = font;
    }
private:
    Glib::ustring m_text;
    Gdk::Color m_color;
    Glib::ustring m_font;
};

class TextShape
: public Shape
{
public:
    TextShape();
    explicit TextShape(const TextShape& orig) = delete;
    virtual ~TextShape() = default;

    void setText(const TextInfo& text);
    bool render(
            const Cairo::RefPtr<Cairo::Context>& cairoCtx,
            int width,
            int height) override;
    Gdk::Rectangle getBounds(
            int width,
            int height) override;
    void edit(Preview& preview) override;
    static bool ask_text(TextInfo& text, Preview& preview);

private:
    TextInfo m_text;
    double m_ext_height{0.0};
    double m_ext_width{0.0};
};

