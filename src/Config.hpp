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

#pragma once

class Config
: public Glib::Object {
public:
    Config();
    explicit Config(const Config& cnf) = delete;
    virtual ~Config() = default;

    void save_config();
    int getDelay() {
        return m_delaySec;
    }
    void setDelay(int delay) {
        m_delaySec = delay;
    }
    bool isCaptureWindow() {
        return m_captureWindow;
    }
    void setCaptureWindow(bool captureWindow) {
        m_captureWindow = captureWindow;
    }

protected:
    Glib::ustring get_config_name();
    void read_config();
    static constexpr auto DEFAULT_DELAY{5};
private:
    int m_delaySec{DEFAULT_DELAY};
    bool m_captureWindow{true};
    Glib::KeyFile* m_config;
    //Glib::Property<Glib::ustring> property_delay_;
    //Glib::Property<bool> property_captWindow_;

    static constexpr auto MAIN_GRP{"main"};
    static constexpr auto DELAY_CONF{"deplaySec"};
    static constexpr auto CAPTURE_WINDOW_CONF{"captureWindow"};
    static constexpr auto CONF_FILE{"nomad.conf"};
};


