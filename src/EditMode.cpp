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

#include <DisplayImage.hpp>


#include "EditMode.hpp"
#include "NomadWin.hpp"

EditMode::EditMode()
{
}

void
EditMode::setNomadWin(NomadWin* nomadWin)
{
    m_nomadWin = nomadWin;
}

bool
EditMode::isComplete()
{
    return true;
}

void
EditMode::show(ViewIntf* viewIntf)
{
    std::array<int,2> size{800,600};
    auto background = m_nomadWin->getConfig()->getBackgroundColor();
    m_nomadWin->ask_size(size, background);
    auto pixbuf = Gdk::Pixbuf::create(
            Gdk::Colorspace::COLORSPACE_RGB
            , true
            , 8
            , size[0]
            , size[1]);
    guint32 pixel = (background.get_red() >> 8u) << 24u
                  | (background.get_green() >> 8u) << 16u
                  | (background.get_blue())      // eliminated >> 8u) << 8u as this will be a no op
                  | 0xffu;
    pixbuf->fill(pixel);
    auto displayImage = DisplayImage::create(pixbuf);
    viewIntf->setDisplayImage(displayImage);
}

void
EditMode::buildMenu(Gtk::Menu* subMenu, ViewIntf* imageView, function_ptr fun)
{
}

void
EditMode::prev()
{
}

void
EditMode::next()
{
}

void
EditMode::set(int32_t n)
{
}

bool
EditMode::join(std::shared_ptr<Mode> other)
{
    return false;
}

bool EditMode::hasNavigation()
{
    return false;
}
