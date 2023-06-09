/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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

#include <cmath>
#include <iostream>
#include <istream>

#include "Preview.hpp"
#include "NomadWin.hpp"

RSvg::RSvg()
: m_handle{nullptr}
{
    m_handle = rsvg_handle_new_with_flags(RSVG_HANDLE_FLAGS_NONE);
}

RSvg::~RSvg()
{
    g_object_unref(m_handle);
}

bool
RSvg::from_file(const Glib::RefPtr<Gio::File>& f)
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
        std::clog << "RSvg::from_file " << msg << std::endl;
        //throw std::runtime_error(msg);
    }
    return result;
}

bool
RSvg::pixel_size(gdouble* svgWidth, gdouble* svgHeight)
{
    return rsvg_handle_get_intrinsic_size_in_pixels(m_handle, svgWidth, svgHeight);
}

bool
RSvg::render(const Cairo::RefPtr<Cairo::Context>& cairoCtx, int width, int height)
{
    RsvgRectangle view;
    view.x = 0.0;
    view.y = 0.0;
    //gdouble w,h;
    //pixel_size(&w, &h);
    view.width = width;
    view.height = height;
    return rsvg_handle_render_document(m_handle, cairoCtx->cobj(), &view, nullptr);
}


Preview::Preview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::DrawingArea(cobject)
, m_nomadWin{nomadWin}
, m_pixbuf{}
, m_scaled{}
{
    m_svg = std::make_shared<RSvg>();
    auto f = Gio::File::create_for_path(Glib::filename_from_utf8("/home/rpf/Downloads/arrow-up-svgrepo-com.svg"));
    if (m_svg->from_file(f)) {
        std::clog << "svg/debug: SVG loaded successfully." << std::endl;
        double svgWidth,svgHeight;
        if (m_svg->pixel_size(&svgWidth, &svgHeight)) {
            std::clog << "svg/debug: "
                      << " width " << svgWidth
                      << " height " << svgHeight << std::endl;
        }
        else {
            std::clog << "svg/debug no size! " << std::endl;
        }
    }
    else {
        std::clog << "svg/debug: error loading svg." << std::endl;
    }
}


void
Preview::setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
    m_pixbuf = pixbuf;
    m_scaled.reset();
    queue_draw();
}

bool
Preview::on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx)
{
    if (m_pixbuf) {
        double wScale = static_cast<double>(get_width()) / static_cast<double>(m_pixbuf->get_width());
        double hScale = static_cast<double>(get_height()) / static_cast<double>(m_pixbuf->get_height());
        double scale = std::min(wScale, hScale);
        int scaledWidth = static_cast<int>(static_cast<double>(m_pixbuf->get_width()) * scale);
        int scaledHeight = static_cast<int>(static_cast<double>(m_pixbuf->get_height()) * scale);
        if (!m_scaled
         || (std::abs(scaledWidth - m_scaled->get_width()) > 10
         &&  std::abs(scaledHeight - m_scaled->get_height()) > 10)) {   // scale with steps not every pixel
            //std::cout << "scaling "
            //          << " width " << scaledWidth
            //          << " height " << scaledHeight << std::endl;
            m_scaled = m_pixbuf->scale_simple(scaledWidth, scaledHeight, Gdk::InterpType::INTERP_BILINEAR);
        }
        //int xoffs = std::max(get_width() - static_cast<int>(static_cast<double>(m_pixbuf->get_width()) * scale) / 2, 0);
        //int yoffs = std::max(get_height() - static_cast<int>(static_cast<double>(m_pixbuf->get_height()) * scale) / 2, 0);
        Gdk::Cairo::set_source_pixbuf(cairoCtx, m_scaled, 0, 0);
        cairoCtx->rectangle(0, 0, m_scaled->get_width(), m_scaled->get_height());
        cairoCtx->fill();
    }
    if (m_svg) {
        m_svg->render(cairoCtx, get_width(), get_height());
    }
    return true;
}

