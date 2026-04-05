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
#pragma once

#include <gtkmm.h>

class ScanPreview
: public Gtk::DrawingArea
{
public:
    ScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    explicit ScanPreview(const ScanPreview &other) = delete;
    virtual ~ScanPreview();

    inline double getXStart() {
        return m_xstart;
    }
    inline double getYStart() {
        return m_ystart;
    }
    inline double getXEnd() {
        return m_xend;
    }
    inline double getYEnd() {
        return m_yend;
    }
    void setShowMask(bool showMask);
    bool getShowMask();
    Glib::RefPtr<Gdk::Pixbuf> getPixbuf();
    bool savePng(const Glib::ustring& file);
    void saveGrayscale(const Glib::ustring& file);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx) override;
    double convertRel2X(double relx);
    double convertRel2Y(double rely);
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    Gdk::CursorType getCursor(GdkEventMotion* motion_event);

    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    Glib::RefPtr<Gdk::Pixbuf> m_scaled;
    double m_scale{1.0};
    double m_xstart{0.1};
    double m_ystart{0.1};
    double m_xend{0.9};
    double m_yend{0.9};
    bool m_showMask{true};
    bool m_changedCursor{false};

};