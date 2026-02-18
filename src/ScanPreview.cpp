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

#include "ScanPreview.hpp"
#include "NomadWin.hpp"
#include "WiaProperty.hpp"
#include "nomad_config.h"

#ifdef __WIN32__
#include "WiaScan.hpp"

WorkThread::WorkThread(Glib::Dispatcher& dispatcher
                        , Glib::Dispatcher& completed
                        , const Glib::ustring& deviceId
                        , const std::map<uint32_t, int32_t>& properties)
: m_dispatcher{dispatcher}
, m_completed{completed}
, m_deviceId{deviceId}
, m_properties{properties}
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
#endif

ScanPreview::ScanPreview(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>& builder,
        Glib::Dispatcher& completed)
: Gtk::DrawingArea(cobject)
, m_pixbuf()
, m_scaled()
, m_dispatcher()
, m_completed{completed}
{
    m_dispatcher.connect(sigc::mem_fun(*this, &ScanPreview::scanProgress));
    add_events(Gdk::EventMask::BUTTON_PRESS_MASK
        | Gdk::EventMask::BUTTON_RELEASE_MASK
        | Gdk::EventMask::POINTER_MOTION_MASK
        | Gdk::EventMask::BUTTON_MOTION_MASK);
}

 ScanPreview::~ScanPreview()
 {
     cleanup(); // remove remaining
 }

 Gdk::CursorType
 ScanPreview::getCursor(GdkEventMotion* motion_event)
 {
    const auto sensitifity = 12;
    const auto sensitifity_2 = sensitifity / 2;
    double mouseX = motion_event->x;
    double mouseY = motion_event->y;
    double x1 = convertRel2X(m_xstart);
    double y1 = convertRel2Y(m_ystart);
    double x2 = convertRel2X(m_xend);
    double y2 = convertRel2Y(m_yend);
    Gdk::CursorType cursorType = Gdk::CursorType::ARROW;
    if (mouseY >= (y1-sensitifity_2) && mouseY <= (y2+sensitifity_2)) {
        if (std::abs(mouseX - x1) < sensitifity) {
            // left
            cursorType = Gdk::CursorType::LEFT_SIDE;
        }
        if (std::abs(mouseX - x2) < sensitifity && mouseY >= y1 && mouseY <= y2) {
            // right
            cursorType = Gdk::CursorType::RIGHT_SIDE;
        }
    }
    if (mouseX >= (x1-sensitifity_2) && mouseX <= (x2+sensitifity_2)) {
        if (std::abs(mouseY - y1) < sensitifity) {
            // top
            cursorType = Gdk::CursorType::TOP_SIDE;
        }
        if (std::abs(mouseY - y2) < sensitifity) {
            // bottom
            cursorType = Gdk::CursorType::BOTTOM_SIDE;
        }
    }
    return cursorType;
}

bool
ScanPreview::on_button_release_event(GdkEventButton* event)
{
    auto gdkWindow = get_window();
    if (m_changedCursor) {
        gdkWindow->set_cursor();
        m_changedCursor = false;
    }
    return FALSE;
}

bool
ScanPreview::on_motion_notify_event(GdkEventMotion* motion_event)
{
    bool btn1 = (motion_event->state  & Gdk::ModifierType::BUTTON1_MASK) != 0x0;
    Gdk::CursorType cursorType = getCursor(motion_event);
    if (m_showMask) {
        auto gdkWindow = get_window();
        if (cursorType != Gdk::CursorType::ARROW) {
            Glib::RefPtr<Gdk::Cursor> display_cursor = Gdk::Cursor::create(gdkWindow->get_display(), cursorType);
            gdkWindow->set_cursor(display_cursor);
            m_changedCursor = true;
        }
        else {
            gdkWindow->set_cursor();
            m_changedCursor = false;
        }
    }
    if (btn1 && m_scaled) {
        bool redraw = false;
        double mouseX = motion_event->x;
        double mouseY = motion_event->y;
        double displayedWidth = m_scaled->get_width();
        double displayedHeight = m_scaled->get_height();
        if (cursorType == Gdk::CursorType::LEFT_SIDE) {
            m_xstart = mouseX / displayedWidth;
            redraw = true;
        }
        else if (cursorType == Gdk::CursorType::RIGHT_SIDE) {
            m_xend = mouseX / displayedWidth;
            redraw = true;
        }
        else if (cursorType == Gdk::CursorType::TOP_SIDE) {
            m_ystart = mouseY / displayedHeight;
            redraw = true;
        }
        else if (cursorType ==  Gdk::CursorType::BOTTOM_SIDE) {
            m_yend = mouseY / displayedHeight;
            redraw = true;
        }
        if (redraw) {
            queue_draw();
        }
        return TRUE;
    }
    return FALSE;
}

