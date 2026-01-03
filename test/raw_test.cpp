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
#include <glibmm.h>
#include <gtkmm.h>
#include <memory>

#include "config.h"

//   you probably want to adapt this (relative to home dir):
static constexpr const char* RAW_FILEPATH{"Pictures/_DSC9438.NEF"};


#ifdef LIBRAW
#include <libraw.h>

static int
progressCallback(void *callback_data,enum LibRaw_progress stage, int iteration, int expected)
{
    const char* stageName = LibRaw::strprogress(stage);
    std::cout << "stage " << stageName
              << " iter " << iteration
              << "/" << expected << std::endl;
    return 0;   // keep going
}

static Glib::RefPtr<Gdk::Pixbuf>
toPixbuf(libraw_processed_image_t* memimg, const char* name)
{
    auto ret = Glib::RefPtr<Gdk::Pixbuf>();
    auto pixbuf = Gdk::Pixbuf::create(Gdk::COLORSPACE_RGB, false, 8, memimg->width, memimg->height);
    std::cout << "Gdk::Pixbuf rowstride " << pixbuf->get_rowstride()
              << " bits " << pixbuf->get_bits_per_sample()
              << " chan " << pixbuf->get_n_channels() << std::endl;
    if (memimg->bits == 8 && memimg->colors == 3 && memimg->type == 2) {
        auto dest = pixbuf->get_pixels();
        auto src = &memimg->data;
        const size_t rawRowSize{static_cast<unsigned>(memimg->width * memimg->colors)};
        for (size_t r = 0; r < memimg->height; r++) {
            std::memcpy(dest, src, rawRowSize);
            dest += pixbuf->get_rowstride();
            src += rawRowSize;
        }
        try {
            pixbuf->save(name, "jpeg");
            ret = pixbuf;
        }
        catch (const Glib::Error& err) {
            std::cout << err.what() << std::endl;
        }
    }
    else {
        std::cout << "We are only prepared for bits=8 actual " << memimg->bits
                  << " colors=3 actual " << memimg->colors
                  << " type=2 actual " << memimg->type << std::endl;
    }
return ret;
}
#endif



template<typename T>
using expl_destruct_unique_ptr = std::unique_ptr<T,std::function<void(T*)>>;

// this is a simple test to see what data is accesssible,
//   the errorhandling and resource management is poor.
static bool
check_raw_read()
{
#   ifdef LIBRAW
    auto filePath = Glib::build_filename(Glib::get_home_dir(), RAW_FILEPATH);
    auto file = Gio::File::create_for_path(filePath);
    if (!file->query_exists()) {            // can't blame the test if file isn't there
        std::cout << "The requested file " << file->get_path() << " was not found nothing tested!" << std::endl;
        return true;
    }
    expl_destruct_unique_ptr<LibRaw> iRawProcessor(new LibRaw(), [](LibRaw *libRaw) {
        libRaw->recycle();
    });
    auto cap = iRawProcessor->capabilities();
    std::cout << "Capabilities " << std::hex << cap << std::dec << std::endl;
    // see LibRaw_runtime_capabilities
    auto ver = iRawProcessor->version();
    std::cout << "Version " << ver << std::endl;
    iRawProcessor->set_progress_handler(progressCallback, nullptr);
    int ret = iRawProcessor->open_file(file->get_path().c_str());
    if (ret) {
        std::cout << "Cannot open file" << std::endl;
        return false;
    }
    ret = iRawProcessor->unpack();
    if (ret != LIBRAW_SUCCESS) {
        std::cout << "Unable to unpack " << libraw_strerror(ret) << std::endl;
        return false;
    }
    std::cout << "bright " << iRawProcessor->imgdata.params.bright << std::endl;
    iRawProcessor->imgdata.params.bright = 0.5f; //def 1.0 - is less brightness
    std::cout << "gamm0 " << iRawProcessor->imgdata.params.gamm[0] << std::endl;
    //iRawProcessor->imgdata.params.gamm[0] = 0.4f; def 0.45 - is less contrast
    std::cout << "gamm1 " << iRawProcessor->imgdata.params.gamm[1] << std::endl;
    for (int t = 0; t < iRawProcessor->imgdata.thumbs_list.thumbcount; t++) {
        if ((ret = iRawProcessor->unpack_thumb_ex(t)) != LIBRAW_SUCCESS) {
            std::cout << "Cannot unpack_thumb #" << t
                      << " error " << libraw_strerror(ret) << std::endl;
            return false;
        }
        if (LIBRAW_FATAL_ERROR(ret)) {
            std::cout << "Error retriving thumb " << t << std::endl;
            return false;
        }
        std::cout << "Got thumb " << t
                  << " size " << iRawProcessor->imgdata.thumbnail.twidth
                  << "x" << iRawProcessor->imgdata.thumbnail.theight
                  << " fmt " << (iRawProcessor->imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_JPEG
                        ? Glib::ustring("jpg")
                        : Glib::ustring::sprintf("fmt %d", iRawProcessor->imgdata.thumbnail.tformat)) << std::endl;
        if (iRawProcessor->imgdata.thumbnail.tformat == LIBRAW_THUMBNAIL_BITMAP) {
            // as it seems this doesn't honor params from above
            auto thumbImg = iRawProcessor->dcraw_make_mem_thumb(&ret);
            if (thumbImg) {
                auto thumbPix = toPixbuf(thumbImg, "thumb.jpg");
                LibRaw::dcraw_clear_mem(thumbImg);
            }
            else {
                std::cout << "Cannot dcraw_make_mem_thumb #" << t
                          << " error " << libraw_strerror(ret) << std::endl;
                return false;
            }
        }
    }
    ret = iRawProcessor->dcraw_process();
    if (ret != LIBRAW_SUCCESS) {
        std::cout << "Unable to dcraw_process " << libraw_strerror(ret) << std::endl;
        return false;
    }
    auto memimg = iRawProcessor->dcraw_make_mem_image(&ret);
    if (memimg) {
        auto pixbuf = toPixbuf(memimg, "test.jpg");
        LibRaw::dcraw_clear_mem(memimg);
    }
    else {
        std::cout << "error make_mem_image " << libraw_strerror(ret) << std::endl;
        return false;
    }
#   endif
    return true;
}

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!
    Gio::init();
    Gtk::Main main(0, 0, false);                   // needed to make wrap work

    if (!check_raw_read()) {
        return 1;
    }
    return 0;
}