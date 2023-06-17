/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * Copyright (C) 2023 RPf <gpl3@pfeifer-syscon.de>
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

#include <cmath>
#include <iostream>
#include <istream>

#include "Preview.hpp"
#include "SvgShape.hpp"
#include "NomadWin.hpp"
#include "TextShape.hpp"

#ifdef __WIN32__
#include "WiaScan.hpp"

WorkThread::WorkThread(Glib::Dispatcher& dispatcher)
: m_dispatcher{dispatcher}
{
}

void
WorkThread::run()
{
    WiaScan winScan;
    auto devs = winScan.getDevices();
    if (!devs.empty()) {
        m_pCallback  = new WiaDataCallback(m_dispatcher);
        if (m_pCallback) {
            auto dev = devs[0];
            bool r = dev->scan(m_pCallback);
            std::cout << "scan " << (r ? "ok" : "err") << std::endl;
        }
        else {
            std::cout << "Error creating callback" << std::endl;
        }
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

Preview::Preview(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, NomadWin* nomadWin)
: Gtk::DrawingArea(cobject)
, m_nomadWin{nomadWin}
, m_pixbuf{}
, m_scaled{}
, m_dispatcher{}
{
    add_events(Gdk::EventMask::BUTTON_PRESS_MASK
            | Gdk::EventMask::BUTTON_RELEASE_MASK
            | Gdk::EventMask::BUTTON_MOTION_MASK);

    m_dispatcher.connect(sigc::mem_fun(*this, &Preview::scanProgress));
    std::array<int,2> size {800,600};
    Gdk::Color color;
    color.set("#000");
    create(size, color);

    // Keep some infos on foreground
    WiaScan winScan;
    auto devs = winScan.getDevices();
    std::cout << "Devs " << devs.size() << std::endl;
    if (!devs.empty()) {
        auto dev = devs[0];
        dev->getProperties();
    }
}

void
Preview::loadSvg(const Glib::RefPtr<Gio::File>& f)
{
    auto svg = std::make_shared<SvgShape>();
    svg->from_file(f);
    svg->setScale(0.1);
    add(svg);
}

void
Preview::add(const std::shared_ptr<Shape>& shape)
{
    m_shapes.push_back(shape);
    queue_draw();
}

void
Preview::addText(const TextInfo& text)
{
    auto textShape = std::make_shared<TextShape>();
    textShape->setText(text);
    add(textShape);
}

void
Preview::create(std::array<int,2> size, const Gdk::Color& background)
{
    m_pixbuf = Gdk::Pixbuf::create(
            Gdk::Colorspace::COLORSPACE_RGB
            , false
            , 8
            , size[0]
            , size[1]);
    guint32 pixel = (background.get_red() >> 8u) << 24u
                  | (background.get_green() >> 8u) << 16u
                  | (background.get_blue())      // eliminated >> 8u) << 8u as this will be a no op
                  | 0xffu;
    m_pixbuf->fill(pixel);
    m_shapes.clear();
    m_selected.reset();
    queue_draw();
}

void
Preview::loadImage(const Glib::RefPtr<Gio::File>& f)
{
    m_pixbuf = Gdk::Pixbuf::create_from_file(f->get_path());
    m_shapes.clear();
    m_selected.reset();
    queue_draw();
}

bool
Preview::on_motion_notify_event(GdkEventMotion* motion_event)
{
    bool btn1 = (motion_event->state  & Gdk::ModifierType::BUTTON1_MASK) != 0x0;
    if (btn1) {
        if (m_selected && m_scaled) {
            Gdk::Rectangle old = m_selected->getBounds(m_scaled->get_width(), m_scaled->get_height());
            double relX = (motion_event->x - m_relX) / static_cast<double>(m_scaled->get_width());
            double relY = (motion_event->y - m_relY) / static_cast<double>(m_scaled->get_height());
            m_selected->setRelPosition(relX, relY);
            Gdk::Rectangle next = m_selected->getBounds(m_scaled->get_width(), m_scaled->get_height());
            next.join(old);
            queue_draw_area(
                    std::max(next.get_x()-20, 0),
                    std::max(next.get_y()-20, 0),
                    next.get_width() + 40,
                    next.get_height() + 40);   // draw only required
            return TRUE;
        }
    }
    return FALSE;
}

bool
Preview::on_button_release_event(GdkEventButton* event)
{
//    bool btn1 = (event->button == 1);
//    if (btn1 && m_selected) {
//        return TRUE;
//    }
    return FALSE;
}

bool
Preview::on_button_press_event(GdkEventButton* event)
{
    bool btn1 = (event->button == 1);
    if (btn1 && m_scaled) {
        m_selected.reset();
        double mouseX = event->x;
        double mouseY = event->y;
        for (auto shape : m_shapes) {
            auto r =shape->getBounds(m_scaled->get_width(), m_scaled->get_height());
            if (mouseX >= r.get_x()
             && mouseX < r.get_x() + r.get_width()
             && mouseY >= r.get_y()
             && mouseY < r.get_y() + r.get_height()) {
                m_relX = (mouseX - r.get_x());
                m_relY = (mouseY - r.get_y());
                m_selected = shape;
                break;
            }
        }
    }
    //g_warning("button: %dx%d btn:0x%04x", event->x, event->y, event->button);
    return TRUE;
}

void
Preview::setPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& pixbuf)
{
    m_pixbuf = pixbuf;
    m_scaled.reset();
    queue_draw();
}

Glib::RefPtr<Gdk::Pixbuf>
Preview::getPixbuf()
{
    return m_pixbuf;
}

void
Preview::render(const Cairo::RefPtr<Cairo::Context>& cairoCtx,
        const Glib::RefPtr<Gdk::Pixbuf> pixbuf)
{
    Gdk::Cairo::set_source_pixbuf(cairoCtx, pixbuf, 0, 0);
    cairoCtx->rectangle(0, 0, pixbuf->get_width(), pixbuf->get_height());
    cairoCtx->fill();
    for (auto shape : m_shapes) {
        shape->render(cairoCtx, pixbuf->get_width(), pixbuf->get_height());
    }
}

bool
Preview::saveImage(const Glib::ustring& file)
{
    if (m_pixbuf) {
        Cairo::RefPtr<Cairo::ImageSurface> outpixmap = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32,
            m_pixbuf->get_width()
            , m_pixbuf->get_height());
        {
            Cairo::RefPtr<Cairo::Context> cairoCtx = Cairo::Context::create(outpixmap);
            render(cairoCtx, m_pixbuf);
        }
        outpixmap->write_to_png(file);
        return true;
    }
    return false;
}

