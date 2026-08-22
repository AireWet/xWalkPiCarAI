/******************************************************************************
 * @file        xHal_Rpi5CarSpeechRecognizerVosk.cpp
 * @brief       Implements dynamically loaded offline Vosk recognition.
 *
 * @details
 * Resolves the required Vosk C API, owns one model, recognizes bounded ALSA
 * PCM buffers, and extracts final transcript text from provider JSON.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpeechRecognizerVosk.h"

#include "xHal_Rpi5CarVoskRecognizerGuard.h"

#include "xHal_Rpi5CarTrace.h"
#include <cstring>
#include <dlfcn.h>
#include <limits>

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
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Loads one Vosk library and model.
     * @param[in] libraryName Non-empty shared-library name or path.
     * @param[in] modelPath Non-empty Vosk model directory.
     * @throws std::invalid_argument If either configured value is empty.
     * @throws std::runtime_error If the library, API, or model cannot be loaded.
     */
    XWalkSpeechRecognizerVosk::XWalkSpeechRecognizerVosk(stringview libraryName, stringview modelPath)
    {
        const hal::boolean libraryNameModelPathInvalid =
            static_cast<hal::boolean>(libraryName.empty() || modelPath.empty());
        if (libraryNameModelPathInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Vosk library and model path are required");
        }
        const string ownedLibraryName(libraryName);
        const string ownedModelPath(modelPath);
        libraryHandle = ::dlopen(ownedLibraryName.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (libraryHandle == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk shared library could not be loaded");
        }
        api.modelNew = loadFunction<voskmodelnewfunction>(libraryHandle, "vosk_model_new");
        api.modelFree = loadFunction<voskmodelfreefunction>(libraryHandle, "vosk_model_free");
        api.recognizerNew = loadFunction<voskrecognizernewfunction>(libraryHandle, "vosk_recognizer_new");
        api.acceptWaveform = loadFunction<voskacceptwaveformfunction>(libraryHandle, "vosk_recognizer_accept_waveform");
        api.result = loadFunction<voskresultfunction>(libraryHandle, "vosk_recognizer_result");
        api.partialResult = loadFunction<voskpartialresultfunction>(libraryHandle, "vosk_recognizer_partial_result");
        api.finalResult = loadFunction<voskfinalresultfunction>(libraryHandle, "vosk_recognizer_final_result");
        api.recognizerFree = loadFunction<voskrecognizerfreefunction>(libraryHandle, "vosk_recognizer_free");
        if ((api.modelNew == nullptr) || (api.modelFree == nullptr) || (api.recognizerNew == nullptr) ||
            (api.acceptWaveform == nullptr) || (api.result == nullptr) || (api.partialResult == nullptr) ||
            (api.finalResult == nullptr) || (api.recognizerFree == nullptr))
        {
            static_cast<void>(::dlclose(libraryHandle));
            libraryHandle = nullptr;
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Required Vosk API symbol is unavailable");
        }
        modelHandle = api.modelNew(ownedModelPath.c_str());
        if (modelHandle == nullptr)
        {
            static_cast<void>(::dlclose(libraryHandle));
            libraryHandle = nullptr;
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk model could not be loaded");
        }
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /** @brief Releases the model and then closes the Vosk shared library. */
    XWalkSpeechRecognizerVosk::~XWalkSpeechRecognizerVosk()
    {
        cancellationRequested.store(true);
        if ((modelHandle != nullptr) && (api.modelFree != nullptr))
        {
            api.modelFree(modelHandle);
            modelHandle = nullptr;
        }
        if (libraryHandle != nullptr)
        {
            static_cast<void>(::dlclose(libraryHandle));
            libraryHandle = nullptr;
        }
    }

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Returns callbacks consumed by the ALSA speech-capture adapter.
     * @return Recognition operation table requiring this provider as context.
     */
    XWalkSpeechToTextAlsaOperations XWalkSpeechRecognizerVosk::operations() const noexcept
    {
        XWalkSpeechToTextAlsaOperations result{};
        result.recognizerReady = &ready;
        result.recognizePcm = &recognizePcm;
        result.startRecognition = &startRecognition;
        result.feedRecognition = &feedRecognition;
        result.finishRecognition = &finishRecognition;
        result.releaseRecognition = &releaseRecognition;
        result.recognizeFile = &recognizeFile;
        result.cancelRecognition = &cancel;
        return result;
    }

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

    /**
     * @brief Converts a callback context into its required provider.
     * @param[in,out] context Non-null pointer to a live provider.
     * @return Referenced provider.
     * @throws std::invalid_argument If `context` is null.
     */
    XWalkSpeechRecognizerVosk& XWalkSpeechRecognizerVosk::provider(contextpointer context)
    {
        if (context == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Vosk provider context must not be null");
        }
        return *static_cast<XWalkSpeechRecognizerVosk*>(context);
    }

    /**
     * @brief Resolves one required function from the loaded shared library.
     * @tparam FunctionType Function-pointer type associated with `symbolName`.
     * @param[in,out] library Non-null dynamic-library handle.
     * @param[in] symbolName Non-empty exported Vosk symbol.
     * @return Resolved function pointer or null when the symbol is unavailable.
     */
    template <typename FunctionType>
    FunctionType XWalkSpeechRecognizerVosk::loadFunction(contextpointer library, cstring symbolName)
    {
        static_assert(sizeof(FunctionType) == sizeof(contextpointer),
                      "Vosk function pointers must match dynamic symbol pointer size");
        static_cast<void>(::dlerror());
        contextpointer symbol = ::dlsym(library, symbolName);
        const hal::boolean symbolInvalid = static_cast<hal::boolean>((symbol == nullptr) || (::dlerror() != nullptr));
        if (symbolInvalid)
        {
            return FunctionType{};
        }
        FunctionType function{};
        std::memcpy(&function, &symbol, sizeof(function));
        return function;
    }

    /** @brief Reports model readiness. @param[in,out] context Provider context.
     * @return Readiness state. */
    boolean XWalkSpeechRecognizerVosk::ready(contextpointer context)
    {
        return provider(context).modelHandle != nullptr;
    }

    /**
     * @brief Recognizes one complete signed sixteen-bit PCM payload.
     * @param[in,out] context Non-null provider context.
     * @param[in] pcmData Complete interleaved PCM bytes.
     * @param[in] sampleRateHz Positive PCM sample rate in Hertz.
     * @param[in] channelCount Required mono channel count.
     * @return Owned final transcript, which may be empty.
     */
    string XWalkSpeechRecognizerVosk::recognizePcm(contextpointer context,
                                                   const bytevector& pcmData,
                                                   uint32 sampleRateHz,
                                                   uint8 channelCount)
    {
        XWalkSpeechRecognizerVosk& self = provider(context);
        if ((sampleRateHz == 0U) || (channelCount != 1U))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Vosk recognition requires positive-rate mono PCM");
        }
        const hal::boolean pcmDataTooLarge =
            static_cast<hal::boolean>(pcmData.size() > static_cast<size>(std::numeric_limits<int32>::max()));
        if (pcmDataTooLarge)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Vosk PCM exceeds the C API byte-count range");
        }
        const hal::boolean selfCancellationRequestedExchangeInvalid =
            static_cast<hal::boolean>(self.cancellationRequested.exchange(false) || pcmData.empty());
        if (selfCancellationRequestedExchangeInvalid)
        {
            return {};
        }
        voskrecognizerhandle recognizer = self.api.recognizerNew(self.modelHandle, static_cast<float>(sampleRateHz));
        if (recognizer == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk recognizer could not be created");
        }
        XWalkVoskRecognizerGuard recognizerGuard(recognizer, self.api.recognizerFree);
        const cstring pcmBytes = reinterpret_cast<cstring>(pcmData.data());
        static_cast<void>(self.api.acceptWaveform(recognizer, pcmBytes, static_cast<int32>(pcmData.size())));
        const cstring finalJson = self.api.finalResult(recognizer);
        const string result = (finalJson == nullptr) ? string{} : extractText(finalJson);
        const hal::boolean exchangeSucceeded = static_cast<hal::boolean>(self.cancellationRequested.exchange(false));
        if (exchangeSucceeded)
        {
            return {};
        }
        return result;
    }

    /**
     * @brief Creates one streaming Vosk recognizer for a microphone listen.
     * @param[in,out] context Non-null provider context.
     * @param[in] sampleRateHz Positive PCM sample rate in Hertz.
     * @param[in] channelCount Required mono channel count.
     * @return Newly owned recognizer handle, or null when Vosk creation fails.
     */
    speechrecognitionsession
    XWalkSpeechRecognizerVosk::startRecognition(contextpointer context, uint32 sampleRateHz, uint8 channelCount)
    {
        XWalkSpeechRecognizerVosk& self = provider(context);
        const hal::boolean formatInvalid = static_cast<hal::boolean>((sampleRateHz == 0U) || (channelCount != 1U));
        if (formatInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Vosk streaming recognition requires positive-rate mono PCM");
        }
        self.cancellationRequested.store(false);
        return self.api.recognizerNew(self.modelHandle, static_cast<float>(sampleRateHz));
    }

    /**
     * @brief Feeds one PCM period and reports Vosk endpoint acceptance.
     * @param[in,out] context Non-null provider context.
     * @param[in,out] session Non-null streaming Vosk recognizer.
     * @param[in] pcmData Non-empty signed-sixteen PCM bytes.
     * @return Listening, endpoint, or cancellation status.
     */
    XWalkSpeechRecognitionFeedStatus XWalkSpeechRecognizerVosk::feedRecognition(contextpointer context,
                                                                                speechrecognitionsession session,
                                                                                const bytevector& pcmData)
    {
        XWalkSpeechRecognizerVosk& self = provider(context);
        const hal::boolean sessionPcmInvalid = static_cast<hal::boolean>((session == nullptr) || pcmData.empty());
        if (sessionPcmInvalid)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Vosk streaming feed requires a session and PCM period");
        }
        const hal::boolean pcmDataTooLarge =
            static_cast<hal::boolean>(pcmData.size() > static_cast<size>(std::numeric_limits<int32>::max()));
        if (pcmDataTooLarge)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "Vosk streaming PCM exceeds the C API byte-count range");
        }
        const boolean cancellationRequested = self.cancellationRequested.load();
        if (cancellationRequested)
        {
            return XWalkSpeechRecognitionFeedStatus::Cancelled;
        }
        const cstring pcmBytes = reinterpret_cast<cstring>(pcmData.data());
        const int32 result = self.api.acceptWaveform(session, pcmBytes, static_cast<int32>(pcmData.size()));
        if (result < 0)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk streaming PCM ingestion failed");
        }
        const boolean cancellationRequested = self.cancellationRequested.load();
        if (cancellationRequested)
        {
            return XWalkSpeechRecognitionFeedStatus::Cancelled;
        }
        if (result > 0)
        {
            return XWalkSpeechRecognitionFeedStatus::Endpoint;
        }
        const cstring partialJson = self.api.partialResult(session);
        if (partialJson == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk streaming partial result is unavailable");
        }
        const string partialText = extractJsonValue(partialJson, "partial");
        const hal::boolean partialTextAvailable = static_cast<hal::boolean>(partialText.empty() == false);
        return partialTextAvailable ? XWalkSpeechRecognitionFeedStatus::SpeechObserved
                                    : XWalkSpeechRecognitionFeedStatus::Listening;
    }

    /**
     * @brief Retrieves the final transcript for an endpoint or hard timeout.
     * @param[in,out] context Non-null provider context.
     * @param[in,out] session Non-null streaming Vosk recognizer.
     * @param[in] endpointDetected Selects accepted-endpoint rather than timeout finalization.
     * @return Owned transcript, which may be empty for silence or cancellation.
     */
    string XWalkSpeechRecognizerVosk::finishRecognition(contextpointer context,
                                                        speechrecognitionsession session,
                                                        boolean endpointDetected)
    {
        XWalkSpeechRecognizerVosk& self = provider(context);
        if (session == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "Vosk streaming finalization requires a session");
        }
        const boolean cancellationRequested = self.cancellationRequested.exchange(false);
        if (cancellationRequested)
        {
            return {};
        }
        const cstring finalJson = endpointDetected ? self.api.result(session) : self.api.finalResult(session);
        if (finalJson == nullptr)
        {
            XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk streaming final result is unavailable");
        }
        return extractText(finalJson);
    }

    /**
     * @brief Releases one Vosk streaming recognizer.
     * @param[in,out] context Nullable provider context retained for callback symmetry.
     * @param[in,out] session Non-null recognizer handle to release.
     */
    void XWalkSpeechRecognizerVosk::releaseRecognition(contextpointer context,
                                                       speechrecognitionsession session) noexcept
    {
        if ((context != nullptr) && (session != nullptr))
        {
            XWalkSpeechRecognizerVosk& self = *static_cast<XWalkSpeechRecognizerVosk*>(context);
            self.api.recognizerFree(session);
        }
    }

    /**
     * @brief Rejects direct file transcription because no decoder is owned.
     * @param[in,out] context Non-null provider context.
     * @param[in] filePath Non-empty path supplied by the coordinator.
     * @return This function does not return normally.
     * @throws std::runtime_error Always, because only captured PCM is supported.
     */
    string XWalkSpeechRecognizerVosk::recognizeFile(contextpointer context, stringview filePath)
    {
        static_cast<void>(provider(context));
        static_cast<void>(filePath);
        XWALK_HAL_ERROR(XWALK_RUNTIME, "Vosk provider does not own an audio-file decoder");
    }

    /** @brief Requests recognition cancellation. @param[in,out] context Provider
     * context. */
    void XWalkSpeechRecognizerVosk::cancel(contextpointer context) noexcept
    {
        if (context != nullptr)
        {
            static_cast<XWalkSpeechRecognizerVosk*>(context)->cancellationRequested.store(true);
        }
    }

    /**
     * @brief Extracts the transcript from final Vosk JSON.
     * @param[in] jsonResult Final JSON object returned by Vosk.
     * @return Unescaped `text` value, or an empty string when absent.
     */
    string XWalkSpeechRecognizerVosk::extractText(stringview jsonResult)
    {
        return extractJsonValue(jsonResult, "text");
    }

    /**
     * @brief Extracts one unescaped named string from Vosk JSON.
     * @param[in] jsonResult JSON object returned by the Vosk C API.
     * @param[in] key Non-empty member name whose string value is requested.
     * @return Unescaped member value, or an empty string when absent or malformed.
     */
    string XWalkSpeechRecognizerVosk::extractJsonValue(stringview jsonResult, stringview key)
    {
        const string quotedKey = string{"\""} + string(key) + "\"";
        const size keyPosition = jsonResult.find(quotedKey);
        if (keyPosition == string::npos)
        {
            return {};
        }
        const size colonPosition = jsonResult.find(':', keyPosition + quotedKey.size());
        const size quotePosition = jsonResult.find('"', colonPosition + 1U);
        if ((colonPosition == string::npos) || (quotePosition == string::npos))
        {
            return {};
        }
        string result;
        boolean escaped{false};
        for (size index = quotePosition + 1U; index < jsonResult.size(); ++index)
        {
            const char value = jsonResult[index];
            if (escaped)
            {
                if (value == 'n')
                {
                    result.push_back('\n');
                }
                else if (value == 'r')
                {
                    result.push_back('\r');
                }
                else if (value == 't')
                {
                    result.push_back('\t');
                }
                else
                {
                    result.push_back(value);
                }
                escaped = false;
            }
            else if (value == '\\')
            {
                escaped = true;
            }
            else if (value == '"')
            {
                break;
            }
            else
            {
                result.push_back(value);
            }
        }
        return result;
    }

} /* namespace xwalk::hal */
