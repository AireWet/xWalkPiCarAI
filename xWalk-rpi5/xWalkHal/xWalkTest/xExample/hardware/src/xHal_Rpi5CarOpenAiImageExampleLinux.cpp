/******************************************************************************
 * @file        xHal_Rpi5CarOpenAiImageExampleLinux.cpp
 * @brief       Implements camera, console, and HTTPS OpenAI composition.
 *
 * @project     xWalk Firmware
 * @module      xExample Hardware
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

#include "xHal_Rpi5CarOpenAiImageExampleLinux.h"

#include "xHal_Rpi5CarCameraLinux.h"
#include "xHal_Rpi5CarExampleConfig.h"
#include "xHal_Rpi5CarLanguageModelOllama.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains contracts and adapters for ported example programs.
 */
namespace xwalk::hal::example
{

    /**
     * @brief Runs bounded OpenAI image chat through Linux adapters.
     * @param[in] apiKey Non-empty OpenAI API credential.
     * @param[in] maximumPrompts Prompt limit from one through 100.
     * @param[in] connection Exact lowercase `csi` or `usb` camera connection.
     * @param[in] captureExecutable Non-empty camera capture executable.
     * @param[in] cameraDevice V4L2 device used only for USB capture.
     * @warning Captures images and sends them with prompt text to OpenAI.
     */
    void XWalkOpenAiImageExampleLinux::run(stringview apiKey,
                                           uint32 maximumPrompts,
                                           stringview connection,
                                           stringview captureExecutable,
                                           stringview cameraDevice)
    {
        constexpr stringview endpoint{"https://api.openai.com/v1/chat/completions"};
        constexpr stringview modelName{"gpt-4o"};
        constexpr stringview imagePath{XHAL_RPI5CAR_EXAMPLE_IMAGE_PATH};

        const XWalkCameraConnection connectionValue = XWalkCamera::connectionFromString(connection);
        XWalkCameraLinux cameraBackend(connectionValue, captureExecutable, cameraDevice);
        const XWalkCameraConfiguration cameraConfiguration{640U, 480U, 5'000U};
        XWalkCamera camera(&cameraBackend, cameraBackend.callback(), cameraConfiguration);
        XWalkLanguageModelHttp modelBackend(
            XWalkLanguageModelHttpDialect::OpenAiChatCompletions, endpoint, modelName, apiKey);
        XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        const XWalkOpenAiImageExampleCallbacks callbacks{&readPrompt, &write};
        XWalkOpenAiImageExample example(camera, languageModel, this, callbacks);
        example.run(maximumPrompts, imagePath);
    }

    /**
     * @brief Prints the source prompt and reads one terminal line.
     * @param[in,out] context Unused callback context.
     * @param[out] inputText Entered line without its delimiter.
     * @return `true` after a line is read, or `false` at end of input.
     */
    boolean XWalkOpenAiImageExampleLinux::readPrompt(contextpointer context, string& inputText)
    {
        static_cast<void>(context);
        std::cout << ">>> " << std::flush;
        return static_cast<boolean>(std::getline(std::cin, inputText));
    }

    /**
     * @brief Writes one source-compatible output fragment.
     * @param[in,out] context Unused callback context.
     * @param[in] text Text to write without modification.
     * @param[in] appendNewline Whether to append one newline.
     * @param[in] flushOutput Whether to flush after writing.
     */
    void XWalkOpenAiImageExampleLinux::write(contextpointer context,
                                             stringview text,
                                             boolean appendNewline,
                                             boolean flushOutput)
    {
        static_cast<void>(context);
        std::cout << text;
        if (appendNewline)
        {
            std::cout << '\n';
        }
        if (flushOutput)
        {
            std::cout << std::flush;
        }
    }

} /* namespace xwalk::hal::example */
