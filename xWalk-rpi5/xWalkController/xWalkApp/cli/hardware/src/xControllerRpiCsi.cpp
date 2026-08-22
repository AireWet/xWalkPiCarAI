/******************************************************************************
 * @file        xControllerRpiCsi.cpp
 * @brief       Implements CSI/GStreamer Raspberry Pi process initialization.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerRpi.h"

#include <cstdlib>
#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::ctrl
{

    ::ctrl::int32 XWALK_initRpi() noexcept
    {
        ::ctrl::string pluginPath{XWALK_GSTREAMER_PLUGIN_DIRECTORY};
        const auto* currentPath = ::getenv("GST_PLUGIN_PATH_1_0");
        if ((currentPath != nullptr) && (currentPath[0] != '\0'))
        {
            pluginPath += ":";
            pluginPath += currentPath;
        }
        const ::ctrl::int32 environmentResult = ::setenv("GST_PLUGIN_PATH_1_0", pluginPath.c_str(), 1);
        if (environmentResult != 0)
        {
            std::cerr << "Could not configure the GStreamer plugin search path" << std::endl;
            return 2;
        }
        return 0;
    }

} /* namespace xwalk::ctrl */
