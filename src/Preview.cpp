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
#include "SvgShape.hpp"
#include "NomadWin.hpp"

Preview::Preview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::DrawingArea(cobject)
, m_nomadWin{nomadWin}
, m_pixbuf{}
, m_scaled{}
{
    add_events(Gdk::EventMask::BUTTON_PRESS_MASK
            | Gdk::EventMask::BUTTON_RELEASE_MASK
            | Gdk::EventMask::BUTTON_MOTION_MASK);

}

bool
Preview::load(const Glib::RefPtr<Gio::File>& f)
{
    auto svg = std::make_shared<SvgShape>();
    if (svg->from_file(f)) {
        std::clog << "svg/debug: SVG loaded successfully." << std::endl;
        svg->setScale(0.1);
        m_shapes.push_back(svg);
        return true;
    }
    else {
        std::clog << "svg/debug: error loading " << f->get_path() << " svg." << std::endl;
    }
    return false;
}

bool
Preview::on_motion_notify_event(GdkEventMotion* motion_event)
{
    bool btn1 = (motion_event->state  & Gdk::ModifierType::BUTTON1_MASK) != 0x0;
    if (btn1) {
        if (m_selected && m_scaled) {
            Gdk::Rectangle old;
            old.set_x(m_selected->toRealX(m_scaled->get_width()));
            old.set_y(m_selected->toRealX(m_scaled->get_height()));
            old.set_width(m_selected->toRealWidth(m_scaled->get_width()));
            old.set_height(m_selected->toRealHeight(m_scaled->get_height()));
            double relX = (motion_event->x - m_relX) / static_cast<double>(m_scaled->get_width());
            double relY = (motion_event->y - m_relY) / static_cast<double>(m_scaled->get_height());
            m_selected->setRelPosition(relX, relY);
            Gdk::Rectangle next;
            next.set_x(m_selected->toRealX(m_scaled->get_width()));
            next.set_y(m_selected->toRealY(m_scaled->get_height()));
            next.set_width(m_selected->toRealWidth(m_scaled->get_width()));
            next.set_height(m_selected->toRealHeight(m_scaled->get_height()));
            next.join(old);
            queue_draw_area(
                    std::max(next.get_x()-20, 0),
                    std::max(next.get_y()-20, 0),
                    next.get_width() + 40,
                    next.get_height() + 40);   // draw only required
            return TRUE;
        }
    }
    return FALSE;
}

bool
Preview::on_button_release_event(GdkEventButton* event)
{
//    bool btn1 = (event->button == 1);
//    if (btn1 && m_selected) {
//        return TRUE;
//    }
    return FALSE;
}

bool
Preview::on_button_press_event(GdkEventButton* event)
{
    bool btn1 = (event->button == 1);
    if (btn1 && m_scaled) {
        m_selected.reset();
        double mouseX = event->x;
        double mouseY = event->y;
        for (auto shape : m_shapes) {
            int x = shape->toRealX(m_scaled->get_width());
            int y = shape->toRealY(m_scaled->get_height());
            if (mouseX >= x && mouseY >= y) {
                int w = shape->toRealWidth(m_scaled->get_width());
                int h = shape->toRealHeight(m_scaled->get_height());
                if (mouseX < x + w && mouseY < y + h) {
                    m_relX = (mouseX - x);
                    m_relY = (mouseY - y);
                    m_selected = shape;
                    break;
                }
            }
        }
    }
    //g_warning("button: %dx%d btn:0x%04x", event->x, event->y, event->button);
    return TRUE;
}

void
Preview::setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
    m_pixbuf = pixbuf;
    m_scaled.reset();
    queue_draw();
}

Glib::RefPtr<Gdk::Pixbuf>
Preview::getPixbuf()
{
    return m_pixbuf;
}

void
Preview::render(const Cairo::RefPtr<Cairo::Context>& cairoCtx,
        const Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    Gdk::Cairo::set_source_pixbuf(cairoCtx, pixbuf, 0, 0);
    cairoCtx->rectangle(0, 0, pixbuf->get_width(), pixbuf->get_height());
    cairoCtx->fill();
    for (auto shape : m_shapes) {
        shape->render(cairoCtx, pixbuf->get_width(), pixbuf->get_height());
    }
}

bool
Preview::save(const Glib::ustring& file)
{
    if (m_pixbuf) {
        Cairo::RefPtr<Cairo::ImageSurface> outpixmap = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
            m_pixbuf->get_width()
            , m_pixbuf->get_height());
        {
            Cairo::RefPtr<Cairo::Context> cairoCtx = Cairo::Context::create(outpixmap);
            render(cairoCtx, m_pixbuf);
        }
        outpixmap->write_to_png(file);
        return true;
    }
    return false;
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
        render(cairoCtx, m_scaled);
    }
    return true;
}

