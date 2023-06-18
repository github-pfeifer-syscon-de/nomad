/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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

class NomadWin;

#ifdef __WIN32__
class WiaDataCallback;
// put the scanning into a separate class
//   subclassing std::thread is discouraged...
class WorkThread
{
public:
    WorkThread(Glib::Dispatcher& dispatcher);
    virtual ~WorkThread();
    void run();
    WiaDataCallback* getDataCallback();
private:
    Glib::Dispatcher& m_dispatcher;
    WiaDataCallback* m_pCallback;

};
#endif

class ScanPreview
: public Gtk::DrawingArea
{
public:
    ScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
    explicit ScanPreview(const ScanPreview& orig) = delete;
    virtual ~ScanPreview() = default;

    void scan();
protected:
    void scanProgress();
    bool on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx);
    void create(std::array<int,2> size, const Gdk::Color& background);
private:
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    Glib::RefPtr<Gdk::Pixbuf> m_scaled;
    std::thread* m_workThread{nullptr};
    WorkThread* m_worker{nullptr};
    Glib::Dispatcher m_dispatcher;
    bool m_initScan{false};
    int32_t m_RowLast{0};
    double m_scale{1.0};
};

