/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelSimulation.h
 * @brief       Declares the network-free language-model simulation.
 * @details     Exercises coordinator behavior through an in-memory backend.
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_LANGUAGE_MODEL_SIMULATION_H
#define XHAL_RPI5CAR_LANGUAGE_MODEL_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
    int32 runLanguageModelSimulation();
}
#endif
