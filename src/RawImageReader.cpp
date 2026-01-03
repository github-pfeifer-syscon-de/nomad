/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2025 RPf
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
#include <cstring>
#include <StringUtils.hpp>

#include "RawImageReader.hpp"

#include <ApplicationSupport.hpp>
#include <ImageFileChooser.hpp>

#define DEBUG_PREVIEW 1



RawImageHelper::RawImageHelper(libraw_processed_image_t* memimg)
: m_memimg{memimg}
{
}

RawImageHelper::~RawImageHelper()
{
    if (m_memimg) {
        LibRaw::dcraw_clear_mem(m_memimg);
    }
}

Glib::RefPtr<Gdk::Pixbuf>
RawImageHelper::getPixbuf()
{
#   ifdef DEBUG_PREVIEW
    std::cout << "RawImageHelper::getPixbuf Converted RAW image size: " << m_memimg->width
              << " x " << m_memimg->height << std::endl;
#   endif
    Glib::RefPtr<Gdk::Pixbuf> pixbuf;
    if (m_memimg->bits == 8 && m_memimg->colors == 3 && m_memimg->type == 2) {
        pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, m_memimg->width, m_memimg->height);
#       ifdef DEBUG_PREVIEW
        std::cout << "Gdk::Pixbuf rowstride " << pixbuf->get_rowstride()
                  << " bits " << pixbuf->get_bits_per_sample()
                  << " chan " << pixbuf->get_n_channels() << std::endl;
#       endif
        auto dest = pixbuf->get_pixels();
        auto src = &m_memimg->data;
        const size_t rawRowSize{static_cast<unsigned>(m_memimg->width * m_memimg->colors)};
        for (size_t r = 0; r < m_memimg->height; r++) {
            std::memcpy(dest, src, rawRowSize);
            dest += pixbuf->get_rowstride();
            src += rawRowSize;
        }
    }
    else {
        std::cout << "We are only prepared for bits=8 actual " << m_memimg->bits
                  << " colors=3 actual " << m_memimg->colors
                  << " type=2 actual " << m_memimg->type << std::endl;
    }
    return pixbuf;
}

RawHelper::RawHelper()
{
}

RawHelper::~RawHelper()
{
    m_iProcessor.recycle();
}

void
RawHelper::setBrightness(float bright)
{
    m_bright = bright;
}
void
RawHelper::setContrast(float contrast)
{
    m_contrast = contrast;
}

// this gives nice looking 8bit-data
RawImageHelper
RawHelper::read(const Glib::RefPtr<Gio::File>& file) {
    // Let us create an image processor
    std::cout << "Open the file " << file->get_path() << " and read the metadata" << std::endl;
    // a extensions list would be nice...
    //const char **clist = LibRaw::cameraList();
    //const char **cc = clist;
    //while (*cc) {
    //    printf("%s\n", *cc);
    //    cc++;
    //}
    // as we are (at the moment) not interested in meta stuff
    // putenv((char *)"TZ=UTC"); // dcraw compatibility, affects TIFF datestamp field

    int ret = m_iProcessor.open_file(file->get_path().c_str());
    if (ret) {
        std::cout << "Cannot open file" << std::endl;
        throw std::runtime_error("Cannot open file");
    }
    ret = m_iProcessor.unpack();
    if (ret != LIBRAW_SUCCESS) {
        std::cout << "Unable to unpack " << libraw_strerror(ret) << std::endl;
        throw std::runtime_error("Unable to unpack");
    }
    // allow adjusting the parameters
    if (m_bright != 0.0f) {
        m_iProcessor.imgdata.params.bright = m_bright;
    }
    if (m_contrast != 0.0f) {
        m_iProcessor.imgdata.params.gamm[0] = m_contrast;
    }
    ret = m_iProcessor.dcraw_process();
    if (ret != LIBRAW_SUCCESS) {
        std::cout << "Unable to dcraw_process " << libraw_strerror(ret) << std::endl;
        throw std::runtime_error("Unable to dcraw_process");
    }

    auto memimg = m_iProcessor.dcraw_make_mem_image(&ret);
    if (!memimg) {
        std::cout << "error make_mem_image " << libraw_strerror(ret) << std::endl;
        throw std::runtime_error("Cannot make mem image");
    }
    return RawImageHelper(memimg);
}

