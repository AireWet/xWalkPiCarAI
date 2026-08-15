/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiGptCar.cpp
 * @brief       Selects the Raspberry Pi GPT-car profile.
 * @details     Delegates to the shared configured voice-active composition.
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"

namespace xwalk::agent
{

    /**
     * @brief Runs the configured GPT-car profile.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @param[in,out] boardControl Caller-owned board controller.
     * @param[in,out] picarx Caller-owned PiCar-X coordinator.
     * @param[in] gpioDevice Configured GPIO device path.
     * @param[in] gpioChipName Optional exact GPIO chip name.
     * @param[in] gpioChipLabel Optional exact GPIO chip label.
     * @param[in] minimumGpioLineCount Required minimum GPIO line count.
     * @param[in] gpioCallbacks Linux GPIO callback table.
     * @return Status returned by `callback`.
     */
    agent::int32 XWalkBootRpi::runGptCar(agent::contextpointer context,
                                         bootapplicationcallback callback,
                                         hal::XWalkConfigStore& config,
                                         hal::XWalkBoardControl& boardControl,
                                         XWalkPicarx& picarx,
                                         agent::stringview gpioDevice,
                                         agent::stringview gpioChipName,
                                         agent::stringview gpioChipLabel,
                                         agent::uint32 minimumGpioLineCount,
                                         const hal::XWalkGpioCallbacks& gpioCallbacks)
    {
        return runVoiceActiveMode(XWALK_BOOT_GPT_CAR_REQ,
                                  context,
                                  callback,
                                  config,
                                  boardControl,
                                  picarx,
                                  gpioDevice,
                                  gpioChipName,
                                  gpioChipLabel,
                                  minimumGpioLineCount,
                                  gpioCallbacks);
    }

} /* namespace xwalk::agent */
