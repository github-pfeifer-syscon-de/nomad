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


#include "PenlAppWindow.hpp"
#include "PenlApp.hpp"

PenlAppWindow::PenlAppWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, PenlApp *application)
: Gtk::Window(cobject)       //Calls the base class constructor
, m_drawingArea{nullptr}
, m_application{application}
{
    Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(
            application->get_resource_base_path() + "/penl.png");
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
    m_drawingArea->signal_motion_notify_event().connect(sigc::mem_fun(*this, &PenlAppWindow::on_motion));

    m_drawingArea->signal_button_press_event().connect(
        [=] (GdkEventButton* button) {
        std::cout << "button " << button->button << std::endl;
        return true;
    });
    m_drawingArea->signal_draw().connect(
        [=] (const Cairo::RefPtr<Cairo::Context>& cr) {
        //double x1,y1,x2,y2;
        //cr->get_clip_extents(x1, y1, x2, y2);
        cr->set_source_rgb(1.0,1.0,1.0);
        cr->set_line_width(1.0);
        for (auto pnt : m_path) {
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
        cr->stroke();
        return false;
    });
    m_drawingArea->set_can_focus(true);
    m_drawingArea->signal_key_press_event().connect([=] (GdkEventKey* keyEvent) {
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


PenlAppWindow::~PenlAppWindow()
{
    //std::cout << "PenlAppWindow::~PenlAppWindow " << std::endl;
}

bool
PenlAppWindow::on_motion(GdkEventMotion* motion)
{
    auto src = gdk_event_get_source_device((GdkEvent*)motion);
    if (src) {
        auto dev = Glib::wrap(src, true);
        if (dev->get_source() == Gdk::InputSource::SOURCE_PEN)  {    // only listen to pen
            guint button = 0;
            double localX{motion->x};
            double localY{motion->y};
            //translate_coordinates(*m_drawingArea, (int)motion->x, (int)motion->y, localX, localY);
            //std::cout << "motion x " << motion->x << " y " << motion->y
            //          << " local x " << localX << " y " << localY << std::endl;
            gdk_event_get_button((GdkEvent*)motion, &button);
            double press;
            if (gdk_event_get_axis((GdkEvent*)motion, GdkAxisUse::GDK_AXIS_PRESSURE, &press)) {
                std::shared_ptr<Point> pnt;
                if (press > 0.0) {
                    if (!m_draw) {
                        pnt = std::make_shared<Point>(false, localX, localY);
                        m_draw = true;
                    }
                    else {
                        double lx{0.0},ly{0.0};
                        if (!m_path.empty()) {
                            lx = m_path.back()->getX();
                            ly = m_path.back()->getY();
                        }
                        if (std::abs(lx - localX) > 5.0 ||
                            std::abs(ly - localY) > 5.0) {   // avoid recording too many points
                            pnt = std::make_shared<Point>(true, localX, localY);
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

//bool
//PenlAppWindow::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
//{
//}

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
PenlAppWindow::on_action_preferences()
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



//    Gtk::Dialog *dlg = m_monglView->monitors_config();
//    dlg->set_transient_for(*this);
//    dlg->run();
//    m_monglView->save_config();
//    dlg->hide();
//    delete dlg;
}

void
PenlAppWindow::on_action_about()
{
    auto refBuilder = Gtk::Builder::create();
    try {
        refBuilder->add_from_resource(
            m_application->get_resource_base_path() + "/abt-dlg.ui");
        auto object = refBuilder->get_object("abt-dlg");
        auto abtdlg = Glib::RefPtr<Gtk::AboutDialog>::cast_dynamic(object);
        if (abtdlg) {
            Glib::RefPtr<Gdk::Pixbuf> pix = Gdk::Pixbuf::create_from_resource(
                    m_application->get_resource_base_path() +"/penl.png");
            abtdlg->set_logo(pix);
            abtdlg->set_transient_for(*this);
            abtdlg->run();
            abtdlg->hide();
        } else
            std::cerr << "PenlAppWindow::on_action_about(): No \"abt-dlg\" object in abt-dlg.ui"
                << std::endl;
    } catch (const Glib::Error& ex) {
        std::cerr << "PenlAppWindow::on_action_about(): " << ex.what() << std::endl;
    }

}
