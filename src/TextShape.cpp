/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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
#include <cairomm/cairomm.h>

#include "TextShape.hpp"
#include "Shape.hpp"
#include "Preview.hpp"

TextShape::TextShape()
: Shape()
{
}

void
TextShape::setText(const TextInfo& text)
{
    m_text = text;
}

bool
TextShape::render(
        const Cairo::RefPtr<Cairo::Context>& cairoCtx
        , int outpWidth
        , int outpHeight)
{
    int x = toRealX(outpWidth);
    int y = toRealY(outpHeight);
    cairoCtx->move_to(x, y);
    //std::cout << "Text"
    //          << " width " <<  extends.width
    //          << " height " <<  extends.height
    //          << std::endl;
    cairoCtx->set_source_rgb(
            m_text.getColor().get_red_p(),
            m_text.getColor().get_green_p(),
            m_text.getColor().get_blue_p());
    auto layout = Pango::Layout::create(cairoCtx);
    auto descr = Pango::FontDescription(m_text.getFont());
    // make size depend on output
    auto size = static_cast<int>(static_cast<double>(outpWidth) / 400.0 * static_cast<double>(descr.get_size()));
    descr.set_size(size);       // set_absolute_size(size);
    layout->set_font_description(descr);
    layout->set_text(m_text.getText());
    //auto font =
    //    Cairo::ToyFontFace::create(m_text.getFont(),
    //                           m_text.getSlant(),
    //                           m_text.getWeight());
    //cairoCtx->set_font_face(font);
    //cairoCtx->show_text(m_text.getText());
    layout->show_in_cairo_context(cairoCtx);
    auto extend = layout->get_ink_extents();

    Pango::Rectangle pixels;
    layout->get_pixel_extents(extend, pixels);
    m_ext_width = pixels.get_width();
    m_ext_height = pixels.get_height();
    //std::cout << "TextShape::render"
    //          << " descr size " << descr.get_size()
    //        //  << " size " << size
    //          << " width " << m_ext_width
    //          << " height " << m_ext_height
    //          << std::endl;
    //cairoCtx->set_line_width(1.0);
    //cairoCtx->set_source_rgb(0.5, 0.5, 0.5);
    //cairoCtx->rectangle(x, y,
    //                    pixels.get_width(), pixels.get_height());
    //cairoCtx->stroke();
    return true;
}

Gdk::Rectangle
TextShape::getBounds(
        int width
        , int height)
{
    Gdk::Rectangle next;
    // this assumes text was rendered with actual values ...
    next.set_x(toRealX(width));
    next.set_y(toRealY(height));
    next.set_width(static_cast<int>(m_ext_width));
    next.set_height(static_cast<int>(m_ext_height));
    return next;
}

void
TextShape::edit(Preview& preview)
{
    if (ask_text(m_text, preview)) {
        preview.queue_draw();
    }
}

bool
TextShape::ask_text(TextInfo& textInfo, Preview& preview)
{
	bool ret = false;
    ApplicationSupport& appSupport = preview.getApplicationSupport();
	auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(appSupport.getApplication()->get_resource_base_path() + "/text-dlg.ui");
		Gtk::Dialog* dlg;
        builder->get_widget("dlg", dlg);
		Gtk::Entry* text;
        builder->get_widget("text", text);
        text->set_text(textInfo.getText());
        Gtk::ColorButton* color;
        builder->get_widget("color", color);
        color->set_color(textInfo.getColor());
        Gtk::FontButton *font;
        builder->get_widget("font", font);
        font->set_font_name(textInfo.getFont());
	    int result = dlg->run();
		switch (result) {
			case Gtk::RESPONSE_OK:
				textInfo.setText(text->get_text());
                textInfo.setColor(color->get_color());
                textInfo.setFont(font->get_font_name());
                ret = true;
				break;
			default:
				break;
		}
		delete dlg;
    }
    catch (const Glib::Error &ex) {
        appSupport.showError(Glib::ustring::sprintf("Unable to load text-dlg: %s",  ex.what()));
    }
	return ret;
}
