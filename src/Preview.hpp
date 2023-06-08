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
#include <librsvg/rsvg.h>

class NomadWin;

class RSvg {
public:
    RSvg();
    explicit RSvg(const RSvg&) = delete;
    virtual ~RSvg();

    bool from_file(const Glib::RefPtr<Gio::File>& f);
    bool pixel_size(gdouble* svgWidth, gdouble* svgHeight);
    bool render(const Cairo::RefPtr<Cairo::Context>& cairoCtx, int width, int height);
private:
    RsvgHandle* m_handle;
};

class Preview
: public Gtk::DrawingArea
{
public:
    Preview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin);
    virtual ~Preview() = default;

    void setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx) override;
protected:
    void loadSVG(std::string const& filename);

private:
    NomadWin* m_nomadWin;
    std::shared_ptr<RSvg> m_svg;
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    Glib::RefPtr<Gdk::Pixbuf> m_scaled;
};



