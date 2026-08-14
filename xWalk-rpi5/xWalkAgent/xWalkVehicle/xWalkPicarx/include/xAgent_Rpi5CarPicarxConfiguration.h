/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxConfiguration.h
 * @brief       Declares PiCar-X build-time configuration defaults.
 *
 * @details
 * Provides a source-visible fallback for the writable PiCar-X configuration
 * path. CMake overrides this value with an absolute deployment path for the
 * official application and hardware-test targets.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
 *
 * @author      Joxy John
 * @date        2026-07-31
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_PICARX_CONFIGURATION_H
#define XAGENT_RPI5CAR_PICARX_CONFIGURATION_H

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

/**
 * @brief Writable PiCar-X calibration configuration path.
 *
 * @details
 * The fallback resolves from the workspace root. Official CMake targets replace
 * it with the configured absolute `XWALK_PICARX_CONFIG_FILE` cache value.
 */
#ifndef XWALK_PICARX_CONFIG_FILE
#define XWALK_PICARX_CONFIG_FILE "xWalk-rpi5/xWalkController/xWalkConfig/picar-x.conf"
#endif

#endif /* XAGENT_RPI5CAR_PICARX_CONFIGURATION_H */
