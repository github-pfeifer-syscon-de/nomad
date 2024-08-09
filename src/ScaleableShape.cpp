/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
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

#include <iostream>

#include "ScaleableShape.hpp"
#include "Preview.hpp"
#include "CairoShape.hpp"

int
ScaleableShape::toRealWidth(int outpWidth)
{
    return static_cast<int>(m_scale * static_cast<double>(outpWidth));
}

int
ScaleableShape::toRealHeight(int outpHeight)
{
    return static_cast<int>(m_scale * static_cast<double>(outpHeight));
}

void
ScaleableShape::setScale(double scale)
{
    m_scale = scale;
}

double
ScaleableShape::getScale()
{
    return m_scale;
}

void
ScaleableShape::setRotate(int rotate)
{
    m_rotate = rotate;
}

int ScaleableShape::getRotate()
{
    return m_rotate;
}


void
ScaleableShape::edit(Preview& preview)
{
    if (askScale(preview, this)) {
        preview.queue_draw();
    }
}


bool
ScaleableShape::askScale(Preview& preview, ScaleableShape* shape)
{
    bool ret = false;
    ApplicationSupport& appSupport = preview.getApplicationSupport();
	auto builder = Gtk::Builder::create();
    try {
        builder->add_from_resource(appSupport.getApplication()->get_resource_base_path() + "/scale-dlg.ui");
		Gtk::Dialog* dlg;
        builder->get_widget("dlg", dlg);
		Gtk::Scale* scale;
        builder->get_widget("scale", scale);
        scale->set_value(shape->getScale());
        Gtk::ColorButton* color;
        builder->get_widget("color", color);
        Gtk::Label* lblColor;
        builder->get_widget("lblColor", lblColor);
        Gtk::SpinButton* spinRotate;
        builder->get_widget("rotate", spinRotate);
        spinRotate->set_value(shape->getRotate());
        CairoShape* cairoShape = dynamic_cast<CairoShape*>(shape);
        if (cairoShape) {
            color->set_color(cairoShape->getColor());
        }
        else {
            color->set_visible(false);
            lblColor->set_visible(false);
        }
	    int result = dlg->run();
		switch (result) {
			case Gtk::RESPONSE_OK:
                std::cout << "Scale " << scale->get_value() << std::endl;
                std::cout << "Rotate " << spinRotate->get_value_as_int() << std::endl;
                shape->setScale(scale->get_value());
                shape->setRotate(spinRotate->get_value_as_int());
                if (cairoShape) {
                    cairoShape->setColor(color->get_color());
                }
                ret = true;
				break;
			default:
				break;
		}
		delete dlg;
    }
    catch (const Glib::Error &ex) {
        appSupport.showError(Glib::ustring::sprintf("Unable to load scale-dlg: %s",  ex.what()));
    }
    return ret;
}


