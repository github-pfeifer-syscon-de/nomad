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

Config::Config(Glib::KeyFile* config)
: Glib::ObjectBase(typeid (Config))
, m_config{config}
{
    read_config();
}

void
Config::read_config()
{
    if (m_config->has_group(MAIN_GRP)
      && m_config->has_key(MAIN_GRP, DELAY_CONF)) {
        m_delaySec = m_config->get_integer(MAIN_GRP, DELAY_CONF);
    }
    if (m_config->has_group(MAIN_GRP)
     && m_config->has_key(MAIN_GRP, CAPTURE_WINDOW_CONF)) {
        m_captureWindow = m_config->get_boolean(MAIN_GRP, CAPTURE_WINDOW_CONF);
    }
    if (m_config->has_group(MAIN_GRP)
     && m_config->has_key(MAIN_GRP, BACKGROUNDCOLOR_CONF)) {
        auto color = m_config->get_string(MAIN_GRP, BACKGROUNDCOLOR_CONF);
        m_background.set(color);
    }
    else {
        m_background.set("#000");
    }
    if (m_config->has_group(MAIN_GRP)
     && m_config->has_key(MAIN_GRP, FOREGROUNDCOLOR_CONF)) {
        auto color = m_config->get_string(MAIN_GRP, FOREGROUNDCOLOR_CONF);
        m_foreground.set(color);
    }
    else {
        m_foreground.set("#fff");
    }
    if (m_config->has_group(MAIN_GRP)
     && m_config->has_key(MAIN_GRP, TEXTFONT_CONF)) {
        m_textFont = m_config->get_string(MAIN_GRP, TEXTFONT_CONF);
    }
}

void
Config::save_config()
{
    if (m_config) {
        m_config->set_integer(MAIN_GRP, DELAY_CONF, m_delaySec);
        m_config->set_boolean(MAIN_GRP, CAPTURE_WINDOW_CONF, m_captureWindow);
        m_config->set_string(MAIN_GRP, BACKGROUNDCOLOR_CONF, m_background.to_string());
        m_config->set_string(MAIN_GRP, FOREGROUNDCOLOR_CONF, m_foreground.to_string());
        m_config->set_string(MAIN_GRP, TEXTFONT_CONF, m_textFont);
    }
}