void
ScanPreview::setShowMask(bool showMask)
{
    m_showMask = showMask;
}

bool ScanPreview::getShowMask()
{
    return m_showMask;
}

bool
ScanPreview::getResult()
{
    return m_worker ? m_worker->getResult() : true;
}

Glib::RefPtr<Gdk::Pixbuf>
ScanPreview::getPixbuf()
{
    return m_pixbuf;
}

double
ScanPreview::convertRel2X(double relx)
{
    double x = 0.0;
    if (m_scaled) {
        int displayedWidth = m_scaled->get_width();
        x = relx * displayedWidth;
    }
    return x;
}

double
ScanPreview::convertRel2Y(double rely)
{
    double y = 0.0;
    if (m_scaled) {
        int displayedHeight = m_scaled->get_height();
        y = rely * displayedHeight;
    }
    return y;
}

bool
ScanPreview::on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx)
{
    if (m_pixbuf) {
        double wScale = static_cast<double>(get_width()) / static_cast<double>(m_pixbuf->get_width());
        double hScale = static_cast<double>(get_height()) / static_cast<double>(m_pixbuf->get_height());
        m_scale = std::min(wScale, hScale);
        int scaledWidth = static_cast<int>(static_cast<double>(m_pixbuf->get_width()) * m_scale);
        int scaledHeight = static_cast<int>(static_cast<double>(m_pixbuf->get_height()) * m_scale);
        if (!m_scaled
         || (std::abs(scaledWidth - m_scaled->get_width()) > 10
         &&  std::abs(scaledHeight - m_scaled->get_height()) > 10)) {   // scale with steps not every pixel
            //std::cout << "scaling "
            //          << " width " << scaledWidth
            //          << " height " << scaledHeight << std::endl;
            m_scaled = m_pixbuf->scale_simple(scaledWidth, scaledHeight, Gdk::InterpType::INTERP_BILINEAR);
        }
        int displayedWidth = m_scaled->get_width();
        int displayedHeight = m_scaled->get_height();
        Gdk::Cairo::set_source_pixbuf(cairoCtx, m_scaled, 0, 0);
        cairoCtx->rectangle(0, 0, displayedWidth, displayedHeight);
        cairoCtx->fill();
        if (m_showMask) {
            // draw mask
            cairoCtx->set_fill_rule(Cairo::FillRule::FILL_RULE_EVEN_ODD);
            cairoCtx->rectangle(0, 0, displayedWidth, displayedHeight);
            double x = convertRel2X(m_xstart);
            double y = convertRel2Y(m_ystart);
            double width = convertRel2X(m_xend - m_xstart);
            double height = convertRel2Y(m_yend - m_ystart);
            cairoCtx->rectangle(x, y, width, height);
            cairoCtx->set_source_rgba(0.5, 0.5, 0.5, 0.5);
            cairoCtx->fill();
        }
    }
    return true;
}

