/******************************************************************************
 * @file        xControllerSequenceTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xControllerSequenceTest.cpp.
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

#ifndef XCONTROLLERSEQUENCETESTTYPES_H
#define XCONTROLLERSEQUENCETESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerSequence.h"
#include "xHal_Rpi5CarTestFunctions.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xcontrollersequencetest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xcontrollersequencetest
{

    using namespace xwalk::hal;
    using namespace xwalk::controller;

    /** @brief Records every output line emitted by the simulated CLI backend. */
    struct TestState
    {
            /** @brief Output lines retained in exact callback order. */
            ::ctrl::stringvector outputLines;
    };

} /* namespace xwalk::source_types::xcontrollersequencetest */

#endif /* XCONTROLLERSEQUENCETESTTYPES_H */
