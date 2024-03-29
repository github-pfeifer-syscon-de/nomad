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

#include <librsvg/rsvg.h>

#include "ScaleableShape.hpp"

class SvgException
: public Glib::Error
{
public:
    SvgException(GError* err)
    : Glib::Error(err, true)
    {
    }
    virtual ~SvgException() = default;
};

class SvgShape
: public ScaleableShape
{
public:
    SvgShape();
    explicit SvgShape(const SvgShape& orig) = delete;
    virtual ~SvgShape();

    void from_file(const Glib::RefPtr<Gio::File>& f);
    bool pixel_size(gdouble* svgWidth, gdouble* svgHeight);
    bool render(
            const Cairo::RefPtr<Cairo::Context>& cairoCtx,
            int width,
            int height) override;
    Gdk::Rectangle getBounds(
            int width,
            int height) override;

private:
    RsvgHandle* m_handle;
};

