/******************************************************************************
 * @file        xHal_Rpi5CarToneSequenceLinux.h
 * @brief       Declares Linux composition for the Robot HAT tone sequence.
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

#ifndef XHAL_RPI5CAR_TONE_SEQUENCE_LINUX_H
#define XHAL_RPI5CAR_TONE_SEQUENCE_LINUX_H

#include "xHal_Rpi5CarToneSequence.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

    /** @brief Composes the tone sequence with the real Linux ALSA backend. */
    class XWalkToneSequenceLinux
    {
        protected:
            /** @brief Traces one measure heading before its first note. */
            static void reportMeasure(contextpointer context, uint8 measureNumber);

        public:
            /**
             * @brief Plays the complete melody through explicitly selected ALSA devices.
             *
             * @param[in] pcmDevice Non-empty ALSA PCM playback device name.
             * @param[in] mixerDevice Non-empty ALSA mixer device name.
             * @param[in] mixerElement Non-empty ALSA mixer element name.
             *
             * @warning Produces approximately 48 seconds of audio at 80-percent volume.
             */
            void run(stringview pcmDevice, stringview mixerDevice, stringview mixerElement);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_TONE_SEQUENCE_LINUX_H */
