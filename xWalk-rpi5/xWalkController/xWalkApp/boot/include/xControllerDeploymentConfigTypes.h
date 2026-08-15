/******************************************************************************
 * @file        xControllerDeploymentConfigTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xControllerDeploymentConfig.cpp.
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

#ifndef XCONTROLLERDEPLOYMENTCONFIGTYPES_H
#define XCONTROLLERDEPLOYMENTCONFIGTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerDeploymentConfig.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include <array>
#include <charconv>
#include <cmath>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xcontrollerdeploymentconfig
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xcontrollerdeploymentconfig
{

    using namespace xwalk::hal;
    using namespace xwalk::controller;

    struct ConfigDefault
    {
            ::ctrl::cstring name;
            ::ctrl::cstring value;
    };

} /* namespace xwalk::source_types::xcontrollerdeploymentconfig */

#endif /* XCONTROLLERDEPLOYMENTCONFIGTYPES_H */
