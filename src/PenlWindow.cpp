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

#include <iostream>
#include <gtkmm.h>
#include <cmath>
#include <stdio.h>


#include "PenlWindow.hpp"

// see https://jackschaedler.github.io/handwriting-recognition/
// for description

PenlWindow::PenlWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Gtk::Application* application)
: Gtk::Window(cobject)       //Calls the base class constructor
, m_drawingArea{nullptr}
, m_application{application}
{
    Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(
            application->get_resource_base_path() + "/nomad.png");
    set_icon(pix);
    builder->get_widget("drawing", m_drawingArea);
    m_drawingArea->set_app_paintable(true);
    m_drawingArea->add_events(Gdk::EventMask::BUTTON_PRESS_MASK
               | Gdk::EventMask::BUTTON_MOTION_MASK
               | Gdk::EventMask::SCROLL_MASK
               | Gdk::EventMask::KEY_PRESS_MASK
               | Gdk::EventMask::LEAVE_NOTIFY_MASK);
    // set_events is wrong!
    //Gdk::EventMask::POINTER_MOTION_MASK
    //                        | Gdk::EventMask::ALL_EVENTS_MASK); // button_motion,touch wont work
    //m_drawingArea->signal_motion_notify_event().connect(sigc::mem_fun(*this, &PenlWindow::on_motion));
    //m_drawingArea->signal_button_press_event().connect(
    //    [=] (GdkEventButton* button) {
    //    std::cout << "button " << button->button << std::endl;
    //    return true;
    //});
    //m_drawingArea->signal_draw().connect(sigc::mem_fun(*this, &PenlWindow::draw));
    m_drawingArea->set_can_focus(true);
    m_drawingArea->signal_key_press_event().connect(
        [this] (GdkEventKey* keyEvent) {
        //guint16 keyCode;
        //gdk_event_get_keycode((GdkEvent*)keyEvent, &keyCode);
        if ((keyEvent->keyval >= GDK_KEY_A
         && keyEvent->keyval <= GDK_KEY_Z)
         || (keyEvent->keyval >= 'a'
         &&  keyEvent->keyval <= 'z' )
         || (keyEvent->keyval >= '0'
         &&  keyEvent->keyval <= '9' )) {
            Glib::ustring name = Glib::ustring::sprintf("%c.path", keyEvent->keyval);
            std::cout << "saving " << name << std::endl;
            auto f = Gio::File::create_for_path(name);
            auto os = f->replace("",false,Gio::FileCreateFlags::FILE_CREATE_REPLACE_DESTINATION);
            for (auto pnt : m_path) {
                Glib::ustring seg = Glib::ustring::sprintf("%c %f %f ",
                    (pnt->isDraw() ? 'L' : 'M'), pnt->getX(), pnt->getY());
                os->write(seg);
            }
            os->write("\n");
            os->close();
            m_path.clear();
            m_drawingArea->queue_draw();
        }
        return true;
    });
    set_default_size(480, 640);
    show_all_children();
}

void
PenlWindow::draw(const Cairo::RefPtr<Cairo::Context>& cr, std::list<std::shared_ptr<DrawPoint>>& path, bool points)
{
    for (auto pnt : path) {
        double localX{pnt->getX()};
        double localY{pnt->getY()};
        //m_drawingArea->translate_coordinates(*this, (int)pnt->getX(), (int)pnt->getY(), localX, localY);
        if (!pnt->isDraw()) {
            cr->move_to(localX, localY);
        }
        else {
            cr->line_to(localX, localY);
        }
    }
    if (points) {
        for (auto pnt : path) {
            double localX{pnt->getX()};
            double localY{pnt->getY()};
            cr->arc(localX, localY, 3.0, 0.0, M_PI * 2.0);
        }
    }
    cr->stroke();
}

