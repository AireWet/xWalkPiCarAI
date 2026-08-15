/******************************************************************************
 * @file        xHal_Rpi5CarAudioSimulationHardware.cpp
 * @brief       Composes the physical ALSA Audio simulation backend.
 *
 * @details
 * Selects real libasound operations for the explicitly requested hardware
 * build while reusing the bounded standalone Audio handler.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Hardware Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioSimulation.h"

#include "xHal_Rpi5CarAudioAlsa.h"
#include "xHal_Rpi5CarAudioHandler.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim
{

    int32 runAudioSimulation()
    {
        XWALK_HAL_TRACE_UID0(RPI .095, "Creating physical ALSA Audio simulation backend");
        XWalkAudioAlsa audio;
        XWalkAudioHandler handler;
        return handler.run(audio);
    }

} /* namespace xwalk::hal::sim */
