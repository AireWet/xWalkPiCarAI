/******************************************************************************
 * @file        xHal_Rpi5CarTtsEspeakExample.h
 * @brief       Declares the configured Espeak text-to-speech example.
 *
 * @details
 * Defines the exact upstream amplitude, speed, gap, pitch, message, and one
 * injected synchronous speech operation without owning platform resources.
 *
 * @project     xWalk Firmware
 * @module      xExample
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TTS_ESPEAK_EXAMPLE_H
#define XHAL_RPI5CAR_TTS_ESPEAK_EXAMPLE_H

#include "xHal_Rpi5CarCommon.h"

namespace xwalk::hal::example
{

    /** @brief Exact Espeak amplitude selected by the source example. */
    inline constexpr uint8 XWALK_TTS_ESPEAK_EXAMPLE_AMPLITUDE{100U};
    /** @brief Exact Espeak word speed selected by the source example. */
    inline constexpr uint16 XWALK_TTS_ESPEAK_EXAMPLE_SPEED{150U};
    /** @brief Exact Espeak word gap selected by the source example. */
    inline constexpr uint16 XWALK_TTS_ESPEAK_EXAMPLE_GAP{1U};
    /** @brief Exact Espeak pitch selected by the source example. */
    inline constexpr uint8 XWALK_TTS_ESPEAK_EXAMPLE_PITCH{80U};
    /** @brief Exact speech message supplied by the source example. */
    inline constexpr stringview XWALK_TTS_ESPEAK_EXAMPLE_MESSAGE{"Hello world!"};

    /** @brief Synthesizes and plays one configured Espeak message. */
    using ttsespeakspeakcallback =
        void (*)(contextpointer context, uint8 amplitude, uint16 speed, uint16 gap, uint8 pitch, stringview text);

    /** @brief Delivers the exact upstream Espeak settings through one callback. */
    class XWalkTtsEspeakExample final
    {
        private:
            /** @brief Non-owning context forwarded to the speech operation. */
            contextpointer callbackContext;
            /** @brief Non-null synchronous speech operation. */
            ttsespeakspeakcallback speakCallback;

        public:
            /**
             * @brief Binds one configured synchronous Espeak operation.
             * @param[in,out] context Non-owning context forwarded to `speak`.
             * @param[in] speak Non-null operation accepting every source setting.
             * @throws std::invalid_argument If `speak` is null.
             */
            XWalkTtsEspeakExample(contextpointer context, ttsespeakspeakcallback speak);

            XWalkTtsEspeakExample(const XWalkTtsEspeakExample&) = delete;
            XWalkTtsEspeakExample(XWalkTtsEspeakExample&&) = delete;
            XWalkTtsEspeakExample& operator=(const XWalkTtsEspeakExample&) = delete;
            XWalkTtsEspeakExample& operator=(XWalkTtsEspeakExample&&) = delete;

            /** @brief Synthesizes and plays the exact upstream request once. */
            void run();
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_TTS_ESPEAK_EXAMPLE_H */