void
PenlWindow::draw(const Cairo::RefPtr<Cairo::Context>& cr, std::list<std::shared_ptr<DirPoint>>& path)
{
    for (auto pnt : path) {
        double localX{pnt->getX()};
        double localY{pnt->getY()};
        switch (pnt->getDirection()) {
        case Direction::Up:
            cr->move_to(localX - ArrowLen2, localY - ArrowLen);
            cr->line_to(localX, localY);
            cr->line_to(localX + ArrowLen2, localY - ArrowLen);
            break;
        case Direction::Down:
            cr->move_to(localX - ArrowLen2, localY + ArrowLen);
            cr->line_to(localX, localY);
            cr->line_to(localX + ArrowLen2, localY + ArrowLen);
            break;
        case Direction::Left:
            cr->move_to(localX - ArrowLen, localY - ArrowLen2);
            cr->line_to(localX, localY);
            cr->line_to(localX - ArrowLen, localY + ArrowLen2);
            break;
        case Direction::Right:
            cr->move_to(localX + ArrowLen, localY - ArrowLen2);
            cr->line_to(localX, localY);
            cr->line_to(localX + ArrowLen, localY + ArrowLen2);
            break;
        case Direction::none:
            break;
        }
    }
    cr->stroke();
}


bool
PenlWindow::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    //double x1,y1,x2,y2;
    //cr->get_clip_extents(x1, y1, x2, y2);
    cr->set_source_rgb(0.5,0.5,0.5);
    cr->set_line_width(1.0);
    draw(cr, m_path);
    // integrated thinning in event proc so this should net be necessary
    //auto smooth = smoothing(m_path, Smooth_factor);
    //cr->set_source_rgb(0.5,0.7,0.5);
    //draw(cr, smooth);
    //std::cout << "smooth "  << smooth.size() << std::endl;
    //auto thin = thinnig(smooth, Thin_distance);
    //cr->set_source_rgb(0.5,0.5,0.7);
    //draw(cr, thin, true);
    //std::cout << "thin "  << thin.size() << std::endl;
    auto dirs = direction(m_path);
    cr->set_source_rgb(0.7,0.5,0.5);
    draw(cr, dirs);
    std::cout << "dir "  << dirs.size() << std::endl;
    return false;
}

std::list<std::shared_ptr<DirPoint>>
PenlWindow::direction(
        std::list<std::shared_ptr<DrawPoint>>& pnts)
{
    std::list<std::shared_ptr<DirPoint>> ret;
    double sumX = 0.0;
    double sumY = 0.0;
    int count = 0;
    double lastX = OffCoord;
    double lastY = OffCoord;
    Direction dir{Direction::none};
    for (auto pnt : pnts) {
        double dX = lastX != OffCoord ? lastX - pnt->getX() : 0.0;
        double dY = lastY != OffCoord ? lastY - pnt->getY() : 0.0;

        Direction next{Direction::none};
        if (dX != 0.0 || dY != 0.0) {
            if (std::abs(dX) > std::abs(dY)) {
                next = dX < 0.0 ? Direction::Left : Direction::Right;
            }
            else {
                next = dY < 0.0 ? Direction::Up : Direction::Down;
            }
        }
        sumX += pnt->getX();
        sumY += pnt->getY();
        ++count;
        if ((next != Direction::none
         && next != dir) || !pnt->isDraw()) {
            if (!ret.empty()) {
                auto last = ret.back();
                last->setX(sumX / static_cast<double>(count));
                last->setY(sumY  / static_cast<double>(count));
            }
            auto nextPnt = std::make_shared<DirPoint>(next, pnt->isDraw(), pnt->getX(), pnt->getY());
            ret.push_back(std::move(nextPnt));
            sumX = 0.0;
            sumY = 0.0;
            count = 0;
            dir = next;
        }
        if (!pnt->isDraw()) {
            lastX = OffCoord;
            lastY = OffCoord;
            dir = Direction::none;
        }
        else {
            lastX = pnt->getX();
            lastY = pnt->getY();
        }
    }
    if (!ret.empty()) {
        auto last = ret.back();
        last->setX(sumX / static_cast<double>(count));
        last->setY(sumY  / static_cast<double>(count));
    }

    return ret;
}


