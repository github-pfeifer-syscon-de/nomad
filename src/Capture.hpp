/* screenshot-backend-x11.h - Fallback X11 backend
 *
 * Copyright (C) 2020 Alexander Mikhaylenko <alexm@gnome.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 */

#pragma once

#include <glib-object.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>

#define HAVE_X11

#pragma once

class Capture
{
public:
    Capture() = default;
    explicit Capture(const Capture& orig) = delete;
    virtual ~Capture() = default;

    virtual GdkPixbuf *get_pixbuf(GdkRectangle* rectangle) = 0;

    bool get_take_window_shot() {
        return m_take_window_shot;
    }
    void set_take_window_shot(bool take_window_shot) {
        m_take_window_shot = take_window_shot;
    }
    bool get_take_area_shot() {
        return m_take_area_shot;
    }
    void set_take_area_shot(bool take_area_shot) {
        m_take_area_shot = take_area_shot;
    }
    bool get_include_pointer() {
        return m_include_pointer;
    }
    void set_include_pointer(bool include_pointer) {
        m_include_pointer = include_pointer;
    }
private:
protected:
    bool m_take_window_shot{false};
    bool m_take_area_shot{false};
    bool m_include_pointer{false};

};

#ifdef HAVE_X11

class X11Capture
: public Capture
{
public:
    X11Capture() = default;
    explicit X11Capture(const X11Capture& orig) = delete;
    virtual ~X11Capture() = default;

    GdkPixbuf *get_pixbuf(GdkRectangle* rectangle) override;

private:
    void blank_rectangle_in_pixbuf(
            GdkPixbuf *pixbuf,
            GdkRectangle *rect);
    void blank_region_in_pixbuf(
            GdkPixbuf *pixbuf,
            cairo_region_t *region);
    cairo_region_t *make_region_with_monitors(
            GdkDisplay *display);
    void mask_monitors (GdkPixbuf *pixbuf,
            GdkWindow *root_window);
    void get_window_rect_coords(GdkWindow    *window,
            GdkRectangle *real_coordinates_out,
            GdkRectangle *screenshot_coordinates_out);
    GdkWindow* do_find_current_window();
    GdkWindow* find_active_window();
    gboolean window_is_desktop(
            GdkWindow* current_window);
    GdkWindow* fallback_find_current_window();
    Window find_wm_window(
            GdkWindow* window);
};

#endif