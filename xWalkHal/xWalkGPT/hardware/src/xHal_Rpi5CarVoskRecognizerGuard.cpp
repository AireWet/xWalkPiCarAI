/******************************************************************************
 * @file        xHal_Rpi5CarVoskRecognizerGuard.cpp
 * @brief       Implements scope-bound Vosk recognizer cleanup.
 *
 * @details
 * Stores one recognizer handle and releases it through its Vosk C API function.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Vosk Provider
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xHal_Rpi5CarVoskRecognizerGuard.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for xWalk firmware.
 */
namespace xwalk::hal
{

/**
 * @brief Binds one recognizer handle and its release operation.
 *
 * @param[in] recognizer
 * Non-null Vosk recognizer handle to release.
 *
 * @param[in] release
 * Non-null Vosk release operation valid through destruction.
 *
 * @pre
 * Both arguments are non-null and refer to the same loaded Vosk API.
 */
XWalkVoskRecognizerGuard::XWalkVoskRecognizerGuard(voskrecognizerhandle recognizer,
    voskrecognizerfreefunction release) noexcept:
    recognizerHandle(recognizer), freeFunction(release)
{
}

/**
 * @brief Releases the bound recognizer.
 *
 * @post
 * The recognizer has been passed exactly once to the release operation.
 */
XWalkVoskRecognizerGuard::~XWalkVoskRecognizerGuard() noexcept
{
    freeFunction(recognizerHandle);
}

} /* namespace xwalk::hal */