std::list<std::shared_ptr<DrawPoint>>
PenlWindow::thinnig(
        std::list<std::shared_ptr<DrawPoint>>& pnts
        , double distance)
{
    std::list<std::shared_ptr<DrawPoint>> ret;
    double lastX = OffCoord;
    double lastY = OffCoord;
    double dist2 = distance * distance;     // save sqrt
    for (auto pnt : pnts) {
        double dX = lastX - pnt->getX();
        double dY = lastY - pnt->getY();
        double dist = dX * dX + dY * dY;
        //std::cout << " x " << pnt->getX()
        //          << " y " << pnt->getY()
        //          << " dX " << dX
        //          << " dY " << dY
        //          << " dist " << dist
        //          << std::endl;
        if (dist > dist2) {
            auto next = std::make_shared<DrawPoint>(pnt->isDraw(), pnt->getX(), pnt->getY());
            ret.push_back(std::move(next));
            lastX = pnt->getX();
            lastY = pnt->getY();
        }
        if (!pnt->isDraw()) {
            lastX = OffCoord;
            lastY = OffCoord;
        }
    }
    return ret;
}


std::list<std::shared_ptr<DrawPoint>>
PenlWindow::smoothing(
        std::list<std::shared_ptr<DrawPoint>>& pnts
        , double factor)
{
    double lastX = OffCoord;
    double lastY = OffCoord;
    std::list<std::shared_ptr<DrawPoint>> ret;
    for (auto pnt : pnts) {
        double nextX = pnt->getX();
        double nextY = pnt->getY();
        if (lastX != OffCoord
         && lastY != OffCoord) {
            nextX = lastX * factor + pnt->getX() * (1.0 - factor);
            nextY = lastY * factor + pnt->getY() * (1.0 - factor);
        }
        auto next = std::make_shared<DrawPoint>(pnt->isDraw(), nextX, nextY);
        ret.push_back(std::move(next));
        lastX = pnt->isDraw() ? nextX : OffCoord;
        lastY = pnt->isDraw() ? nextY : OffCoord;
    }
    return ret;
}



bool
PenlWindow::on_motion_notify_event(GdkEventMotion* motion)
{
    auto src = gdk_event_get_source_device((GdkEvent*)motion);
    if (src) {
        auto dev = Glib::wrap(src, true);
        if (dev->get_source() == Gdk::InputSource::SOURCE_PEN)  {    // only listen to pen
            guint button = 0;
            double localX{motion->x};
            double localY{motion->y};
            //translate_coordinates(*m_drawingArea, (int)motion->x, (int)motion->y, localX, localY);
            gdk_event_get_button((GdkEvent*)motion, &button);
            double press;
            if (gdk_event_get_axis((GdkEvent*)motion, GdkAxisUse::GDK_AXIS_PRESSURE, &press)) {
                std::shared_ptr<DrawPoint> pnt;
                //std::cout << " local x " << localX
                //          << " y " << localY
                //          << " press " << press
                //          << std::endl;
                if (press > 0.0) {
                    if (!m_draw) {
                        pnt = std::make_shared<DrawPoint>(false, localX, localY);
                        m_draw = true;
                    }
                    else {
                        double lx{OffCoord};
                        double ly{OffCoord};
                        if (!m_path.empty()) {
                            lx = m_path.back()->getX();
                            ly = m_path.back()->getY();
                        }
                        double dx = lx - localX;
                        double dy = ly - localY;
                        if (dx * dx + dy * dy > ThinDistance) {   // avoid recording too many points, so this is already thinned
                            pnt = std::make_shared<DrawPoint>(m_draw, localX, localY);
                        }
                    }
                }
                else {
                    m_draw = false;
                }
                if (pnt) {
                    m_path.push_back(pnt);
                    if (m_path.size() % 100 == 0) {
                        std::cout << "Size " << m_path.size() << std::endl;
                    }
                    m_drawingArea->queue_draw();
                }
//                    std::cout << "type " << motion->type
//                              << " btn " << button
//                              << " press " << press
//                              << " dev " << dev->get_name()
//                              << " x " << motion->x
//                              << " y "<< motion->y << std::endl;
            }
        }
    }
    return true;
}


