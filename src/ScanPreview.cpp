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

#include <iostream>
#include <gdkmm-3.0/gdkmm/pixbuf.h>
#include <gdkmm-3.0/gdkmm/rectangle.h>


#include "ScanPreview.hpp"
#include "NomadWin.hpp"
#include "WiaProperty.hpp"

#ifdef __WIN32__
#include "WiaScan.hpp"

WorkThread::WorkThread(Glib::Dispatcher& dispatcher
                        , const Glib::ustring& deviceId
                        , const std::map<uint32_t, WiaValue>& properties)
: m_dispatcher{dispatcher}
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
            m_pCallback  = new WiaDataCallback(m_dispatcher);
            if (m_pCallback) {
                m_result = dev->scan(m_pCallback, m_properties);
                std::cout << "scan " << (m_result ? "ok" : "err") << std::endl;    // will show error on color scan ?
                m_completed= true;
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
}

WiaDataCallback*
WorkThread::getDataCallback()
{
    return m_pCallback;
}

WorkThread::~WorkThread()
{
    if (m_pCallback) {
        m_pCallback->Release();
        m_pCallback = nullptr;
    }

}
#endif

ScanPreview::ScanPreview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::DrawingArea(cobject)
, m_pixbuf()
, m_scaled()
, m_dispatcher()
{
    m_dispatcher.connect(sigc::mem_fun(*this, &ScanPreview::scanProgress));
    add_events(Gdk::EventMask::BUTTON_PRESS_MASK
        /*| Gdk::EventMask::BUTTON_RELEASE_MASK */
        | Gdk::EventMask::BUTTON_MOTION_MASK);
}

 ScanPreview::~ScanPreview()
 {
     cleanup(); // remove remaining
 }

bool
ScanPreview::on_motion_notify_event(GdkEventMotion* motion_event)
{
    bool btn1 = (motion_event->state  & Gdk::ModifierType::BUTTON1_MASK) != 0x0;
    if (btn1 && m_scaled) {
        const auto sensitifity = 12;
        double mouseX = motion_event->x;
        double mouseY = motion_event->y;
        double x1 = convertRel2X(m_xstart);
        double y1 = convertRel2Y(m_ystart);
        double x2 = convertRel2X(m_xend);
        double y2 = convertRel2Y(m_yend);
        double displayedWidth = m_scaled->get_width();
        double displayedHeight = m_scaled->get_height();
        bool redraw = false;
        if (mouseY >= y1 && mouseY <= y2) {
            if (std::abs(mouseX - x1) < sensitifity) {
                // left
                m_xstart = mouseX / displayedWidth; // std::min(, x2 - 4.0);
                redraw = true;
            }
            if (std::abs(mouseX - x2) < sensitifity && mouseY >= y1 && mouseY <= y2) {
                // right
                m_xend = mouseX / displayedWidth; //std::max(, 4.0);
                redraw = true;
            }
        }
        if (mouseX >= x1 && mouseX <= x2) {
            if (std::abs(mouseY - y1) < sensitifity) {
                // top
                m_ystart = mouseY / displayedHeight; // std::min(, y2 - 4.0);
                redraw = true;
            }
            if (std::abs(mouseY - y2) < sensitifity) {
                // bottom
                m_yend = mouseY / displayedHeight; //std::max(, 4.0);
                redraw = true;
            }
        }
        if (redraw) {
            queue_draw();
        }
        return TRUE;
    }
    return FALSE;
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
    return true;
}