void
ScanPreview::create(int width, int height, const Gdk::Color& background)
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
ScanPreview::transferRow(uint8_t* pixSrc, uint32_t yPos, uint32_t transferPixels)
{
    auto pixData = reinterpret_cast<uint32_t*>(m_pixbuf->get_pixels()) + (yPos) * (m_pixbuf->get_rowstride() / sizeof(uint32_t));
    for (size_t x = 0; x < transferPixels; ++x) {
        //   windows   BGR?
        uint32_t rgb;
        const auto callback = m_worker->getDataCallback();
        const auto bytePerPixel = callback->getBytePerPixel();
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
ScanPreview::scanProgress()
{
    if (m_worker) {
        const auto callback = m_worker->getDataCallback();
        if (callback->getWidth() == 0
         || callback->getHeight() == 0) {   // not yet ready
            std::cout << "ScanPreview::scanProgress no data " << std::endl;
            return;
        }
        if (!m_pixbuf) {
            Gdk::Color color;
            color.set("#000");
            create(callback->getWidth(), callback->getHeight(), color);
        }
        auto& queue = callback->getQueue();
        std::shared_ptr<WiaData> item;
        while (queue.try_pop(item)) {
            const auto srcBytesPerRow = callback->getBytePerRow();
            const auto srcBytePerPixel = callback->getBytePerPixel();
            uint32_t yPos = m_dataOffs / srcBytesPerRow;
            uint8_t* pixSrc = item->getData();            
            if (m_reaminder) {  // as the packets don't care of row or pixel border we have to connect those pieces
                uint32_t xBytes = m_reaminder->getUsed();
                std::cout << "ScanPreview::scanProgress"
                           << " dataOffs " << m_dataOffs
                           << " used " << m_reaminder->getUsed() 
                           << " ypos " << yPos << std::endl;
                auto remData = m_reaminder->getData();
                std::copy(pixSrc, pixSrc + xBytes, remData + xBytes);
                transferRow(pixSrc, yPos, callback->getWidth());
                pixSrc += xBytes;
                ++yPos;
                m_reaminder.reset();
            }                        
            const uint32_t srcDataSize = item->getBytesSize();
            uint32_t pixCnt = srcDataSize / srcBytePerPixel;
            std::cout << "ScanPreview::scanProgress"
                      << " item size " << srcDataSize
                      << " ypos " << yPos << std::endl;
            while (pixCnt >= callback->getWidth()) {
                //std::cout << "ScanPreview::scanProgress"
                //          << " y " << y + yPos
                //          << " pixcnt " << pixCnt
                //          << " transf " << transferPixels << std::endl;
                transferRow(pixSrc, yPos, callback->getWidth());
                pixCnt -= callback->getWidth();
                pixSrc += srcBytesPerRow;
                ++yPos;
                if (yPos >= callback->getHeight()) {
                    std::cout << "Got ypos " << yPos << " break!" << std::endl;
                    pixCnt = 0;
                    break;
                }
            }
            std::cout << "ScanPreview::scanProgress"
                      << " pixCnt " << pixCnt << std::endl;
            if (pixCnt > 0) {
                size_t remBytes = pixCnt * srcBytePerPixel;
                m_reaminder = std::make_shared<WiaData>(srcBytesPerRow);
                uint8_t* remData = m_reaminder->getData();
                std::copy(pixSrc, pixSrc + remBytes, remData);
                m_reaminder->setUsed(remBytes);
            }
            m_dataOffs += srcDataSize;
        }
        callback->resetPending();
        m_scaled.reset(); // need to refresh
//                //queue_draw_area(0, m_RowLast, bi->biWidth, uptoRow - m_RowLast);
        queue_draw();   // as the view is scaled no direct relationship...
//                m_RowLast = uptoRow;
    }
    else {
        std::cout << "missing callback!" << std::endl;
    }
}

void
ScanPreview::cleanup()
{
    // use member so we wont fail on destruction of thread and can keep transfer
    if (m_workThread) {
        if (m_workThread->joinable()) {
            std::cout << "Joining ..." << std::endl;
            m_workThread->join();   // wait for previous
        }
        std::cout << "Destroy prev" << std::endl;
        delete m_workThread;
        m_workThread = nullptr;
    }
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
    m_dataOffs = 0l;
}

void
ScanPreview::scan(const Glib::ustring& devId, const std::map<uint32_t, int32_t>& properties)
{
    cleanup();
    if (m_workThread == nullptr) {
        m_initScan = true;
        m_worker = new WorkThread(m_dispatcher, m_completed, devId, properties);
        //m_workThread = ThreadWrapper ([] (WorkThread* worker) {
        //    worker->run();
        //}, m_worker);
        m_workThread = new std::thread(
            [this] {
            m_worker->run();
        });
    }
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

bool
ScanPreview::savePng(const Glib::ustring& file)
{
    bool ret = false;
    if (m_pixbuf) {
        m_pixbuf->save(file, "png");
        ret = true;
        //}
        //else if (m_bytePerPixel == 1){
        //    ImageUtils::grayscalePng(m_pixbuf, file);
        //    ret = true;
        //}
        //else {
        //    ImageUtils::blackandwhitePng(m_pixbuf, file);
        //    ret = true;
        //}
    }
    return ret;
}
