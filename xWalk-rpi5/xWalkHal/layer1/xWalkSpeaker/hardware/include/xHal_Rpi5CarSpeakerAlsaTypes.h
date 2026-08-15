/******************************************************************************
 * @file        xHal_Rpi5CarSpeakerAlsaTypes.h
 * @brief       Declares the injectable speaker decoder operation table.
 *
 * @details
 * Defines the decoder seam used by the shared-ALSA Speaker adapter so optional
 * codec libraries remain isolated from the hardware-independent controller.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpeaker ALSA Adapter
 *
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SPEAKER_ALSA_TYPES_H
#define XHAL_RPI5CAR_SPEAKER_ALSA_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeakerTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkSpeakerAlsaOperations
     * @brief Contains the complete injectable audio-file decoder seam.
     */
    struct XWalkSpeakerAlsaOperations
    {
            /**
             * @brief Decodes one bounded audio file before a Speaker task starts.
             *
             * @details
             * Optional implementations may support FLAC, OGG, MP3, M4A, AAC, or WMA,
             * but must enforce the shared maximum decoded sample count.
             */
            speakeraudiodecodecallback decodeAudio{nullptr};
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEAKER_ALSA_TYPES_H */
