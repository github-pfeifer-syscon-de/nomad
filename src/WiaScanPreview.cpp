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

#include <iostream>
#include <StringUtils.hpp>
#include <ImageUtils.hpp>
#include <mutex>
#include <thread>
#include <chrono>

#include "WiaScanPreview.hpp"
#include "NomadWin.hpp"
#include "WiaProperty.hpp"
#include "nomad_config.h"

#ifdef __WIN32__
#include "WiaScan.hpp"

WorkThread::WorkThread(Glib::Dispatcher& dispatcher
                        , Glib::Dispatcher& completed
                        , const Glib::ustring& deviceId
                        , const std::map<uint32_t, int32_t>& properties
                        , TransferThread* transferThread)
: m_dispatcher{dispatcher}
, m_completed{completed}
, m_deviceId{deviceId}
, m_properties{properties}
, m_transfer{transferThread}
{
}

void
WorkThread::run()
{
    WiaScan winScan;
    auto devs = winScan.getDevices();
    bool found = false;
    for (auto dev : devs) {
        if (dev->getDeviceId() == m_deviceId) {
            found = true;
            m_pCallback = new WiaDataCallback(m_dispatcher);
            if (m_pCallback) {
                if (m_transfer) {
                    m_transfer->setCallback(m_pCallback);
                }
                m_result = dev->scan(m_pCallback, m_properties);
                std::cout << "scan " << (m_result ? "ok" : "err") << std::endl;    // will show error on color scan ?
                m_dispatcher.emit();    // ensure we completed (other places with hight rate of activations may loose signals?)
            }
            else {
                std::cout << "Error creating callback" << std::endl;
            }
            break;
        }
    }
    if (!found) {
        std::cout << "The expected device " << m_deviceId << " was not found!" << std::endl;
    }
    m_completed.emit();
}

WorkThread::~WorkThread()
{
    if (m_pCallback) {
        m_pCallback->Release();
        m_pCallback = nullptr;
    }
    if (m_workThread) {
        if (m_workThread->joinable()) {
            std::cout << "WorkThread::~WorkThread joining ..." << std::endl;
            m_workThread->join();   // wait for previous
        }        
        std::cout << "WorkThread::~WorkThread  destroy" << std::endl;
        delete m_workThread;
        m_workThread = nullptr;
    }   
}

void
WorkThread::start()
{
    m_workThread = new std::thread(
        [this] {
           run();
        });    
}

WiaDataCallback*
WorkThread::getDataCallback()
{
    return m_pCallback;
}

bool WorkThread::getResult()
{
    return m_result;
}

TransferThread::TransferThread()
{
}

TransferThread::~TransferThread()
{
    std::cout << "TransferThread::~TransferThread destroy" << std::endl;
    if (m_transferThread) {
        if (m_transferThread->joinable()) {
            std::cout << "TransferThread::~TransferThread joining ..." << std::endl;
            m_transferThread->join();   // wait for
        }
        std::cout << "TransferThread::~TransferThread destroy" << std::endl;
        delete m_transferThread;
        m_transferThread = nullptr;        
    }
}

void 
TransferThread::checkStart()
{
    // since we are waiting for input from two sides ensure everything is ready before we start
    std::lock_guard<std::mutex> guard(m_checkOnly);
    if (m_pixbuf
     && m_callback 
     && !m_transferThread) {
        std::cout << "TransferThread::checkStart started" << std::endl;
        m_transferThread = new std::thread(
            [this] {
                run();
            });
    }    
}

void
TransferThread::transferRow(uint8_t* pixSrc, uint32_t yPos, uint32_t transferPixels)
{
    auto pixData = reinterpret_cast<uint32_t*>(m_pixbuf->get_pixels()) + (yPos) * (m_pixbuf->get_rowstride() / sizeof(uint32_t));
    for (size_t x = 0; x < transferPixels; ++x) {
        //   windows   BGR?
        uint32_t rgb;
        const auto bytePerPixel = m_callback->getBytePerPixel();
        if (bytePerPixel >= 3) {       // discard alpha in case it is there...
            rgb = (pixSrc[0] << 16u) | (pixSrc[1] << 8u) | pixSrc[2];
        }
        else {
            uint8_t gray;
            if (bytePerPixel == 1)  {   // grayscale
                gray = *pixSrc;
            }
            else {                      // b&w
                auto bit = pixSrc[x >> 3u] & (0x80u >> (x & 0x7u));
                gray = bit != 0 ? 0xffu : 0u;
            }
            rgb = (gray << 16u) | (gray << 8u) | gray;
        }
        //  pixbuf    A 31..24  R 23..16  G 15..8   B 7..0
        *pixData = 0xff000000u | rgb;
        pixSrc += bytePerPixel;
        ++pixData; // next pixel
    }
}

