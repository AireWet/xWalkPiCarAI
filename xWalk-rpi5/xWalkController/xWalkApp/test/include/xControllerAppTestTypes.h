/******************************************************************************
 * @file        xControllerAppTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xControllerAppTest.cpp.
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

#ifndef XCONTROLLERAPPTESTTYPES_H
#define XCONTROLLERAPPTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerCommand.h"
#include "xWalkControllerConfigTypes.h"
#include "xControllerApplicationSupport.h"
#include "xControllerDeploymentConfig.h"
#include "xControllerBootMode.h"
#include "xControllerCommands.h"
#include "xControllerPicarxCommands.h"
#include "xControllerRunner.h"
#include "xHal_Rpi5CarTypes.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xcontrollerapptest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xcontrollerapptest
{

    using namespace xwalk::hal;
    using namespace xwalk::controller;

    struct BootModeTestCase
    {
            ::ctrl::cstring command;
            xwalk::agent::uint8 mode;
    };

    struct CommandTestCase
    {
            ::ctrl::cstring name;
            ::ctrl::uint16 command;
    };

    struct InvalidOverride
    {
            ::ctrl::cstring name;
            ::ctrl::cstring value;
    };

    struct ValidOverride
    {
            ::ctrl::cstring name;
            ::ctrl::cstring value;
    };

} /* namespace xwalk::source_types::xcontrollerapptest */

#endif /* XCONTROLLERAPPTESTTYPES_H */
