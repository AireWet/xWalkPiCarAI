/******************************************************************************
 * @file        xAgent_Rpi5CarSpiTransferTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarSpiTransferTest.cpp.
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

#ifndef XAGENT_RPI5CARSPITRANSFERTESTTYPES_H
#define XAGENT_RPI5CARSPITRANSFERTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarSpiTransfer.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carspitransfertest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carspitransfertest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    /** @brief Records one transmitted payload. */
    struct TestBackend
    {
            agent::bytevector request;
    };

} /* namespace xwalk::source_types::xagent_rpi5carspitransfertest */

#endif /* XAGENT_RPI5CARSPITRANSFERTESTTYPES_H */
