/******************************************************************************
 * @file        xHal_Rpi5CarTextToSpeechLifecycle.cpp
 * @brief       Implements text-to-speech validation and lifecycle behavior.
 *
 * @details
 * Binds caller-owned board and speech-backend dependencies, validates the
 * callback, and activates Robot HAT speaker power during construction.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT
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

#include "xHal_Rpi5CarTextToSpeech.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Validates the injected speech backend before speaker activation.
     *
     * @param[in] callback
     * Speech callback that must be non-null.
     *
     * @throws std::invalid_argument
     * If `callback` is null.
     */
    void XWalkTextToSpeech::validateBackend(texttospeechspeakcallback callback)
    {
        if (callback == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Text-to-speech callback must not be null");
        }
    }

    /**
     * @brief Activates and primes Robot HAT speaker output.
     *
     * @post
     * Speaker power remains active after successful completion.
     *
     * @note
     * Exceptions from board control are propagated and construction fails.
     */
    void XWalkTextToSpeech::prepareSpeaker()
    {
        boardControlPointer->enableSpeaker();
    }

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs a text-to-speech coordinator and activates the speaker.
     *
     * @param[in,out] boardControl
     * Caller-created Robot HAT controller that must outlive this object.
     *
     * @param[in,out] backendContext
     * Nullable non-owning speech-backend context. A non-null object must outlive
     * this object, and null requires explicit callback support.
     *
     * @param[in] backendSpeak
     * Non-null synchronous callback that accepts speech text.
     *
     * @post
     * Robot HAT speaker power has been enabled and primed.
     *
     * @throws std::invalid_argument
     * If `backendSpeak` is null.
     *
     * @note
     * Exceptions from speaker activation or priming are propagated.
     */
    XWalkTextToSpeech::XWalkTextToSpeech(XWalkBoardControl& boardControl,
                                         contextpointer backendContext,
                                         texttospeechspeakcallback backendSpeak)
        : boardControlPointer(&boardControl), backendContextPointer(backendContext), speakCallback(backendSpeak)
    {
        validateBackend(speakCallback);
        prepareSpeaker();
        XWALK_HAL_TRACE_UID0(RPI .361, "Text-to-speech coordinator constructed with speaker enabled");
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the coordinator without disabling shared speaker power.
     *
     * @details
     * This preserves the Python wrapper behavior and avoids changing a board-level
     * resource that may be shared with another audio component.
     */
    XWalkTextToSpeech::~XWalkTextToSpeech() = default;

} /* namespace xwalk::hal */
