/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarComputerVisionTest.cpp.
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

#ifndef XAGENT_RPI5CARCOMPUTERVISIONTESTTYPES_H
#define XAGENT_RPI5CARCOMPUTERVISIONTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarComputerVision.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carcomputervisiontest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carcomputervisiontest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    /** @brief Records every observable provider operation. */
    struct TestState
    {
            agent::boolean started{};
            agent::uint32 stopCount{};
            xwalk::agent::XWalkComputerVisionColor color{xwalk::agent::XWalkComputerVisionColor::Close};
            agent::boolean faceEnabled{};
            agent::boolean qrEnabled{};
            agent::uint32 delayTotalMs{};
            agent::uint32 continueCount{};
            agent::uint32 continueLimit{1'000U};
    };

} /* namespace xwalk::source_types::xagent_rpi5carcomputervisiontest */

#endif /* XAGENT_RPI5CARCOMPUTERVISIONTESTTYPES_H */
