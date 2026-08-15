/******************************************************************************
 * @file        xHal_Rpi5CarMusicSimulation.h
 * @brief       Declares the silent device-free xWalkMusic simulation.
 * @project     xWalk Firmware
 * @module      xWalkMusic Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_MUSIC_SIMULATION_H
#define XHAL_RPI5CAR_MUSIC_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    /** @brief Exercises Music without files, ALSA, or physical playback. */
    int32 runMusicSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_MUSIC_SIMULATION_H */
