/******************************************************************************
 * @file        xHal_Rpi5CarSpeechRecognizerVosk.h
 * @brief       Declares the dynamically loaded offline Vosk provider.
 *
 * @details
 * Owns one Vosk shared-library handle and model while exposing recognition
 * operations compatible with the existing ALSA speech-capture adapter.
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

#ifndef XHAL_RPI5CAR_SPEECH_RECOGNIZER_VOSK_H
#define XHAL_RPI5CAR_SPEECH_RECOGNIZER_VOSK_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechRecognizerVoskTypes.h"

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
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkSpeechRecognizerVosk
 * @brief Provides offline Vosk recognition to `XWalkSpeechToTextAlsa`.
 *
 * @details
 * Loads the Vosk C API at runtime, owns one model, and creates a recognizer for
 * each synchronous PCM request. Public calls require external serialization.
 */
class XWalkSpeechRecognizerVosk final
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Owned dynamic-library handle closed during destruction. */
        contextpointer libraryHandle{nullptr};
        /** @brief Complete resolved Vosk function table. */
        XWalkVoskApi api{};
        /** @brief Owned Vosk model released before the library. */
        voskmodelhandle modelHandle{nullptr};
        /** @brief Atomic cancellation request checked around recognition. */
        atomicboolean cancellationRequested{false};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /** @brief Converts a callback context into its required provider object. */
        static XWalkSpeechRecognizerVosk& provider(contextpointer context);
        /** @brief Resolves one required function symbol from the loaded library. */
        template<typename FunctionType>
        static FunctionType loadFunction(contextpointer library, cstring symbolName);
        /** @brief Reports whether the model remains available for recognition. */
        static boolean ready(contextpointer context);
        /** @brief Recognizes one complete signed sixteen-bit PCM payload. */
        static string recognizePcm(contextpointer context, const bytevector& pcmData,
            uint32 sampleRateHz, uint8 channelCount);
        /** @brief Rejects unsupported direct file transcription explicitly. */
        static string recognizeFile(contextpointer context, stringview filePath);
        /** @brief Requests cancellation of the next or current recognition boundary. */
        static void cancel(contextpointer context) noexcept;
        /** @brief Extracts and unescapes the `text` value from a Vosk JSON result. */
        static string extractText(stringview jsonResult);

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Loads a Vosk library and model selected by deployment configuration.
         * @param[in] libraryName Non-empty shared-library name or path.
         * @param[in] modelPath Non-empty readable Vosk model directory.
         * @throws std::invalid_argument If either configured value is empty.
         * @throws std::runtime_error If the library, API, or model cannot be loaded.
         */
        XWalkSpeechRecognizerVosk(stringview libraryName, stringview modelPath);
        /** @brief Releases the model and then closes the Vosk shared library. */
        ~XWalkSpeechRecognizerVosk();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        XWalkSpeechRecognizerVosk(const XWalkSpeechRecognizerVosk&) = delete;
        XWalkSpeechRecognizerVosk& operator=(const XWalkSpeechRecognizerVosk&) = delete;
        XWalkSpeechRecognizerVosk(XWalkSpeechRecognizerVosk&&) = delete;
        XWalkSpeechRecognizerVosk& operator=(XWalkSpeechRecognizerVosk&&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /** @brief Returns recognizer operations requiring this object as context. */
        XWalkSpeechToTextAlsaOperations operations() const noexcept;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_SPEECH_RECOGNIZER_VOSK_H */
