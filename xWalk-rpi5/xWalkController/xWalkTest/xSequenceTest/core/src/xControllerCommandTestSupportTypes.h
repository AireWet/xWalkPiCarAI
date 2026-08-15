/******************************************************************************
 * @file        xControllerCommandTestSupportTypes.h
 * @brief       Declares translation-unit support types.
 *
 * @details
 * Owns class and structure declarations used by xControllerCommandTestSupport.cpp.
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

#ifndef XCONTROLLERCOMMANDTESTSUPPORTTYPES_H
#define XCONTROLLERCOMMANDTESTSUPPORTTYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xAgent_Rpi5CarGptCar.h"
#include "xAgent_Rpi5CarVoiceActiveCarGpt.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarGrayscaleModule.h"
#include "xHal_Rpi5CarI2c.h"
#include "xHal_Rpi5CarLanguageModel.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarMotor.h"
#include "xHal_Rpi5CarMusic.h"
#include "xHal_Rpi5CarPwm.h"
#include "xHal_Rpi5CarPwmTimerState.h"
#include "xHal_Rpi5CarServo.h"
#include "xHal_Rpi5CarSpeechToText.h"
#include "xHal_Rpi5CarSpi.h"
#include "xHal_Rpi5CarTextToSpeech.h"
#include "xHal_Rpi5CarUltrasonic.h"
#include "xHal_Rpi5CarVoiceAssistant.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::source_types::xcontrollercommandtestsupport
 * @brief Contains translation-unit support types for one implementation.
 */
namespace xwalk::source_types::xcontrollercommandtestsupport
{

    using namespace xwalk::hal;
    using namespace xwalk::controller;

    struct TestBus
    {
            ::ctrl::bytevector sample{0x03U, 0xE8U};
            xwalk::agent::test::ControllerCommandTestState* state{nullptr};
    };

    struct TestGpio
    {
            ::ctrl::boolean value{};
    };

    struct CallbackContext
    {
            xwalk::agent::test::ControllerCommandTestState* state{nullptr};
            xwalk::hal::XWalkMotors* motors{nullptr};
            xwalk::agent::XWalkPicarx* picarx{nullptr};
    };

} /* namespace xwalk::source_types::xcontrollercommandtestsupport */

#endif /* XCONTROLLERCOMMANDTESTSUPPORTTYPES_H */
