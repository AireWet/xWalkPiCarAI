/******************************************************************************
 * @file        xHal_Rpi5CarVoskRecognizerGuard.h
 * @brief       Declares scope-bound Vosk recognizer cleanup.
 *
 * @details
 * Releases one caller-created recognizer during every stack-cleanup path
 * without using exception-handling statements.
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

#ifndef XHAL_RPI5CAR_VOSK_RECOGNIZER_GUARD_H
#define XHAL_RPI5CAR_VOSK_RECOGNIZER_GUARD_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechRecognizerVoskTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkVoskRecognizerGuard
 * @brief Releases one non-owning recognizer binding at scope exit.
 *
 * @details
 * Stores a recognizer handle and its matching Vosk release operation. The
 * caller retains ownership until construction and transfers cleanup
 * responsibility to this guard for the remainder of the scope.
 */
class XWalkVoskRecognizerGuard final
{
    private:
        /** @brief Non-null recognizer handle released during destruction. */
        voskrecognizerhandle recognizerHandle{nullptr};
        /** @brief Non-null Vosk release operation. */
        voskrecognizerfreefunction freeFunction{nullptr};

    public:
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
        XWalkVoskRecognizerGuard(voskrecognizerhandle recognizer,
            voskrecognizerfreefunction release) noexcept;

        /**
         * @brief Releases the bound recognizer.
         *
         * @post
         * The recognizer has been passed exactly once to the release operation.
         */
        ~XWalkVoskRecognizerGuard() noexcept;

        /** @brief Disables copying because the guard owns one cleanup obligation. */
        XWalkVoskRecognizerGuard(const XWalkVoskRecognizerGuard&) = delete;
        /** @brief Disables copy assignment because the guard owns one cleanup obligation. */
        XWalkVoskRecognizerGuard& operator=(const XWalkVoskRecognizerGuard&) = delete;
        /** @brief Disables moving to keep the cleanup binding at one stable scope. */
        XWalkVoskRecognizerGuard(XWalkVoskRecognizerGuard&&) = delete;
        /** @brief Disables move assignment to keep the cleanup binding stable. */
        XWalkVoskRecognizerGuard& operator=(XWalkVoskRecognizerGuard&&) = delete;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_VOSK_RECOGNIZER_GUARD_H */
