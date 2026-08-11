/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerSimulation.h
 * @brief       Declares the silent device-free xWalkSpeaker simulation.
 * @project     xWalk Firmware
 * @module      xWalkSpeaker Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_SPEAKER_SIMULATION_H
#define XHAL_RPI5CAR_SPEAKER_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises Speaker without decoding files or opening audio devices. */
int32 runSpeakerSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_SPEAKER_SIMULATION_H */
