/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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

#include <StringUtils.hpp>
#include <ImageUtils.hpp>

#include "ScanPreview.hpp"

ScanPreview::ScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::DrawingArea(cobject)
{
    add_events(Gdk::EventMask::BUTTON_PRESS_MASK
        | Gdk::EventMask::BUTTON_RELEASE_MASK
        | Gdk::EventMask::POINTER_MOTION_MASK
        | Gdk::EventMask::BUTTON_MOTION_MASK);
}

bool
ScanPreview::on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx) {
    if (m_pixbuf) {
        double wScale = static_cast<double>(get_width()) / static_cast<double>(m_pixbuf->get_width());
        double hScale = static_cast<double>(get_height()) / static_cast<double>(m_pixbuf->get_height());
        m_scale = std::min(wScale, hScale);
        int scaledWidth = static_cast<int>(static_cast<double>(m_pixbuf->get_width()) * m_scale);
        int scaledHeight = static_cast<int>(static_cast<double>(m_pixbuf->get_height()) * m_scale);
        if (!m_scaled
         || (std::abs(scaledWidth - m_scaled->get_width()) > 10
         &&  std::abs(scaledHeight - m_scaled->get_height()) > 10)) {   // scale with steps not every pixel
            //std::cout << "scaling "
            //          << " width " << scaledWidth
            //          << " height " << scaledHeight << std::endl;
            m_scaled = m_pixbuf->scale_simple(scaledWidth, scaledHeight, Gdk::InterpType::INTERP_BILINEAR);
         }
        int displayedWidth = m_scaled->get_width();
        int displayedHeight = m_scaled->get_height();
        Gdk::Cairo::set_source_pixbuf(cairoCtx, m_scaled, 0, 0);
        cairoCtx->rectangle(0, 0, displayedWidth, displayedHeight);
        cairoCtx->fill();
        if (m_showMask) {
             // draw mask
             cairoCtx->set_fill_rule(Cairo::FillRule::FILL_RULE_EVEN_ODD);
             cairoCtx->rectangle(0, 0, displayedWidth, displayedHeight);
             double x = convertRel2X(m_xstart);
             double y = convertRel2Y(m_ystart);
             double width = convertRel2X(m_xend - m_xstart);
             double height = convertRel2Y(m_yend - m_ystart);
             cairoCtx->rectangle(x, y, width, height);
             cairoCtx->set_source_rgba(0.5, 0.5, 0.5, 0.5);
             cairoCtx->fill();
        }
    }
    return true;
}

Gdk::CursorType
ScanPreview::getCursor(GdkEventMotion* motion_event)
{
    const auto sensitifity = 12;
    const auto sensitifity_2 = sensitifity / 2;
    double mouseX = motion_event->x;
    double mouseY = motion_event->y;
    double x1 = convertRel2X(m_xstart);
    double y1 = convertRel2Y(m_ystart);
    double x2 = convertRel2X(m_xend);
    double y2 = convertRel2Y(m_yend);
    Gdk::CursorType cursorType = Gdk::CursorType::ARROW;
    if (mouseY >= (y1-sensitifity_2) && mouseY <= (y2+sensitifity_2)) {
        if (std::abs(mouseX - x1) < sensitifity) {
            // left
            cursorType = Gdk::CursorType::LEFT_SIDE;
        }
        if (std::abs(mouseX - x2) < sensitifity && mouseY >= y1 && mouseY <= y2) {
            // right
            cursorType = Gdk::CursorType::RIGHT_SIDE;
        }
    }
    if (mouseX >= (x1-sensitifity_2) && mouseX <= (x2+sensitifity_2)) {
        if (std::abs(mouseY - y1) < sensitifity) {
            // top
            cursorType = Gdk::CursorType::TOP_SIDE;
        }
        if (std::abs(mouseY - y2) < sensitifity) {
            // bottom
            cursorType = Gdk::CursorType::BOTTOM_SIDE;
        }
    }
    return cursorType;
}

bool
ScanPreview::on_button_release_event(GdkEventButton* event)
{
    auto gdkWindow = get_window();
    if (m_changedCursor) {
        gdkWindow->set_cursor();
        m_changedCursor = false;
    }
    return false;
}

bool
ScanPreview::on_motion_notify_event(GdkEventMotion* motion_event)
{
    bool btn1 = (motion_event->state  & Gdk::ModifierType::BUTTON1_MASK) != 0x0;
    Gdk::CursorType cursorType = getCursor(motion_event);
    if (m_showMask) {
        auto gdkWindow = get_window();
        if (cursorType != Gdk::CursorType::ARROW) {
            Glib::RefPtr<Gdk::Cursor> display_cursor = Gdk::Cursor::create(gdkWindow->get_display(), cursorType);
            gdkWindow->set_cursor(display_cursor);
            m_changedCursor = true;
        }
        else {
            gdkWindow->set_cursor();
            m_changedCursor = false;
        }
    }
    if (btn1 && m_scaled) {
        bool redraw = false;
        double mouseX = motion_event->x;
        double mouseY = motion_event->y;
        double displayedWidth = m_scaled->get_width();
        double displayedHeight = m_scaled->get_height();
        if (cursorType == Gdk::CursorType::LEFT_SIDE) {
            m_xstart = mouseX / displayedWidth;
            redraw = true;
        }
        else if (cursorType == Gdk::CursorType::RIGHT_SIDE) {
            m_xend = mouseX / displayedWidth;
            redraw = true;
        }
        else if (cursorType == Gdk::CursorType::TOP_SIDE) {
            m_ystart = mouseY / displayedHeight;
            redraw = true;
        }
        else if (cursorType ==  Gdk::CursorType::BOTTOM_SIDE) {
            m_yend = mouseY / displayedHeight;
            redraw = true;
        }
        if (redraw) {
            queue_draw();
        }
        return TRUE;
    }
    return FALSE;
}

void
ScanPreview::setShowMask(bool showMask)
{
    m_showMask = showMask;
}

bool
ScanPreview::getShowMask()
{
    return m_showMask;
}

Glib::RefPtr<Gdk::Pixbuf>
ScanPreview::getPixbuf()
{
    return m_pixbuf;
}

double
ScanPreview::convertRel2X(double relx)
{
    double x = 0.0;
    if (m_scaled) {
        int displayedWidth = m_scaled->get_width();
        x = relx * displayedWidth;
    }
    return x;
}

double
ScanPreview::convertRel2Y(double rely)
{
    double y = 0.0;
    if (m_scaled) {
        int displayedHeight = m_scaled->get_height();
        y = rely * displayedHeight;
    }
    return y;
}

bool
ScanPreview::savePng(const Glib::ustring& file)
{
    bool ret = false;
    if (m_pixbuf) {
        m_pixbuf->save(file, "png");
        ret = true;
        //}
        //else if (m_bytePerPixel == 1){
        //    ImageUtils::grayscalePng(m_pixbuf, file);
        //    ret = true;
        //}
        //else {
        //    ImageUtils::blackandwhitePng(m_pixbuf, file);
        //    ret = true;
        //}
    }
    return ret;
}
