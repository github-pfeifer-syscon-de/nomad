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


#include <gdk/gdkx.h>

#include "Capture.hpp"


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

