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
#include "TextShape.hpp"

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

void
Preview::addText(const TextInfo& text)
{
    auto textShape = std::make_shared<TextShape>();
    textShape->setText(text);
    m_shapes.push_back(textShape);
}

void
Preview::create(std::array<int,2> size, const Gdk::Color& background)
{
    m_pixbuf = Gdk::Pixbuf::create(
            Gdk::Colorspace::COLORSPACE_RGB
            , false
            , 8
            , size[0]
            , size[1]);
    guint32 pixel = (background.get_red() >> 8u) << 24u
                  | (background.get_green() >> 8u) << 16u
                  | (background.get_blue())      // eliminated >> 8u) << 8u as this will be a no op
                  | 0xffu;
    m_pixbuf->fill(pixel);
    m_shapes.clear();
    m_selected.reset();
    queue_draw();
}


bool
Preview::on_motion_notify_event(GdkEventMotion* motion_event)
{
    bool btn1 = (motion_event->state  & Gdk::ModifierType::BUTTON1_MASK) != 0x0;
    if (btn1) {
        if (m_selected && m_scaled) {
            Gdk::Rectangle old = m_selected->getBounds(m_scaled->get_width(), m_scaled->get_height());
            double relX = (motion_event->x - m_relX) / static_cast<double>(m_scaled->get_width());
            double relY = (motion_event->y - m_relY) / static_cast<double>(m_scaled->get_height());
            m_selected->setRelPosition(relX, relY);
            Gdk::Rectangle next = m_selected->getBounds(m_scaled->get_width(), m_scaled->get_height());
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
            auto r =shape->getBounds(m_scaled->get_width(), m_scaled->get_height());
            if (mouseX >= r.get_x()
             && mouseX < r.get_x() + r.get_width()
             && mouseY >= r.get_y()
             && mouseY < r.get_y() + r.get_height()) {
                m_relX = (mouseX - r.get_x());
                m_relY = (mouseY - r.get_y());
                m_selected = shape;
                break;
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

