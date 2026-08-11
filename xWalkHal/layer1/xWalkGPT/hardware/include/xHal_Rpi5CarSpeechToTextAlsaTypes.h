/******************************************************************************
 * @file        xHal_Rpi5CarSpeechToTextAlsaTypes.h
 * @brief       Declares microphone and recognition backend operations.
 *
 * @details
 * Defines opaque ALSA capture ownership plus injected recognition operations
 * used by the Linux speech-to-text adapter and device-free host tests.
 *
 * @project     xWalk Firmware
 * @module      xWalkGPT Speech-to-Text ALSA Backend
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

#ifndef XHAL_RPI5CAR_SPEECH_TO_TEXT_ALSA_TYPES_H
#define XHAL_RPI5CAR_SPEECH_TO_TEXT_ALSA_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechToTextTypes.h"

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
 * Type definitions
 ******************************************************************************/

/**
 * @brief Opaque ALSA capture handle owned by the speech adapter.
 *
 * @details A successful open operation returns a non-null handle that remains
 * owned until the corresponding close operation is called exactly once.
 */
using speechcapturehandle = contextpointer;

/**
 * @brief Opens and configures one interleaved signed-16 capture stream.
 *
 * @param[in,out] context Nullable non-owning backend context.
 * @param[in] deviceName Non-empty capture device name valid for the operation.
 * @param[in] sampleRateHz Required sample rate in Hertz.
 * @param[in] channelCount Required channel count.
 * @param[in] periodFrames Maximum frames requested by one read.
 * @return Newly owned non-null capture handle, or null when opening fails.
 */
using speechcaptureopencallback = speechcapturehandle (*)(contextpointer, stringview,
    uint32, uint8, uint32);

/**
 * @brief Reads up to one bounded period and replaces the supplied PCM bytes.
 *
 * @param[in,out] context Nullable non-owning backend context.
 * @param[in,out] captureHandle Non-null open handle owned by the adapter.
 * @param[out] pcmData Interleaved signed-16 PCM for the completed frames.
 * @param[in] frameCount Requested positive frame count no greater than 1,024.
 * @return Positive completed frames, zero, or a negative recoverable error value.
 */
using speechcapturereadcallback = int32 (*)(contextpointer, speechcapturehandle,
    bytevector&, size);

/**
 * @brief Attempts recovery after one negative capture result.
 *
 * @param[in,out] context Nullable non-owning backend context.
 * @param[in,out] captureHandle Non-null open handle owned by the adapter.
 * @param[in] errorValue Negative result returned by the read operation.
 * @return `true` when capture may continue; otherwise `false`.
 */
using speechcapturerecovercallback = boolean (*)(contextpointer, speechcapturehandle, int32);

/**
 * @brief Closes one owned capture handle without throwing.
 *
 * @param[in,out] context Nullable non-owning backend context.
 * @param[in,out] captureHandle Non-null open handle released by this call.
 */
using speechcaptureclosecallback = void (*)(contextpointer, speechcapturehandle);

/**
 * @brief Reports whether the injected recognizer can accept requests.
 *
 * @param[in,out] context Nullable non-owning recognizer context.
 * @return `true` when recognition requests can be accepted; otherwise `false`.
 */
using speechrecognizerreadycallback = boolean (*)(contextpointer);

/**
 * @brief Recognizes one bounded interleaved signed-16 PCM buffer.
 *
 * @param[in,out] context Nullable non-owning recognizer context.
 * @param[in] pcmData Complete PCM bytes retained only for the duration of the call.
 * @param[in] sampleRateHz PCM sample rate in Hertz.
 * @param[in] channelCount PCM channel count.
 * @return Owned transcript, which may be empty for silence or unrecognized speech.
 */
using speechrecognizerpcmcallback = string (*)(contextpointer, const bytevector&, uint32, uint8);

/**
 * @brief Transcribes one recognizer-supported audio file.
 *
 * @param[in,out] context Nullable non-owning recognizer context.
 * @param[in] filePath Non-empty path view retained only for the duration of the call.
 * @return Owned transcript, which may be empty for silence or unrecognized speech.
 */
using speechrecognizerfilecallback = string (*)(contextpointer, stringview);

/**
 * @brief Requests recognition cancellation without throwing.
 *
 * @param[in,out] context Nullable non-owning recognizer context.
 *
 * @note Repeated and idle cancellation requests must be tolerated.
 */
using speechrecognizercancelcallback = void (*)(contextpointer);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkSpeechToTextAlsaOperations
 * @brief Contains complete injectable microphone and recognizer operations.
 *
 * @details Every callback must be non-null before adapter construction. The
 * adapter copies the table but does not own the context used by the callbacks.
 */
struct XWalkSpeechToTextAlsaOperations
{
    /** @brief Opens and configures one bounded capture handle. */
    speechcaptureopencallback openCapture{nullptr};

    /** @brief Reads one bounded PCM capture period. */
    speechcapturereadcallback readCapture{nullptr};

    /** @brief Attempts recovery after a negative capture result. */
    speechcapturerecovercallback recoverCapture{nullptr};

    /** @brief Closes one capture handle without throwing. */
    speechcaptureclosecallback closeCapture{nullptr};

    /** @brief Reports readiness of the selected recognizer. */
    speechrecognizerreadycallback recognizerReady{nullptr};

    /** @brief Recognizes the complete bounded PCM capture. */
    speechrecognizerpcmcallback recognizePcm{nullptr};

    /** @brief Transcribes one application-selected audio file. */
    speechrecognizerfilecallback recognizeFile{nullptr};

    /** @brief Requests recognizer cancellation without throwing. */
    speechrecognizercancelcallback cancelRecognition{nullptr};
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_TO_TEXT_ALSA_TYPES_H */