/****
 * dev Wacom Intuos3 9x12 Pen eraser
 source:Eraser
 mode:Disabled
 type:Slave
dev Wacom Intuos3 9x12 Pen stylus
 source:Pen
 mode:Disabled
 type:Slave
dev Razer Razer Orochi Keyboard
 source:Mouse
 mode:Disabled
 type:Slave
dev Virtual core XTEST pointer
 source:Mouse
 mode:Disabled
 type:Slave
dev Razer Razer Orochi
 source:Mouse
 mode:Disabled
 type:Slave
dev Wacom Intuos3 9x12 Pen cursor
 source:Cursor
 mode:Disabled
 type:Slave
dev Wacom Intuos3 9x12 Pad pad
 source:Tablet pad
 mode:Disabled
 type:Slave

 ****/
void
PenlWindow::on_action_preferences()
{
    auto seat = get_display()->get_default_seat();
    auto devs = seat->get_slaves(Gdk::SeatCapabilities::SEAT_CAPABILITY_ALL_POINTING);
    for (auto dev : devs) {
        std::cout << "dev " << dev->get_name() << std::endl;
        switch(dev->get_source()) {
        case Gdk::InputSource::SOURCE_MOUSE :
            std::cout << " source:" << "Mouse" << std::endl;
            break;
        case Gdk::InputSource::SOURCE_PEN :
            std::cout << " source:" << "Pen" << std::endl;
            break;
        case Gdk::InputSource::SOURCE_ERASER :
            std::cout << " source:" << "Eraser" << std::endl;
            break;
        case Gdk::InputSource::SOURCE_CURSOR :
            std::cout << " source:" << "Cursor" << std::endl;
            break;
        case Gdk::InputSource::SOURCE_TABLET_PAD :
            std::cout << " source:" << "Tablet pad" << std::endl;
            break;
        default:
            std::cout << " source:" << dev->get_source() << std::endl;
            break;
        }
        switch(dev->get_mode()) {
        case Gdk::InputMode::MODE_WINDOW :
            std::cout << " mode:" << "Window" << std::endl;
            break;
        case Gdk::InputMode::MODE_SCREEN :
            std::cout << " mode:" << "Screen" << std::endl;
            break;
        case Gdk::InputMode::MODE_DISABLED :
            std::cout << " mode:" << "Disabled" << std::endl;
            break;
        default:
            std::cout << " mode:" << dev->get_mode() << std::endl;
            break;
        }
        switch(dev->get_device_type()) {
        case Gdk::DeviceType::DEVICE_TYPE_FLOATING :
            std::cout << " type:" << "Float" << std::endl;
            break;
        case Gdk::DeviceType::DEVICE_TYPE_MASTER :
            std::cout << " type:" << "Master" << std::endl;
            break;
        case Gdk::DeviceType::DEVICE_TYPE_SLAVE :
            std::cout << " type:" << "Slave" << std::endl;
            break;
        default:
            std::cout << " type:" << dev->get_device_type() << std::endl;
            break;
        }
        if (dev->get_source() == Gdk::InputSource::SOURCE_PEN) {
            std::cout << "found pen!" << std::endl;
            m_pen = dev;
            //auto grabStatus = dev->grab(get_window(), Gdk::GrabOwnership::OWNERSHIP_APPLICATION,
            //    false, Gdk::EventMask::POINTER_MOTION_MASK,
            //    Gdk::Cursor::create(Gdk::CursorType::CENTER_PTR), GDK_CURRENT_TIME);
            //switch (grabStatus) {
            //case Gdk::GrabStatus::GRAB_SUCCESS:
            //    std::cout << " grab: Success" << std::endl;
            //    break;
            //case Gdk::GrabStatus::GRAB_FAILED:
            //    std::cout << " grab: failed" << std::endl;
            //    break;
            //default:
            //    std::cout << " grab: " << grabStatus << std::endl;
            //    break;
            //}
            //dev->signal_changed().connect([=] {    // e.g. grab, axes changed...
            //    std::cout << "signal_changed" << std::endl;
            //});
        }

    }
}

