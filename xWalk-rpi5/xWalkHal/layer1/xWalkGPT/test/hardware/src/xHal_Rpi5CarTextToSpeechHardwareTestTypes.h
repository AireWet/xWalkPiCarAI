/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechHardwareTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xHal_Rpi5CarTextToSpeechHardwareTest.cpp.
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

#ifndef XHAL_RPI5CARTEXTTOSPEECHHARDWARETESTTYPES_H
#define XHAL_RPI5CARTEXTTOSPEECHHARDWARETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTextToSpeechAlsa.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xhal_rpi5cartexttospeechhardwaretest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xhal_rpi5cartexttospeechhardwaretest
{

    using namespace xwalk::hal;

    /** @brief Retains the deployment-owned raw PCM fixture path. */
    struct FixtureProvider
    {
            /** @brief Existing raw signed sixteen-bit little-endian PCM file path. */
            XWalkHal::filesystempath filePath{};
    };

} /* namespace xwalk::source_types::xhal_rpi5cartexttospeechhardwaretest */

#endif /* XHAL_RPI5CARTEXTTOSPEECHHARDWARETESTTYPES_H */
