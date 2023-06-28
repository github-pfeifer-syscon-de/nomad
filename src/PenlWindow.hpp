/*
 * Copyright (C) 2018 rpf
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

class Point {
private:
    bool m_draw{false};
    double m_x{0.0},m_y{0.0};
public:
    Point(bool _draw, double _x, double _y)
    : m_draw{_draw}
    , m_x{_x}
    , m_y{_y}
    {
    }
    bool isDraw()
    {
        return m_draw;
    }
    double getX()
    {
        return m_x;
    }
    double getY()
    {
        return m_y;
    }
};

class PenlApp;

class PenlAppWindow : public Gtk::ApplicationWindow {
public:
    PenlAppWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PenlApp *application);
    virtual ~PenlAppWindow();

    void on_action_preferences();
    void on_action_about();
protected:
    //bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);
    bool on_motion(GdkEventMotion* motion);
private:
    Gtk::DrawingArea *m_drawingArea;
    Glib::RefPtr<Gdk::Device> m_pen;
    Glib::RefPtr<Gdk::Device> m_eraser;
    Gtk::Application* m_application;
    std::list<std::shared_ptr<Point>> m_path;
    bool m_draw{false};
};