void 
TransferThread::handleItem(const std::shared_ptr<WiaData>& item)
{
    const auto srcBytesPerRow = m_callback->getBytePerRow();
    const auto srcBytePerPixel = m_callback->getBytePerPixel();
    uint32_t yPos = m_dataOffs / srcBytesPerRow;
    uint8_t* pixSrc = item->getData();            
    const uint32_t srcDataSize = item->getBytesSize();
    uint32_t pixCnt = srcDataSize / srcBytePerPixel;
    // since we have a offset after the first "packet", try to use upto "end"
    auto offs = (srcDataSize % srcBytesPerRow);
    std::cout << "TransferThread::handleItem"
              << " item size " << srcDataSize
              << " ypos " << yPos
              << " offs " << offs << std::endl;
    pixSrc += offs;
    pixCnt -= offs / srcBytePerPixel;
    while (pixCnt >= m_callback->getWidth()) {
        //std::cout << "WiaScanPreview::scanProgress"
        //          << " y " << y + yPos
        //          << " pixcnt " << pixCnt
        //          << " transf " << transferPixels << std::endl;
        transferRow(pixSrc, yPos, m_callback->getWidth());
        pixCnt -= m_callback->getWidth();
        pixSrc += srcBytesPerRow;
        ++yPos;
        if (yPos >= m_callback->getHeight()) {
            std::cout << "TransferThread::handleItem got ypos " << yPos << " break!" << std::endl;
            pixCnt = 0;
            m_active = false;   // since we are done ...
            break;
        }
    }
    std::cout << "TransferThread::handleItem"
              << " pixCnt " << pixCnt << std::endl;
    m_dataOffs += srcDataSize;    
}

void
TransferThread::run()
{
    m_done = false;
    while (m_active) {
        auto& queue = m_callback->getQueue();
        std::shared_ptr<WiaData> item;
        while (queue.try_pop(item)) {
            handleItem(item);
            if (!m_active) {
                break;
            }
        }       
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    m_done = true;
}

void 
TransferThread::setActive(bool active)
{
    m_active = active;
}

bool 
TransferThread::isActive()
{
    return m_active;
}

bool 
TransferThread::isDone()
{
    return m_done;
}

void
TransferThread::setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
    m_pixbuf = pixbuf;
    checkStart();
}

void 
TransferThread::setCallback(WiaDataCallback* callback)
{
    m_callback = callback; 
    checkStart();
}

#endif

WiaScanPreview::WiaScanPreview(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>& builder,
        Glib::Dispatcher& completed)
: ScanPreview(cobject, builder)
, m_completed{completed}
{
    m_dispatcher.connect(sigc::mem_fun(*this, &WiaScanPreview::scanProgress));
}

 WiaScanPreview::~WiaScanPreview()
{
    cleanup(); // remove remaining
}

bool
WiaScanPreview::getResult()
{
    return m_worker ? m_worker->getResult() : true;
}

void
WiaScanPreview::create(int width, int height, const Gdk::Color& background)
{
    m_pixbuf = Gdk::Pixbuf::create(
            Gdk::Colorspace::COLORSPACE_RGB
            , true
            , 8
            , width
            , height);
    guint32 pixel = (background.get_red() >> 8u) << 24u
                  | (background.get_green() >> 8u) << 16u
                  | (background.get_blue())      // eliminated >> 8u) << 8u as this will be a no op
                  | 0xffu;
    m_pixbuf->fill(pixel);
    m_scaled.reset();
    queue_draw();
}

void
WiaScanPreview::scanProgress()
{
    if (m_worker) {
        const auto callback = m_worker->getDataCallback();
        if (callback->getWidth() == 0
         || callback->getHeight() == 0) {   // not yet ready
            std::cout << "WiaScanPreview::scanProgress no data " << std::endl;
            return;
        }
        if (!m_pixbuf) {
            Gdk::Color color;
            color.set("#000");
            create(callback->getWidth(), callback->getHeight(), color);
            if (m_transfer) {
                m_transfer->setPixbuf(m_pixbuf);
            }
            else {
                std::cout << "WiaScanPreview::scanProgress no transfer" << std::endl;
            }                 
        }
        callback->resetPending();
        m_scaled.reset(); // need to refresh
        queue_draw();   // as the view is scaled no direct relationship...
    }
    else {
        std::cout << "missing callback!" << std::endl;
    }
}

void
WiaScanPreview::cleanup()
{
    // use member so we wont fail on destruction of thread and can keep transfer
    if (m_transfer) {   // destroy in reverse order
        m_transfer->setActive(false);
        auto start = std::chrono::high_resolution_clock::now();
        while (!m_transfer->isDone()) {
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = duration_cast<std::chrono::milliseconds>(stop - start);
            if (duration.count() > 1000l) { // wait a definite time
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        delete m_transfer;
        m_transfer = nullptr;
    }
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;        
    }
}

void
WiaScanPreview::scan(const Glib::ustring& devId, const std::map<uint32_t, int32_t>& properties)
{
    cleanup();
    m_pixbuf.reset();   // recreeate for each scan
    m_transfer = new TransferThread();
    m_worker = new WorkThread(m_dispatcher, m_completed, devId, properties, m_transfer);
    m_worker->start();
}

// convert alpha to grayscales if necessary
//   looks nice to use something more adaptable,
//   but the outcome looks blured ...
//Cairo::RefPtr<Cairo::Surface>
//convertSurface(Cairo::RefPtr<Cairo::ImageSurface>& surface)
//{
//    if (surface->get_format() == Cairo::Format::FORMAT_A1
//     || surface->get_format() == Cairo::Format::FORMAT_A8) {
//        auto cr = Cairo::Context::create(surface);
//        cr->push_group_with_content(Cairo::Content::CONTENT_COLOR_ALPHA);
//        cr->set_source_rgb(1.0, 1.0, 1.0);
//        cr->paint();
//        cr->set_source_rgb(0.0, 0.0, 0.0);
//        cr->mask(cr->get_target(), 0.0, 0.0);
//        return cr->get_group_target();
//    }
//    return surface;
//}

