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

#pragma once

#include <gtkmm.h>
#include <memory>
#include <vector>

#include "config.h"

class ApplicationSupport;
class ImageFileChooser;

class ImageReader
{
public:
    ImageReader() = default;
    explicit ImageReader(const ImageReader& imageReaders) = delete;
    virtual ~ImageReader() = default;

    virtual void prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter) = 0;
    virtual void addCustomPreview(Gtk::FileChooser& chooser
                , ApplicationSupport& applicationSupport) = 0;

    virtual Glib::RefPtr<Gdk::Pixbuf> read(const Glib::RefPtr<Gio::File>& file
                    , ImageFileChooser& file_chooser) = 0;
};

class GtkImageReader
: public ImageReader
{
public:
    GtkImageReader() = default;
    explicit GtkImageReader(const GtkImageReader& imageReaders) = delete;
    virtual ~GtkImageReader() = default;

    void prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter) override;
    void addCustomPreview(Gtk::FileChooser& chooser
        , ApplicationSupport& applicationSupport) override;
    Glib::RefPtr<Gdk::Pixbuf> read(const Glib::RefPtr<Gio::File>& file
                    , ImageFileChooser& file_chooser) override;
};


class ImageReaders
{
public:
    ImageReaders();
    explicit ImageReaders(const ImageReaders& imageReaders) = delete;
    virtual ~ImageReaders() = default;

    Glib::RefPtr<Gdk::Pixbuf> read(const Glib::RefPtr<Gio::File>& file
                , ImageFileChooser& file_chooser);
    void prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter);
    void addCustomPreview(Gtk::FileChooser& chooser
                , ApplicationSupport& applicationSupport);

    static std::shared_ptr<ImageReaders> createDefault();
protected:
    std::vector<std::shared_ptr<ImageReader>> m_readers;
    static std::shared_ptr<ImageReaders> m_imageReaders;
};

