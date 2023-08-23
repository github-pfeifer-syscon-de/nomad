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

#pragma once

#include <Mode.hpp>

class NomadWin;

class EditMode
: public Mode
{
public:
    EditMode();
    explicit EditMode(const EditMode& orig) = delete;
    virtual ~EditMode() = default;
    void setNomadWin(NomadWin* nomadWin);

    bool isComplete() override;
    void show(ViewIntf* viewIntf) override;
    void buildMenu(Gtk::Menu* subMenu, ViewIntf* imageView, function_ptr fun) override;
    void prev() override;
    void next() override;
    void set(int32_t n) override;
    bool join(std::shared_ptr<Mode> other) override;
    bool hasNavigation() override;
private:
    NomadWin* m_nomadWin;
};

