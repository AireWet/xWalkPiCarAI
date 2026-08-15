/******************************************************************************
 * @file        xHal_Rpi5CarAudioSimulation.h
 * @brief       Declares build-selected Audio simulation composition.
 *
 * @details
 * Keeps the executable entry point independent of whether CMake selected the
 * device-free host stub or the opt-in physical ALSA backend.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
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

#ifndef XHAL_RPI5CAR_AUDIO_SIMULATION_H
#define XHAL_RPI5CAR_AUDIO_SIMULATION_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::sim
{

    /**
     * @brief Runs Audio operations through the backend selected by CMake.
     * @return Zero after the selected backend completes the bounded simulation.
     */
    int32 runAudioSimulation();

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_AUDIO_SIMULATION_H */
