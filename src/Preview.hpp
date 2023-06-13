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

#pragma once

#include <gtkmm.h>

class NomadWin;
class Shape;
class TextInfo;

class Preview
: public Gtk::DrawingArea
{
public:
    Preview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin);
    virtual ~Preview() = default;

    void setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);
    Glib::RefPtr<Gdk::Pixbuf> getPixbuf();

    void loadSvg(const Glib::RefPtr<Gio::File>& f);
    bool saveImage(const Glib::ustring& file);
    void loadImage(const Glib::RefPtr<Gio::File>& f);
    void addText(const TextInfo& text);
    void create(std::array<int,2> size, const Gdk::Color& background);
    void add(const std::shared_ptr<Shape>& shape);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx) override;
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_press_event(GdkEventButton* event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    void render(const Cairo::RefPtr<Cairo::Context>& cairoCtx,
        const Glib::RefPtr<Gdk::Pixbuf> pixbuf);

private:
    NomadWin* m_nomadWin;
    std::list<std::shared_ptr<Shape>> m_shapes;
    std::shared_ptr<Shape> m_selected;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    Glib::RefPtr<Gdk::Pixbuf> m_scaled;
    double m_relX{0.0};
    double m_relY{0.0};
};



