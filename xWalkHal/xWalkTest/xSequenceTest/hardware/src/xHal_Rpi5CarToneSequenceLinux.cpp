/******************************************************************************
 * @file        xHal_Rpi5CarToneSequenceLinux.cpp
 * @brief       Implements Linux composition for the Robot HAT tone sequence.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
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

#include "xHal_Rpi5CarToneSequenceLinux.h"

#include "xHal_Rpi5CarMusicAlsa.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

/**
 * @brief Composes and plays the melody through ALSA.
 *
 * @param[in] pcmDevice ALSA PCM playback device name.
 * @param[in] mixerDevice ALSA mixer device name.
 * @param[in] mixerElement ALSA mixer element name.
 */
void XWalkToneSequenceLinux::run(stringview pcmDevice,
    stringview mixerDevice, stringview mixerElement)
{
    XWalkAudioAlsa audio(pcmDevice, mixerDevice, mixerElement);
    XWalkMusicAlsa adapter(audio);
    XWalkMusic music(&adapter, adapter.callbacks());
    XWalkToneSequence sequence(music, this, &XWalkToneSequenceLinux::reportMeasure);
    sequence.run();
}

/**
 * @brief Prints the source-compatible measure heading.
 *
 * @param[in,out] context Unused callback context.
 * @param[in] measureNumber One-based measure number.
 */
void XWalkToneSequenceLinux::reportMeasure(
    contextpointer context, uint8 measureNumber)
{
    static_cast<void>(context);
    std::cout << "Measure " << static_cast<uint32>(measureNumber) << '\n';
}

} /* namespace xwalk::hal::test */
