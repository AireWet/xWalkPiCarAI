/******************************************************************************
 * @file        xAgent_Rpi5CarVideoRecordingTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarVideoRecordingTest.cpp.
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

#ifndef XAGENT_RPI5CARVIDEORECORDINGTESTTYPES_H
#define XAGENT_RPI5CARVIDEORECORDINGTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarVideoRecording.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carvideorecordingtest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carvideorecordingtest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    struct TestState
    {
            agent::boolean cameraStarted{};
            agent::boolean recording{};
            agent::boolean paused{};
            agent::uint32 delayTotalMs{};
            agent::uint32 continueCount{};
            agent::uint32 continueLimit{1'000U};
            agent::string name{};
    };

} /* namespace xwalk::source_types::xagent_rpi5carvideorecordingtest */

#endif /* XAGENT_RPI5CARVIDEORECORDINGTESTTYPES_H */
