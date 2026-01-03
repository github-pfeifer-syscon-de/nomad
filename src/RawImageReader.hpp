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

#include <libraw/libraw.h>

#include "ImageReader.hpp"

class ApplicationSupport;

class RawImageHelper
{
public:
    RawImageHelper(libraw_processed_image_t* memimg);
    explicit RawImageHelper(const RawImageHelper& rawHelper) = delete;
    virtual ~RawImageHelper();

    Glib::RefPtr<Gdk::Pixbuf> getPixbuf();
private:
    libraw_processed_image_t* m_memimg;
};


// give this some RAII
class RawHelper {
public:
    RawHelper();
    explicit RawHelper(const RawHelper& rawHelper) = delete;
    virtual ~RawHelper();

    RawImageHelper read(const Glib::RefPtr<Gio::File>& file);
    // this was build for Nef-Files with a 160x120 preview (other formats may different conventions)
    Glib::RefPtr<Gdk::Pixbuf> readPreview(const Glib::RefPtr<Gio::File>& file);

    void setBrightness(float bright);
    void setContrast(float contrast);
    static constexpr size_t RAW_RED{0};    // indexes taken from raw - example
    static constexpr size_t RAW_GREEN{1};
    static constexpr size_t RAW_BLUE{2};
    static constexpr size_t RAW_GREEN2{3};

protected:
    //alternate example
    Glib::RefPtr<Gdk::Pixbuf> processRaw2(const Glib::RefPtr<Gio::File>& file);
    LibRaw m_iProcessor;
    float m_bright{};
    float m_contrast{};

};

class RawImagePreview
: public Gtk::Box
{
public:
    RawImagePreview(
          BaseObjectType* cobject
        , const Glib::RefPtr<Gtk::Builder>& builder);
    virtual ~RawImagePreview() = default;

    bool update_preview(const Glib::RefPtr<Gio::File>& rawFile);
    void reset_preview();
    float getBrightness();
    float getContrast();
    static constexpr auto DEF_BRIGHT{1.0f};
    static constexpr auto DEF_CONTRAST{0.45f};
private:
    Gtk::Scale* m_bright;
    Gtk::Scale* m_contrast;
    //Gtk::SpinButton* m_rotate;
    Gtk::Image* m_image;

};

class RawImageReader
: public ImageReader
{
public:
    RawImageReader() = default;
    explicit RawImageReader(const RawImageReader& imageReaders) = delete;
    virtual ~RawImageReader() = default;
    static const std::vector<std::string> rawExtensions()
    {
        std::vector<std::string> rawExt;
        for (auto& ext : RAW_EXT) {
            rawExt.push_back(ext);
        }
        return rawExt;
    }

    void prepareFilter(const Glib::RefPtr<Gtk::FileFilter>& filter) override;
    void addCustomPreview(Gtk::FileChooser& chooser, ApplicationSupport& appSupport) override;

    Glib::RefPtr<Gdk::Pixbuf> read(const Glib::RefPtr<Gio::File>& file
            , ImageFileChooser& file_chooser) override;
    bool acceptFile(const Gtk::FileFilter::Info& filter_info);
    void update_preview();

protected:
    static constexpr const char* RAW_EXT[] {
     "nef"
    ,"crw"
    };
private:
    std::set<std::string> m_types;
    RawImagePreview* m_rawImagePreview{nullptr};
    Gtk::FileChooser* m_chooser{nullptr};
};

