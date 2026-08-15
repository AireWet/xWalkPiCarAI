/******************************************************************************
 * @file        xHal_Rpi5CarDoubaoImageExampleLinux.cpp
 * @brief       Implements camera, console, and HTTPS Doubao composition.
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

#include "xHal_Rpi5CarDoubaoImageExampleLinux.h"

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
     * @brief Runs bounded Doubao image chat through Linux adapters.
     * @param[in] apiKey Non-empty Doubao API credential.
     * @param[in] maximumPrompts Prompt limit from one through 100.
     * @param[in] connection Exact lowercase `csi` or `usb` camera connection.
     * @param[in] captureExecutable Non-empty camera capture executable.
     * @param[in] cameraDevice V4L2 device used only for USB capture.
     * @warning Captures images and sends them with prompt text to Doubao.
     */
    void XWalkDoubaoImageExampleLinux::run(stringview apiKey,
                                           uint32 maximumPrompts,
                                           stringview connection,
                                           stringview captureExecutable,
                                           stringview cameraDevice)
    {
        constexpr stringview endpoint{"https://ark.cn-beijing.volces.com/api/v3/chat/completions"};
        constexpr stringview modelName{"doubao-seed-1-6-250615"};
        constexpr stringview imagePath{XHAL_RPI5CAR_EXAMPLE_IMAGE_PATH};

        const XWalkCameraConnection connectionValue = XWalkCamera::connectionFromString(connection);
        XWalkCameraLinux cameraBackend(connectionValue, captureExecutable, cameraDevice);
        XWalkCamera camera(&cameraBackend, cameraBackend.callback());
        XWalkLanguageModelHttp modelBackend(
            XWalkLanguageModelHttpDialect::OpenAiChatCompletions, endpoint, modelName, apiKey);
        XWalkLanguageModel languageModel(&modelBackend, modelBackend.callbacks());
        const XWalkDoubaoImageExampleCallbacks callbacks{&readPrompt, &write};
        XWalkDoubaoImageExample example(camera, languageModel, this, callbacks);
        example.run(maximumPrompts, imagePath);
    }

    /**
     * @brief Prints the source prompt and reads one terminal line.
     * @param[in,out] context Unused callback context.
     * @param[out] inputText Entered line without its delimiter.
     * @return `true` after a line is read, or `false` at end of input.
     */
    boolean XWalkDoubaoImageExampleLinux::readPrompt(contextpointer context, string& inputText)
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
    void XWalkDoubaoImageExampleLinux::write(contextpointer context,
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
