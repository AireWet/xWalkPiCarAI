/******************************************************************************
 * @file        xControllerAppConfig.h
 * @brief       Defines standalone Controller application path defaults.
 *
 * @details
 * Supplies safe translation-unit and editor defaults for deployment paths that
 * CMake replaces with absolute target-specific values in supported builds.
 *
 * @project     xWalk Firmware
 * @module      xWalkApp
 *
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XCONTROLLER_APP_CONFIG_H
#define XCONTROLLER_APP_CONFIG_H

/******************************************************************************
 * Preprocessor constants
 ******************************************************************************/

/**
 * @brief Default installed resource directory for non-CMake source analysis.
 *
 * @details
 * Supported CMake targets define this macro as their configured absolute data
 * directory. This fallback keeps standalone compilation and editor parsing
 * valid without overriding a build-supplied deployment path.
 */
#ifndef XWALK_RUNTIME_DATA_DIRECTORY
#define XWALK_RUNTIME_DATA_DIRECTORY "/usr/local/share/xwalk"
#endif

#endif /* XCONTROLLER_APP_CONFIG_H */