bool
Preview::on_draw(const Cairo::RefPtr<Cairo::Context>& cairoCtx)
{
    if (m_pixbuf) {
        double wScale = static_cast<double>(get_width()) / static_cast<double>(m_pixbuf->get_width());
        double hScale = static_cast<double>(get_height()) / static_cast<double>(m_pixbuf->get_height());
        double scale = std::min(wScale, hScale);
        int scaledWidth = static_cast<int>(static_cast<double>(m_pixbuf->get_width()) * scale);
        int scaledHeight = static_cast<int>(static_cast<double>(m_pixbuf->get_height()) * scale);
        if (!m_scaled
         || (std::abs(scaledWidth - m_scaled->get_width()) > 10
         &&  std::abs(scaledHeight - m_scaled->get_height()) > 10)) {   // scale with steps not every pixel
            //std::cout << "scaling "
            //          << " width " << scaledWidth
            //          << " height " << scaledHeight << std::endl;
            m_scaled = m_pixbuf->scale_simple(scaledWidth, scaledHeight, Gdk::InterpType::INTERP_BILINEAR);
        }
        render(cairoCtx, m_scaled);
    }
    return true;
}


void
Preview::scanProgress()
{
    if (m_worker) {
        uint32_t size = 0;
        int32_t percent = 0;
        uint8_t* p = m_worker->getDataCallback()->getDataTransfered(&size, &percent);

        std::cout  << "percent " << percent
                   << " size " << size
                   << std::endl;
        if (p && size >= sizeof(BITMAPINFOHEADER)) {
            auto bi = reinterpret_cast<BITMAPINFOHEADER*>(p);
            if (m_initScan) {
                std::cout << "Got scan size "
                          << bi->biWidth << " " << bi->biHeight
                          << " bits " << bi->biBitCount
                          << " compress " << std::hex << bi->biCompression << std::dec
                          << std::endl;
                if (bi->biWidth >= 0) {
                    std::array<int,2> size {std::abs(bi->biWidth),std::abs(bi->biHeight)};
                    Gdk::Color color;
                    color.set("#000");
                    create(size, color);
                    m_initScan = false;
                    m_RowLast = 0;
                }
                else {
                    //std::cout << dump(p, 64) << std::endl;
                    std::cout << "Bitmap wrong info header " << std::endl;
                }
            }
            else {
                // we get width 2550 height -3501 bits 24 compress 0
                // the last packet will be offs 26629000 len 160692
                //   the first 40bytes will be bitmap-header
                //   the byteStrideSize should be 7652 = (2550 * 3) round up to 4
                //   so 26789652 / 7652 will give use the expected 3501
                int32_t height = std::abs(bi->biHeight);
                bool bmpInverseHeight = bi->biHeight < 0;
                int32_t width = std::abs(bi->biWidth);
                uint8_t* bmpData = p + bi->biSize;
                uint8_t* pixData = m_pixbuf->get_pixels();
                uint32_t bytePerPixel = bi->biBitCount / 8;
                uint32_t wordRowStride = ((width * bi->biBitCount + 31) / 32);
                uint32_t byteRowStride = wordRowStride * 4;
                int32_t uptoRow = std::min(static_cast<int32_t>((size - bi->biSize) / byteRowStride), height);
                std::cout << "uptoRow " << uptoRow << " byte stride " << byteRowStride << std::endl;
                for (int32_t y = m_RowLast; y < uptoRow; ++y) {
                    int32_t yd = y;
                    if (!bmpInverseHeight) {
                        yd = (height-1) - y;
                    }
                    //std::cout << "row " << y << std::endl;
                    uint8_t* rows = (uint8_t*)bmpData + y * byteRowStride;
                    uint32_t* rowd = (uint32_t*)pixData + yd * wordRowStride;
                    for (int32_t x = 0; x < width; ++x) {
                        //  windows   BGR?
                        uint32_t rgb;
                        if (bytePerPixel >= 3) {       // discard alpha in case it is there...
                            rgb = (rows[0] << 16u) | (rows[1] << 8u) | rows[2];
                        }
                        else {  // grayscale
                            rgb = (rows[0] << 16u) | (rows[0] << 8u) | rows[0];
                        }
                        //  pixbuf    A 31..24  R 23..16  G 15..8   B 7..0
                        *rowd = 0xff000000u | rgb;
                        ++rowd;
                        rows += bytePerPixel;
                    }
                }
                m_scaled.clear(); // need to refresh
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
Preview::scan()
{
    // use member so we wont fail on destruction of thread and can keep transfer
    if (m_workThread != nullptr) {
        if (m_workThread->joinable()) {
            std::cout << "Joining ..." << std::endl;
            m_workThread->join();   // wait for prev.
        }
        std::cout << "Destroy prev" << std::endl;
        delete m_workThread;
        m_workThread = nullptr;
    }
    if (m_worker) {
        delete m_worker;
        m_worker = nullptr;
    }
    if (m_workThread == nullptr) {
        m_initScan = true;
        m_worker = new WorkThread(m_dispatcher);
        //m_workThread = ThreadWrapper ([] (WorkThread* worker) {
        //    worker->run();
        //}, m_worker);
        m_workThread = new std::thread(
            [this] {
            m_worker->run();
        });
    }
    //t.join ();


}
