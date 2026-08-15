/******************************************************************************
 * @file        xHal_Rpi5CarAudioHandler.h
 * @brief       Declares the standalone Audio simulation handler.
 *
 * @details
 * Executes one bounded silent playback sequence through a caller-owned Audio
 * backend selected by the standalone simulation build.
 *
 * @project     xWalk Firmware
 * @module      xWalkAudio Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_AUDIO_HANDLER_H
#define XHAL_RPI5CAR_AUDIO_HANDLER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAudioAlsa.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::sim
{

    /**
     * @class XWalkAudioHandler
     * @brief Executes representative silent playback operations.
     */
    class XWalkAudioHandler final
    {
        public:
            /** @brief Constructs the stateless Audio simulation handler. */
            XWalkAudioHandler();

            /** @brief Destroys the stateless Audio simulation handler. */
            ~XWalkAudioHandler();

            XWalkAudioHandler(const XWalkAudioHandler&) = delete;
            XWalkAudioHandler& operator=(const XWalkAudioHandler&) = delete;
            XWalkAudioHandler(XWalkAudioHandler&&) = delete;
            XWalkAudioHandler& operator=(XWalkAudioHandler&&) = delete;

            /**
             * @brief Writes one silent period and applies a representative volume.
             * @param[in,out] audio Configured Audio backend that owns the stream.
             * @return Zero after every bounded operation completes.
             */
            int32 run(XWalkAudioAlsa& audio) const;
    };

} /* namespace xwalk::hal::sim */

#endif /* XHAL_RPI5CAR_AUDIO_HANDLER_H */