void
ScanPreview::create(std::array<int,2> size, const Gdk::Color& background)
{
    m_pixbuf = Gdk::Pixbuf::create(
            Gdk::Colorspace::COLORSPACE_RGB
            , true
            , 8
            , size[0]
            , size[1]);
    guint32 pixel = (background.get_red() >> 8u) << 24u
                  | (background.get_green() >> 8u) << 16u
                  | (background.get_blue())      // eliminated >> 8u) << 8u as this will be a no op
                  | 0xffu;
    m_pixbuf->fill(pixel);
    m_scaled.reset();
    queue_draw();
}
void
ScanPreview::scanProgress()
{
    if (m_worker) {
        uint32_t transferedSize = 0;
        int32_t transferedPercent = 0;
        uint32_t transfereAlloc = 0;
        uint8_t* p = m_worker->getDataCallback()->getDataTransfered(&transferedSize, &transferedPercent, &transfereAlloc);

        std::cout  << "percent " << transferedPercent
                   << " size " << transferedSize
                   << std::endl;
        if (p && transferedSize >= sizeof(BITMAPINFOHEADER)) {
            auto bi = reinterpret_cast<BITMAPINFOHEADER*>(p);
            if (m_initScan) {
                if (bi->biWidth >= 0) {
                    std::array<int,2> size {std::abs(bi->biWidth),std::abs(bi->biHeight)};
                    Gdk::Color color;
                    color.set("#000");
                    create(size, color);
                    m_initScan = false;
                    m_RowLast = 0;
                    std::cout << "Got scan size "
                          << bi->biWidth << " " << bi->biHeight
                          << " bits " << bi->biBitCount
                          << " compress " << std::hex << bi->biCompression << std::dec
                          << " rowstride " << m_pixbuf->get_rowstride()
                          << std::endl;
                }
                else {
                    //std::cout << dump(p, 64) << std::endl;
                    std::cout << "Bitmap wrong info header " << std::endl;
                }
            }
            else {
                // we get width 2550 height -3501 bits 24 compress 0
                // the last packet will be offs 26629000 len 160692
                //   the first 40+bytes will be bitmap-header
                //   the byteStrideSize should be 7652 = (2550 * 3) round up to 4
                //   so 26789652 / 7652 will give use the expected 3501
                int32_t height = std::abs(bi->biHeight);
                bool bmpInverseHeight = bi->biHeight < 0;
                int32_t width = std::abs(bi->biWidth);
                uint32_t byteRowStride = ((width * bi->biBitCount + 31) / 32) * 4;
                uint32_t headerSize = (transfereAlloc) - (byteRowStride * height - 1); // the size of header e.g. for gray is larger than just sizeof(BITMAPINFOHEADER)
                uint8_t* bmpData = p + headerSize;
                uint32_t* pixData = reinterpret_cast<uint32_t*>(m_pixbuf->get_pixels());
                uint32_t bytePerPixel = bi->biBitCount / 8;
                int32_t uptoRow = std::min(static_cast<int32_t>((transferedSize - headerSize) / byteRowStride), height);
                std::cout << "uptoRow " << uptoRow
                          << " byte stride " << byteRowStride << std::endl;
                for (int32_t y = m_RowLast; y <= uptoRow; ++y) {
                    int32_t yd = y;
                    if (!bmpInverseHeight) {
                        yd = (height-1) - y;
                    }
                    auto rows = bmpData + (y * byteRowStride);
                    auto rowd = pixData + (yd * m_pixbuf->get_rowstride() / 4);
                    //std::cout << "row " << y << std::endl;
                    for (int32_t x = 0; x < width; ++x) {
                        //  windows   BGR?
                        uint32_t rgb;
                        if (bytePerPixel >= 3) {       // discard alpha in case it is there...
                            rgb = (rows[0] << 16u) | (rows[1] << 8u) | rows[2];
                        }
                        else if (bytePerPixel == 1)  {  // grayscale
                            auto gray = *rows;
                            rgb = (gray << 16u) | (gray << 8u) | gray;
                        }
                        else {  // b&w
                            auto bit = rows[x >> 3u] & (0x80u >> (x & 0x7u));
                            uint8_t gray = bit != 0 ? 0xffu : 0u;
                            rgb = (gray << 16u) | (gray << 8u) | gray;
                        }
                        //  pixbuf    A 31..24  R 23..16  G 15..8   B 7..0
                        *rowd = 0xff000000u | rgb;
                        ++rowd;
                        rows += bytePerPixel;
                    }
                }
                m_scaled.reset(); // need to refresh
                //queue_draw_area(0, m_RowLast, bi->biWidth, uptoRow - m_RowLast);
                queue_draw();   // as the view is scaled no direct relationship...
                m_RowLast = uptoRow;
            }
        }
    }
    else {
        std::cout << "missing callback!" << std::endl;
    }
}

void
ScanPreview::cleanup()
{
    // use member so we wont fail on destruction of thread and can keep transfer
    if (m_workThread != nullptr) {
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
}

void
ScanPreview::scan(const Glib::ustring& devId, const std::map<uint32_t, WiaValue>& properties)
{
    cleanup();
    if (m_workThread == nullptr) {
        m_initScan = true;
        m_worker = new WorkThread(m_dispatcher, devId, properties);
        //m_workThread = ThreadWrapper ([] (WorkThread* worker) {
        //    worker->run();
        //}, m_worker);
        m_workThread = new std::thread(
            [this] {
            m_worker->run();
        });
    }
}

bool
ScanPreview::saveImage(const Glib::ustring& file)
{
    if (m_pixbuf) {
        m_pixbuf->save(file, "png");
        return true;
    }
    return false;
}