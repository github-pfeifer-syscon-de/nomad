/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 rpf
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
#include <thread>

#include "GenericCallback.hpp"
class WiaValue;
class NomadWin;

#ifdef __WIN32__
class WiaDataCallback;
// put the scanning into a separate class
//   subclassing std::thread is discouraged...
class WorkThread
{
public:
    WorkThread(
            Glib::Dispatcher& dispatcher
            , Glib::Dispatcher& completed
            , const Glib::ustring& deviceId
            , const std::map<uint32_t, WiaValue>& properties);
    virtual ~WorkThread();
    void run();
    WiaDataCallback* getDataCallback();
    bool getResult();
private:
    Glib::Dispatcher& m_dispatcher;
    Glib::Dispatcher& m_completed;
    WiaDataCallback* m_pCallback;
    bool m_result{false};
    Glib::ustring m_deviceId;
    std::map<uint32_t, WiaValue> m_properties;

};
#endif

class ScanPreview
: public Gtk::DrawingArea
{
public:
    ScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Glib::Dispatcher& completed);
    explicit ScanPreview(const ScanPreview& orig) = delete;
    virtual ~ScanPreview();

    void scan(const Glib::ustring& devId, const std::map<uint32_t, WiaValue>& properties);
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
    bool getResult();
    void cleanup();
    Glib::RefPtr<Gdk::Pixbuf> getPixbuf();
    bool savePng(const Glib::ustring& file);

protected:
    void scanProgress();
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx);
    void create(std::array<int,2> size, const Gdk::Color& background);
    double convertRel2X(double relx);
    double convertRel2Y(double rely);
    bool on_motion_notify_event(GdkEventMotion* motion_event) override;
    bool on_button_release_event(GdkEventButton* event) override;
    Gdk::CursorType getCursor(GdkEventMotion* motion_event);
    void saveGrayscale(const Glib::ustring& file);
private:
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    Glib::RefPtr<Gdk::Pixbuf> m_scaled;
    std::thread* m_workThread{nullptr};
    WorkThread* m_worker{nullptr};
    Glib::Dispatcher m_dispatcher;
    Glib::Dispatcher& m_completed;
    bool m_initScan{false};
    int32_t m_RowLast{0};
    double m_scale{1.0};
    double m_xstart{0.1};
    double m_ystart{0.1};
    double m_xend{0.9};
    double m_yend{0.9};
    bool m_showMask{true};
    bool m_changedCursor{false};
    uint32_t m_bytePerPixel{1};
};

