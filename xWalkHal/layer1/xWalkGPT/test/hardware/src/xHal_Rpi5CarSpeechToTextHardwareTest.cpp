/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextHardwareTest.cpp
 * @brief       Compile-checks speech-to-text target integration.
 *
 * @details
 * Confirms the public coordinator type is complete without constructing a
 * microphone or recognition backend. No physical operation is performed.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Hardware Test
 *
 * @author      Joxy John
 * @date        2026-07-30
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

#include "xHal_Rpi5CarSpeechToText.h"

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Confirms the speech-to-text type is complete for target compilation.
 *
 * @return
 * Zero when the complete type has non-zero object size.
 *
 * @note
 * No object is constructed, so no microphone or model is accessed.
 */
XWalkHal::int32 main()
{
    const XWalkHal::size objectSize = sizeof(XWalkHal::XWalkSpeechToText);
    return (objectSize > 0U) ? 0 : 1;
}
