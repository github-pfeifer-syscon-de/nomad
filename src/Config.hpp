/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2023 RPf 
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

#include <string.h>

#include "KeyConfig.hpp"

class Config
: public Glib::Object
, public KeyConfig
{
public:
    Config(const char* configName);
    explicit Config(const Config& cnf) = delete;
    virtual ~Config() = default;

    int getDelay() {
        return getInteger(MAIN_GRP, DELAY_CONF, DEFAULT_DELAY);
    }
    void setDelay(int delay) {
        setInteger(MAIN_GRP, DELAY_CONF, delay);
    }
    bool isCaptureWindow() {
        return getBoolean(MAIN_GRP, CAPTURE_WINDOW_CONF, true);
    }
    void setCaptureWindow(bool captureWindow) {
        setBoolean(MAIN_GRP, CAPTURE_WINDOW_CONF, captureWindow);
    }
    Gdk::Color getBackgroundColor() {
        return m_background;
    }
    void setBackgroundColor(const Gdk::Color& _background) {
        m_background = _background;
    }
    Gdk::Color getForegroundColor() {
        return m_foreground;
    }
    void setForegroundColor(const Gdk::Color& foreground) {
        m_foreground = foreground;
    }
    Glib::ustring getTextFont() {
        return m_textFont;
    }
    void setTextFont(const Glib::ustring& textFont) {
        m_textFont = textFont;
    }
    std::string getSanePath() {
        auto path = getString(SCAN_GRP, SANE_PATH_CONF, "");
        if (path.empty()) {
            path = "/usr/lib/sane";
            setString(SCAN_GRP, SANE_PATH_CONF, path);
            saveConfig();   // save to give some hint
        }
        return path;
    }
    Glib::ustring getScanMode() {
        return getString(SCAN_GRP, SCAN_MODE_CONF);
    }
    void setScanMode(const Glib::ustring& mode) {
        setString(SCAN_GRP, SCAN_MODE_CONF, mode);
    }
    int getScanBrightness() {
        return getInteger(SCAN_GRP, SCAN_BRIGHTNESS_CONF, 0);
    }
    void setScanBrightness(int bright) {
        setInteger(SCAN_GRP, SCAN_BRIGHTNESS_CONF, bright);
    }
    int getScanContrast() {
        return getInteger(SCAN_GRP, SCAN_CONTRAST_CONF, 0);
    }
    void setScanContrast(int contrast) {
        setInteger(SCAN_GRP, SCAN_CONTRAST_CONF, contrast);
    }
    void saveConfig() override;
    void loadConfig() override;
protected:
    static constexpr auto DEFAULT_DELAY{5};
private:

    Gdk::Color m_background;
    Gdk::Color m_foreground;
    Glib::ustring m_textFont{"Sans 10"};

    static constexpr auto MAIN_GRP{"main"};
    static constexpr auto SCAN_GRP{"scan"};
    static constexpr auto DELAY_CONF{"delaySec"};
    static constexpr auto CAPTURE_WINDOW_CONF{"captureWindow"};
    static constexpr auto BACKGROUNDCOLOR_CONF{"backgroudColor"};
    static constexpr auto FOREGROUNDCOLOR_CONF{"foregroudColor"};
    static constexpr auto TEXTFONT_CONF{"textFont"};
    static constexpr auto SANE_PATH_CONF{"sanePath"};
    static constexpr auto SCAN_MODE_CONF{"mode"};
    static constexpr auto SCAN_BRIGHTNESS_CONF{"brightness"};
    static constexpr auto SCAN_CONTRAST_CONF{"contrast"};
};


