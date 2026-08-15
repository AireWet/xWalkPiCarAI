/******************************************************************************
 * @file        xAgent_Rpi5CarLocalVoiceChatbotTestTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xAgent_Rpi5CarLocalVoiceChatbotTest.cpp.
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

#ifndef XAGENT_RPI5CARLOCALVOICECHATBOTTESTTYPES_H
#define XAGENT_RPI5CARLOCALVOICECHATBOTTESTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarLocalVoiceChatbot.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarI2c.h"
#include <cassert>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xagent_rpi5carlocalvoicechatbottest
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xagent_rpi5carlocalvoicechatbottest
{

    using namespace xwalk::hal;
    using namespace xwalk::agent;

    struct TestState
    {
            stringvector outputLines{};
            stringvector spokenText{};
            uint32 listenCount{};
            uint32 continueCount{};
            uint32 delayCount{};
    };

} /* namespace xwalk::source_types::xagent_rpi5carlocalvoicechatbottest */

#endif /* XAGENT_RPI5CARLOCALVOICECHATBOTTESTTYPES_H */