Glib::RefPtr<Gdk::Pixbuf>
RawHelper::readPreview(const Glib::RefPtr<Gio::File>& file)
{
    int ret = m_iProcessor.open_file(file->get_path().c_str());
    if (ret) {
        std::cout << "Cannot open file" << std::endl;
        return Glib::RefPtr<Gdk::Pixbuf>();
    }
    ret = m_iProcessor.unpack();
    if (ret != LIBRAW_SUCCESS) {
        std::cout << "Unable to unpack " << libraw_strerror(ret) << std::endl;
        return Glib::RefPtr<Gdk::Pixbuf>();
    }
    for (int t = 0; t < m_iProcessor.imgdata.thumbs_list.thumbcount; t++) {
        if ((ret = m_iProcessor.unpack_thumb_ex(t)) == LIBRAW_SUCCESS) {
            std::cout << "Got thumb " << t
                      << " size " << m_iProcessor.imgdata.thumbnail.twidth
                      << "x" << m_iProcessor.imgdata.thumbnail.theight
                      << " fmt " << (m_iProcessor.imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG
                            ? Glib::ustring("jpg")
                            : Glib::ustring::sprintf("fmt %d", m_iProcessor.imgdata.thumbnail.tformat)) << std::endl;
            if (m_iProcessor.imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_BITMAP) {
                // as it seems this doesn't honor params from above
                auto thumbImg = m_iProcessor.dcraw_make_mem_thumb(&ret);
                if (thumbImg) {
                    RawImageHelper rawImageHelper(thumbImg);
                    auto thumbPix = rawImageHelper.getPixbuf();
                    return thumbPix;
                }
                else {
                    std::cout << "Cannot dcraw_make_mem_thumb #" << t
                              << " error " << libraw_strerror(ret) << std::endl;
                }
            }
        }
    }
    return Glib::RefPtr<Gdk::Pixbuf>();
}

// this gives us 16bit-data (requries advanced processing)
//   at the moment linear scaling gives us some impression of the file
//   nice for astro?
Glib::RefPtr<Gdk::Pixbuf>
RawHelper::processRaw2(const Glib::RefPtr<Gio::File>& file)
{
    // Let us create an image processor
#   ifdef DEBUG_PREVIEW
    std::cout << "RawHelper::processRaw2 Open the file " << file->get_path() << " and read the metadata" << std::endl;
#   endif
    //
    m_iProcessor.open_file(file->get_path().c_str());
#   ifdef DEBUG_PREVIEW
    // The metadata are accessible through data fields of the class
    std::cout << "Image size: " << m_iProcessor.imgdata.sizes.width
              << " x " << m_iProcessor.imgdata.sizes.height << "\n"
              << "Let us unpack the image" << std::endl;
#   endif
    m_iProcessor.unpack();
#   ifdef DEBUG_PREVIEW
    std::cout << "Convert from imgdata.rawdata to imgdata.image:" << std::endl;
#   endif
    m_iProcessor.raw2image();
    //iProcessor.adjust_maximum(); // does not change anything, tested various locations

    ushort pixelMax{};
    for (size_t r = 0; r < m_iProcessor.imgdata.sizes.iheight; r++) {
        size_t sStart = r * m_iProcessor.imgdata.sizes.iwidth;
        for(size_t c = 0; c < m_iProcessor.imgdata.sizes.iwidth; c++) {
            size_t s = sStart + c;
            pixelMax = std::max(pixelMax,
                            std::max(m_iProcessor.imgdata.image[s][RAW_RED],
                                std::max(m_iProcessor.imgdata.image[s][RAW_GREEN],
                                    std::max(m_iProcessor.imgdata.image[s][RAW_BLUE], m_iProcessor.imgdata.image[s][RAW_GREEN2]))));
        }
    }
    float scale = 255.0f / static_cast<float>(pixelMax);
#   ifdef DEBUG_PREVIEW
    std::cout << "pixel max " << pixelMax
              << " scale " << scale << std::endl;
#   endif
    const float scale_2 = scale / 2.0f;  // scale the two greens by half

    // And let us print its dump; the data are accessible through data fields of the class
    auto pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, m_iProcessor.imgdata.sizes.iwidth, m_iProcessor.imgdata.sizes.iheight);
#   ifdef DEBUG_PREVIEW
    std::cout << "Gdk::Pixbuf rowstride " << pixbuf->get_rowstride()
              << " bits " << pixbuf->get_bits_per_sample()
              << " chan " << pixbuf->get_n_channels() << std::endl;
#   endif
    auto pixels = pixbuf->get_pixels();

    for (size_t r = 0; r < m_iProcessor.imgdata.sizes.iheight; r++) {
        size_t sStart = r * m_iProcessor.imgdata.sizes.iwidth;
        size_t dStart = r * pixbuf->get_rowstride();
        for(size_t c = 0; c < m_iProcessor.imgdata.sizes.iwidth; c++) {
            size_t s = sStart + c;
            size_t d = dStart + c * pixbuf->get_n_channels();
            pixels[d+0] = static_cast<uint8_t>((m_iProcessor.imgdata.image[s][RAW_RED] * scale) );
            pixels[d+1] = static_cast<uint8_t>((m_iProcessor.imgdata.image[s][RAW_BLUE] * scale) );
            pixels[d+2] = static_cast<uint8_t>((m_iProcessor.imgdata.image[s][RAW_GREEN] +
                                               m_iProcessor.imgdata.image[s][RAW_GREEN2]) * scale_2);
        }
    }
    return pixbuf;
}

