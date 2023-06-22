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
#include <StringUtils.hpp>
#include <ImageUtils.hpp>

#include "ScanPreview.hpp"
#include "NomadWin.hpp"
#include "WiaProperty.hpp"
#include "config.h"

#ifdef USE_PDF
#include "PdfExport.hpp"
#include "PdfPage.hpp"
#include "PdfFont.hpp"
#include "PdfImage.hpp"
#endif

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
                m_bytePerPixel = bi->biBitCount / 8;
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
                        if (m_bytePerPixel >= 3) {       // discard alpha in case it is there...
                            rgb = (rows[0] << 16u) | (rows[1] << 8u) | rows[2];
                        }
                        else {
                            uint8_t gray;
                            if (m_bytePerPixel == 1)  {   // grayscale
                                gray = *rows;
                            }
                            else {                      // b&w
                                auto bit = rows[x >> 3u] & (0x80u >> (x & 0x7u));
                                gray = bit != 0 ? 0xffu : 0u;
                            }
                            rgb = (gray << 16u) | (gray << 8u) | gray;
                        }
                        //  pixbuf    A 31..24  R 23..16  G 15..8   B 7..0
                        *rowd = 0xff000000u | rgb;
                        ++rowd;
                        rows += m_bytePerPixel;
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
ScanPreview::saveImage(const Glib::ustring& file)
{
    if (m_pixbuf) {
        if (StringUtils::endsWith(file, ".png")) {
            if (m_bytePerPixel >= 3) {
                m_pixbuf->save(file, "png");
            }
            else {
                ImageUtils::grayscalePng(m_pixbuf, file);
            }
            return true;
        }
#ifdef USE_PDF
        else if(StringUtils::endsWith(file, ".pdf")) {
            exportPdf(file);
            return true;
        }
#endif
        else {
            std::cout << "Cannot handle " << file << " expecting e.g. .png!" << std::endl;
        }
    }
    return false;
}

#ifdef USE_PDF
void
ScanPreview::exportPdf(const Glib::ustring& file)
{
    auto pdfExport = std::make_shared<PdfExport>();
    auto helv = pdfExport->createFont("Helvetica");
    auto page = std::make_shared<PdfPage>(pdfExport);
    page->setFont(helv, 20);
    page->drawText("PngDemo", 220, page->getHeight() - 70);
    page->setFont(helv, 12);

    std::string tempName;
    int h = Glib::file_open_tmp(tempName, "temp");
    close(h);
    if (m_bytePerPixel >= 3) {
        m_pixbuf->save(tempName, "png");
    }
    else {
        ImageUtils::grayscalePng(m_pixbuf, tempName);
    }

    auto img = std::make_shared<PdfImage>(pdfExport);
    img->loadPng(tempName);
    std::cout << " width " << img->getWidth()
              << " height " << img->getHeight()
              << " PngImage " << tempName
              << std::endl;
    float pageAvailWidth = page->getWidth() - 100.0f;
    float pageAvailHeight = page->getHeight() - 100.0f;
    float imageWidth = img->getWidth();
    float imageHeight = img->getHeight();
    float relWidth = pageAvailWidth / imageWidth;
    float relHeight = pageAvailHeight / imageHeight;
    float scale = std::min(relWidth, relHeight);
    float scaledWidth = imageWidth * scale;
    float scaledHeight = imageHeight * scale;
    page->drawImage(img,
            50.0f, 50.0f,
            scaledWidth, scaledHeight);
            //page->getWidth() - 100.0f, page->getHeight() - 100.0f);
//    page->drawPng("res/basn0g01.png", 100, page->getHeight() - 150);
//    page->drawText("1bit grayscale.\nbasn0g01.png", 100, page->getHeight() - 150);
//    page->drawPng("res/basn0g02.png", 200, page->getHeight() - 150);
//    page->drawText("2bit grayscale.\nbasn0g02.png", 200, page->getHeight() - 150);
//    page->drawPng("res/basn0g04.png", 300, page->getHeight() - 150);
//    page->drawText("4bit grayscale.\nbasn0g04.png", 300, page->getHeight() - 150);
//    page->drawPng("res/basn0g08.png", 400, page->getHeight() - 150);
//    page->drawText("8bit grayscale.\nbasn0g08.png", 400, page->getHeight() - 150);
    pdfExport->save(file);

    auto tfile = Gio::File::create_for_path(tempName);
    tfile->remove();
}
#endif