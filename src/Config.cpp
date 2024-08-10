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
#include <gtkmm.h>

#include "Config.hpp"

Config::Config(const char* configName)
: Glib::ObjectBase(typeid (Config))
, KeyConfig{configName}
{
}

void
Config::loadConfig()
{
    KeyConfig::loadConfig();
    if (hasKey(MAIN_GRP, BACKGROUNDCOLOR_CONF)) {
        auto color = getString(MAIN_GRP, BACKGROUNDCOLOR_CONF);
        m_background.set(color);
    }
    else {
        m_background.set("#000");
    }
    if (hasKey(MAIN_GRP, FOREGROUNDCOLOR_CONF)) {
        auto color = getString(MAIN_GRP, FOREGROUNDCOLOR_CONF);
        m_foreground.set(color);
    }
    else {
        m_foreground.set("#fff");
    }
    if (hasKey(MAIN_GRP, TEXTFONT_CONF)) {
        m_textFont = getString(MAIN_GRP, TEXTFONT_CONF);
    }
}

void
Config::saveConfig()
{
    setString(MAIN_GRP, BACKGROUNDCOLOR_CONF, m_background.to_string());
    setString(MAIN_GRP, FOREGROUNDCOLOR_CONF, m_foreground.to_string());
    setString(MAIN_GRP, TEXTFONT_CONF, m_textFont);
    KeyConfig::saveConfig();
}
