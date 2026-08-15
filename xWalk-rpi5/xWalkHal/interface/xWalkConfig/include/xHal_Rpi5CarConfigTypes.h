/******************************************************************************
 * @file        xHal_Rpi5CarConfigTypes.h
 * @brief       Declares section-aware configuration value types.
 *
 * @details
 * Provides ordered containers used to represent options within named sections
 * of an xWalk configuration file.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig
 *
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_CONFIG_TYPES_H
#define XHAL_RPI5CAR_CONFIG_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Ordered option names and owned string values belonging to one section. */
    using configsection = orderedmap<string, string>;

    /**
     * @brief Ordered named configuration sections.
     *
     * @note
     * The empty section name represents options that appear before a named section.
     */
    using configsections = orderedmap<string, configsection>;

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CONFIG_TYPES_H */
