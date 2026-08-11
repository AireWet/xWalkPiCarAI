/******************************************************************************
 * @file        xHal_Rpi5CarPwmSimulation.h
 * @brief       Declares the device-free xWalkPwm simulation.
 * @project     xWalk Firmware
 * @module      xWalkPwm Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_PWM_SIMULATION_H
#define XHAL_RPI5CAR_PWM_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises PWM timer and output operations through an in-memory bus. */
int32 runPwmSimulation();
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_PWM_SIMULATION_H */