RawImagePreview::RawImagePreview(
      BaseObjectType* cobject
    , const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Box(cobject)
{
    builder->get_widget("bright", m_bright);
    m_bright->set_value(DEF_BRIGHT);
    builder->get_widget("contrast", m_contrast);
    m_contrast->set_value(DEF_CONTRAST);
    //builder->get_widget("rotate", m_rotate);
    //m_rotate->set_value(0.0);
    builder->get_widget("image", m_image);
}

bool
RawImagePreview::update_preview(const Glib::RefPtr<Gio::File>& rawFile)
{
    RawHelper rawHelper;
    auto pixbuf = rawHelper.readPreview(rawFile);
    if (pixbuf) {
        m_image->set(pixbuf);
        return true;
    }
    return false;
}

void
RawImagePreview::reset_preview()
{
    m_image->clear();
}

float
RawImagePreview::getBrightness()
{
    float bright{DEF_BRIGHT};
    if (m_bright) {
        bright = static_cast<float>(m_bright->get_value());
    }
    return bright;
}

float
RawImagePreview::getContrast()
{
    float contrast{DEF_CONTRAST};
    if (m_contrast) {
        contrast = static_cast<float>(m_contrast->get_value());
    }
    return contrast;
}


void
RawImageReader::prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter)
{
    if (m_types.empty()) {
        for (auto& ext : rawExtensions()) {
            m_types.insert(ext);
        }
    }
    filter->add_custom(Gtk::FileFilterFlags::FILE_FILTER_FILENAME, sigc::mem_fun(*this, &RawImageReader::acceptFile));
}

void
RawImageReader::update_preview() {
    auto file = m_chooser->get_preview_file();
    bool set{false};
    if (file) {
        auto ext = StringUtils::lower(StringUtils::getExtension(file));
        if (m_types.contains(ext)) {
            if (m_rawImagePreview->update_preview(file)) {
                set = true;
            }
        }
    }
    if (!set) {
        m_rawImagePreview->reset_preview();
    }
}

void
RawImageReader::addCustomPreview(Gtk::FileChooser& chooser, ApplicationSupport& appSupport)
{
    m_chooser = &chooser;
    auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(appSupport.getApplication()->get_resource_base_path() + "/raw-dlg.ui");
        // this is sufficient, destructor for preview is called
        builder->get_widget_derived("content", m_rawImagePreview);
        chooser.set_preview_widget(*m_rawImagePreview);
        chooser.signal_update_preview().connect(sigc::mem_fun(*this, &RawImageReader::update_preview));
    }
    catch (const Glib::Error &ex) {
        std::cout<< Glib::ustring::sprintf("Unable to load raw-dlg: %s",  ex.what()) << std::endl;
    }
}




Glib::RefPtr<Gdk::Pixbuf>
RawImageReader::read(const Glib::RefPtr<Gio::File>& file, ImageFileChooser& file_chooser)
{
    bool accept{false};
    auto fileExt = StringUtils::lower(StringUtils::getExtension(file->get_basename()));
    for (auto& ext : rawExtensions()) {
        if (ext == fileExt) {
            accept = true;
            break;
        }
    }
    if (accept) {
        auto preview = file_chooser.get_preview_widget();
        auto rawPreview = dynamic_cast<RawImagePreview*>(preview);
        RawHelper rawHelper;
        if (rawPreview) {
    #       ifdef DEBUG_PREVIEW
            std::cout << "RawImageReader::read using "
                << " bright " << rawPreview->getBrightness()
                << " contrast " << rawPreview->getContrast()
                << "file " << file->get_path() << std::endl;
    #       endif
            rawHelper.setBrightness(rawPreview->getBrightness());
            rawHelper.setContrast(rawPreview->getContrast());
        }
        auto rawImageHelper = rawHelper.read(file);
        return rawImageHelper.getPixbuf();
    }
#   ifdef DEBUG_PREVIEW
    std::cout << "RawImageReader::read not accepted " << file->get_path() << std::endl;
#   endif
    return Glib::RefPtr<Gdk::Pixbuf>();
}


bool
RawImageReader::acceptFile(const Gtk::FileFilter::Info& filter_info)
{
    auto ext = StringUtils::getExtension(filter_info.filename);
    return m_types.contains(StringUtils::lower(ext));
}
