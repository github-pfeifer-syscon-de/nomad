/* -*- Mode: c++; c-basic-offset: 4; tab-width: 4; coding: utf-8; -*-  */
/*
 * Copyright (C) 2026 RPf
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
#include <memory>
#include <vector>
#include <psc_format.hpp>
#include <sane/sane.h>

#include "nomad_config.h"   // use common configured
#include "SaneScanDevice.hpp"




static bool
check_sane()
{
    auto initStatus = SaneScanDevice::load_sanelib("/usr/lib/sane");
    if (initStatus != SANE_STATUS_GOOD) {
        std::cout << "Error sane init status " << initStatus << std::endl;
        return false;
    }
    std::vector<std::shared_ptr<SaneScanDevice>> devices = SaneScanDevice::devices();;
    for (auto& device : devices) {
        std::cout << "dev " << device->getName() << std::endl;
        for (auto& opt : device->getOptions()) {
            std::cout << "  opt " << opt->getName()
                      << " = " << opt->getInfo() << std::endl;
        }
        SANE_Parameters parameter = device->getParameters();
        std::cout << "param format " << parameter.format
                  << " last " <<  std::boolalpha << parameter.last_frame
                  << " line_bytes " << parameter.bytes_per_line
                  << " line_pixels " << parameter.pixels_per_line
                  << " lines " << parameter.lines
                  << " depth " << parameter.depth << std::endl;
        device->close();
    }
    devices.clear();    // ensure all closed devices
    SaneScanDevice::unload_sanelib();
    return true;
}

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");      // make locale dependent, and make glib accept u8 const !!!

    if (!check_sane()) {
        return 1;
    }
    return 0;
}