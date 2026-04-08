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
#include <ApplicationSupport.hpp>

#include "nomad_config.h"

#include "ImageReader.hpp"

#ifdef LIBRAW
#include "RawImageReader.hpp"
#endif

#undef DEBUG_READER

void
GtkImageReader::prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter)
{
    filter->add_pixbuf_formats();
}

void
GtkImageReader::addCustomPreview(Gtk::FileChooser& chooser, ApplicationSupport& applicationSupport)
{

}

Glib::RefPtr<Gdk::Pixbuf>
GtkImageReader::read(const Glib::RefPtr<Gio::File>& file, ImageFileChooser& file_chooser)
{
    //auto strm = file->read();
    auto pixbuf = Gdk::Pixbuf::create_from_file(file->get_path());
    //strm->close();
    return pixbuf;
}

std::shared_ptr<ImageReaders> ImageReaders::m_imageReaders;

ImageReaders::ImageReaders()
{
#   ifdef LIBRAW    // try this first, as we use extensions and this should be fast
    m_readers.push_back(std::make_shared<RawImageReader>());
#   endif
    m_readers.push_back(std::make_shared<GtkImageReader>());
}

std::shared_ptr<ImageReaders>
ImageReaders::createDefault()
{
    // as we are only holding the state "have raw" which persists
    //   for the whole program livetime, a singleton should be ok
    if (!m_imageReaders) {
        m_imageReaders = std::make_shared<ImageReaders>();
    }
    return m_imageReaders;
}

Glib::RefPtr<Gdk::Pixbuf>
ImageReaders::read(const Glib::RefPtr<Gio::File>& file, ImageFileChooser& file_chooser) const
{
    for (auto reader : m_readers) { // no auto& as the pixbuf decays to early :(
        auto pixbuf = reader->read(file, file_chooser);
        if (pixbuf) {
#           ifdef DEBUG_READER
            std::cout << "ImageReaders::read got"
                      << " pixbuf " << pixbuf->get_width() << "x" << pixbuf->get_height()
                      << " for " << file->get_path() << std::endl;
#           endif
            return pixbuf;
        }
    }
#   ifdef DEBUG_READER
    std::cout << "ImageReaders::read no reader accepted " << file->get_path()
              << " readers " << m_readers.size() << std::endl;
#   endif
    return {};
}

void
ImageReaders::prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter)
{
    for (auto& reader : m_readers) {
        reader->prepareFilter(filter);
    }
}

void
ImageReaders::addCustomPreview(Gtk::FileChooser& chooser
                , ApplicationSupport& applicationSupport)
{
    for (auto& reader : m_readers) {
        reader->addCustomPreview(chooser, applicationSupport);
    }
}

