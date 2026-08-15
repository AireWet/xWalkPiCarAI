/******************************************************************************
 * @file        xHal_Rpi5CarOllamaImageExampleLinux.h
 * @brief       Declares camera, console, and HTTPS Ollama composition.
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

#ifndef XHAL_RPI5CAR_OLLAMA_IMAGE_EXAMPLE_LINUX_H
#define XHAL_RPI5CAR_OLLAMA_IMAGE_EXAMPLE_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarOllamaImageExample.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::example
 * @brief Contains contracts and adapters for ported example programs.
 */
namespace xwalk::hal::example
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Composes Ollama image chat with Linux camera and terminal adapters. */
    class XWalkOllamaImageExampleLinux final
    {
        protected:
            /** @brief Prints `>>> ` and reads one terminal input line. */
            static boolean readPrompt(contextpointer context, string& inputText);
            /** @brief Writes one welcome or response fragment to standard output. */
            static void write(contextpointer context, stringview text, boolean appendNewline, boolean flushOutput);

        public:
            /**
             * @brief Runs bounded camera chat through the Ollama API.
             * @param[in] maximumPrompts Prompt limit from one through 100.
             * @param[in] connection Exact lowercase `csi` or `usb` camera connection.
             * @param[in] captureExecutable Non-empty camera capture executable.
             * @param[in] cameraDevice V4L2 device used only for USB capture.
             * @warning Captures images and sends them with user prompts over HTTPS.
             */
            void
            run(uint32 maximumPrompts, stringview connection, stringview captureExecutable, stringview cameraDevice);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_OLLAMA_IMAGE_EXAMPLE_LINUX_H */
