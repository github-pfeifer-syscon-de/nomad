/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include <cmath>
#include <memory>

class BasePoint
{
public:
    BasePoint(double _x, double _y)
    : m_x{_x}
    , m_y{_y}
    {
    }
    virtual ~BasePoint() = default;
    double getX() const
    {
        return m_x;
    }
    void setX(double _x)
    {
        m_x = _x;
    }
    double getY() const
    {
        return m_y;
    }
    void setY(double _y)
    {
        m_y = _y;
    }
protected:
    double m_x,m_y;

};

class MinMax
{
public:
    MinMax()
    {
        reset();
    }
    virtual ~MinMax() = default;
    void add(const std::shared_ptr<BasePoint>& p)
    {
        if (p->getX() < xmin) {
            xmin = p->getX();
        }
        if (p->getX() > xmax) {
            xmax = p->getX();
        }
        if (p->getY() < ymin) {
            ymin = p->getY();
        }
        if (p->getY() > ymax) {
            ymax = p->getY();
        }
    }
    double dist()
    {
        double dx = xmax - xmin;
        double dy = ymax - ymin;
        return std::sqrt(dx * dx + dy * dy);
    }
    void reset()
    {
        xmin = std::numeric_limits<double>::max();
        xmax = std::numeric_limits<double>::min();
        ymin = std::numeric_limits<double>::max();
        ymax = std::numeric_limits<double>::min();
    }
protected:
    double xmin;
    double xmax;
    double ymin;
    double ymax;
};


class DrawPoint
: public BasePoint
{
public:
    DrawPoint(bool _draw, double _x, double _y)
    : BasePoint(_x, _y)
    , m_draw{_draw}
    {
    }
    virtual ~DrawPoint() = default;
    bool isDraw()
    {
        return m_draw;
    }
protected:
    bool m_draw;
};


enum class Direction
{
    none,
    Up,
    Right,
    Down,
    Left
};

class DirPoint
: public DrawPoint
{
public:
    DirPoint(Direction _dir, bool _draw, double _x, double _y, double _l)
    : DrawPoint(_draw, _x, _y)
    , m_dir{_dir}
    , m_l{_l}
    {
    }
    virtual ~DirPoint() = default;
    Direction getDirection()
    {
        return m_dir;
    }
    double getLength()
    {
        return m_l;
    }
protected:
    Direction m_dir;
    double m_l;
};

class PenlWindow
: public Gtk::Window
{
public:
    PenlWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Gtk::Application* application);
    virtual ~PenlWindow() = default;

    void on_action_preferences();
    void on_action_about();
protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
    bool on_motion_notify_event(GdkEventMotion* motion) override;
    std::list<std::shared_ptr<DrawPoint>> smoothing(
            std::list<std::shared_ptr<DrawPoint>>& pnts
            , double factor);
    std::list<std::shared_ptr<DrawPoint>> thinnig(
            std::list<std::shared_ptr<DrawPoint>>& pnts
            , double distance);
    std::list<std::shared_ptr<DirPoint>> direction(
            std::list<std::shared_ptr<DrawPoint>>& pnts);
    std::list<std::shared_ptr<DirPoint>> direction2(
            std::list<std::shared_ptr<DrawPoint>>& pnts);
    void draw(const Cairo::RefPtr<Cairo::Context>& cr, std::list<std::shared_ptr<DrawPoint>>& path, bool points = false);
    void draw(const Cairo::RefPtr<Cairo::Context>& cr, std::list<std::shared_ptr<DirPoint>>& path);
    void writePathDetail(guint chr);
    void writePathDir(guint chr);

    static constexpr auto SmoothFactor = 0.75;
    static constexpr auto OffCoord = -10000.0;
    static constexpr auto ThinDistance = 7.0;
    static constexpr auto ThinDistance_2 = ThinDistance * ThinDistance;
    static constexpr auto ArrowLen = 10.0;
    static constexpr auto ArrowLen2 = ArrowLen / 2.0;
private:
    Gtk::DrawingArea *m_drawingArea;
    Glib::RefPtr<Gdk::Device> m_pen;
    Glib::RefPtr<Gdk::Device> m_eraser;
    Gtk::Application* m_application;
    std::list<std::shared_ptr<DrawPoint>> m_path;
    bool m_draw{false};
};


