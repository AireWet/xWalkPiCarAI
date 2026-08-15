/******************************************************************************
 * @file        xHal_Rpi5CarTtsPiperExampleTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTtsPiperExampleTest.cpp.
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

#ifndef XHAL_RPI5CARTTSPIPEREXAMPLETESTTYPES_H
#define XHAL_RPI5CARTTSPIPEREXAMPLETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTtsPiperExample.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5carttspiperexampletest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5carttspiperexampletest
{

    using namespace xwalk::hal;

    /** @brief Records one deterministic voice model and speech message. */
    struct TtsPiperExampleState
    {
            /** @brief Owned voice model supplied to the speech callback. */
            XWalkHal::string model;

            /** @brief Owned message supplied to the speech callback. */
            XWalkHal::string text;

            /** @brief Number of observed synchronous speech requests. */
            XWalkHal::uint32 callCount{};
    };

} /* namespace xwalk::source_types::xhal_rpi5carttspiperexampletest */

#endif /* XHAL_RPI5CARTTSPIPEREXAMPLETESTTYPES_H */
