/* -*- Mode: c++; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
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
#include <glibmm.h>
#include <gdkmm-3.0/gdkmm/color.h>
#include <gdkmm-3.0/gdkmm/rgba.h>

#include "Config.hpp"

Config::Config()
: Glib::ObjectBase(typeid (Config))
, m_config{nullptr}
//, property_delay_(*this, DELAY_CONF, Glib::ustring::sprintf("%d", getDelay()))
//, property_captWindow_(*this, CAPTURE_WINDOW_CONF, isCaptureWindow())
{
    read_config();
}

Glib::ustring
Config::get_config_name()
{
    auto cfgdir = Glib::get_user_config_dir();
    Glib::ustring fullPath = Glib::canonicalize_filename(CONF_FILE, cfgdir.c_str());
    return fullPath;
}

void
Config::read_config()
{
    m_config = new Glib::KeyFile();
    auto fullPath = get_config_name();
    try {
        if (m_config->load_from_file(fullPath, Glib::KEY_FILE_NONE)) {
            if (m_config->has_group(MAIN_GRP)) {
                if (m_config->has_key(MAIN_GRP, DELAY_CONF)) {
                    m_delaySec = m_config->get_integer(MAIN_GRP, DELAY_CONF);
                }
                if (m_config->has_key(MAIN_GRP, CAPTURE_WINDOW_CONF)) {
                    m_captureWindow = m_config->get_boolean(MAIN_GRP, CAPTURE_WINDOW_CONF);
                }
                if (m_config->has_key(MAIN_GRP, BACKGROUNDCOLOR_CONF)) {
                    auto color = m_config->get_string(MAIN_GRP, BACKGROUNDCOLOR_CONF);
                    m_background.set(color);
                }
                if (m_config->has_key(MAIN_GRP, FOREGROUNDCOLOR_CONF)) {
                    auto color = m_config->get_string(MAIN_GRP, FOREGROUNDCOLOR_CONF);
                    m_foreground.set(color);
                }
                if (m_config->has_key(MAIN_GRP, TEXTFONT_CONF)) {
                    m_textFont = m_config->get_string(MAIN_GRP, TEXTFONT_CONF);
                }
            }
        }
        else {
            std::cerr << "Error loading " << fullPath << std::endl;
        }
    }
    catch (const Glib::FileError& exc) {
        // may happen if didn't create a file (yet) but we can go on
        std::cerr << "File Error loading " << fullPath << " if its missing, it will be created?" << std::endl;
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
        auto fullPath = get_config_name();
        if (!m_config->save_to_file(fullPath)) {
             std::cerr << "Error saving " << fullPath << std::endl;
        }
    }
}
