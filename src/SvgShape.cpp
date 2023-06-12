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

#include "SvgShape.hpp"

SvgShape::SvgShape()
: m_handle{nullptr}
{
    m_handle = rsvg_handle_new_with_flags(RSVG_HANDLE_FLAGS_NONE);
}

SvgShape::~SvgShape()
{
    g_object_unref(m_handle);
}

bool
SvgShape::from_file(const Glib::RefPtr<Gio::File>& f)
{
    rsvg_handle_set_base_uri(m_handle, f->get_path().c_str());
    auto fs = f->read();
    GError* pError = nullptr;
    // rsvg_new_from_gfile might be an option but for the moment keep this generic
    gboolean result = rsvg_handle_read_stream_sync(m_handle, G_INPUT_STREAM(fs.get()->gobj()), nullptr, &pError);
    fs->close();
    if (!result) {
        std::string msg("Unable to load " + f->get_path());
        if (pError) {
            msg = msg + " error " + pError->message;
            g_error_free(pError);
        }
        std::clog << "SvgShape::from_file " << msg << std::endl;
        //throw std::runtime_error(msg);
    }
    return result;
}

bool
SvgShape::pixel_size(gdouble* svgWidth, gdouble* svgHeight)
{
    return rsvg_handle_get_intrinsic_size_in_pixels(m_handle, svgWidth, svgHeight);
}

bool
SvgShape::render(const Cairo::RefPtr<Cairo::Context>& cairoCtx, int width, int height)
{
    RsvgRectangle view;
    view.x = toRealX(width);
    view.y = toRealY(height);
    view.width = toRealWidth(width);
    view.height = toRealHeight(height);
    return rsvg_handle_render_document(m_handle, cairoCtx->cobj(), &view, nullptr);
}

Gdk::Rectangle
SvgShape::getBounds(
        int width,
        int height)
{
    Gdk::Rectangle next;
    next.set_x(toRealX(width));
    next.set_y(toRealY(height));
    next.set_width(toRealWidth(width));
    next.set_height(toRealHeight(height));
    return next;
}