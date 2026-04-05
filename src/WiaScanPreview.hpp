/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <future>

#include "ScanPreview.hpp"

class WiaValue;
class WiaValue2;
class WiaData;
class NomadWin;

#ifdef __WIN32__
class WiaDataCallback;
class TransferThread
{
public:
    TransferThread();
    virtual ~TransferThread();
    void run();
    void setCallback(WiaDataCallback* pCallback);
    void setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf);

    void setActive(bool active);
    bool isActive();
    bool isDone();
protected:
    void transferRow(uint8_t* srcData, uint32_t yPos, uint32_t transferPixels);
    void checkStart();
    void handleItem(const std::shared_ptr<WiaData>& item);
    
private:
    Glib::RefPtr<Gdk::Pixbuf> m_pixbuf;
    WiaDataCallback* m_callback{};
    std::thread* m_transferThread{};
    bool m_active{true};
    bool m_done{true};
    size_t m_dataOffs{};
    std::mutex m_checkOnly;
};
// put the scanning into a separate class
//   subclassing std::thread is discouraged...
class WorkThread
{
public:
    WorkThread(
            Glib::Dispatcher& dispatcher
            , Glib::Dispatcher& completed
            , const Glib::ustring& deviceId
            , const std::map<uint32_t, int32_t>& properties
            , TransferThread* transfer);
    virtual ~WorkThread();
    void start();
    WiaDataCallback* getDataCallback();
    bool getResult();
protected:
    void run();
private:
    Glib::Dispatcher& m_dispatcher;
    Glib::Dispatcher& m_completed;
    WiaDataCallback* m_pCallback{};
    bool m_result{false};
    Glib::ustring m_deviceId;
    std::map<uint32_t, int32_t> m_properties;
    TransferThread* m_transfer{};
    std::thread* m_workThread{};
};

#endif

class WiaScanPreview
: public ScanPreview
{
public:
    WiaScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, Glib::Dispatcher& completed);
    explicit WiaScanPreview(const WiaScanPreview& orig) = delete;
    virtual ~WiaScanPreview();

    void scan(const Glib::ustring& devId, const std::map<uint32_t, int32_t>& properties);
    bool getResult();
    void cleanup();

protected:
    void scanProgress();
    void create(int width, int height, const Gdk::Color& background);
private:
    WorkThread* m_worker{};
    TransferThread* m_transfer{};
    Glib::Dispatcher m_dispatcher;
    Glib::Dispatcher& m_completed;

};

