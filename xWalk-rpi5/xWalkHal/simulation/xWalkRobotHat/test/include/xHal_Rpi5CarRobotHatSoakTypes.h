/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSoakTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarRobotHatSoak.cpp.
 *
 * @project     xWalk Firmware
 * @module      Source Type Support
 *
 * @author      Joxy John
 * @date        2026-08-15
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CARROBOTHATSOAKTYPES_H
#define XHAL_RPI5CARROBOTHATSOAKTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLogicalModels.h"
#include <dirent.h>
#include <unistd.h>
#include <array>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carrobothatsoak
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carrobothatsoak
{

    using namespace xwalk::hal;

    struct Options
    {
            uint64 seed{42U};
            uint64 iterations{10'000U};
            uint64 logicalDuration{20'000U};
            float64 faultRate{0.05};
            string report{"xwalk-robot-hat-soak-report.json"};
    };

    struct ProcessSnapshot
    {
            uint64 residentBytes{};
            uint64 descriptors{};
            uint64 threads{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carrobothatsoak */

#endif /* XHAL_RPI5CARROBOTHATSOAKTYPES_H */
