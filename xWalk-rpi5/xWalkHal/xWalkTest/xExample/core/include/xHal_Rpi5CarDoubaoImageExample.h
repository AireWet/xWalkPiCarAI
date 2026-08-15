/******************************************************************************
 * @file        xHal_Rpi5CarDoubaoImageExample.h
 * @brief       Declares the bounded Doubao camera-chat example flow.
 *
 * @details
 * Coordinates injected still capture, image-capable language-model prompting,
 * and console input/output without owning any of those dependencies.
 *
 * @project     xWalk Firmware
 * @module      xExample
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

#ifndef XHAL_RPI5CAR_DOUBAO_IMAGE_EXAMPLE_H
#define XHAL_RPI5CAR_DOUBAO_IMAGE_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCamera.h"
#include "xHal_Rpi5CarLanguageModel.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Highest bounded prompt count accepted by the camera-chat example. */
#define XHAL_RPI5CAR_DOUBAO_IMAGE_EXAMPLE_MAXIMUM_PROMPTS 100U

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
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Reads one prompted console line.
     * @param[in,out] context Non-owning console context.
     * @param[out] inputText Owned user input replaced by the callback.
     * @return `true` when input was read, or `false` at end of input.
     */
    using doubaoimagereadcallback = boolean (*)(contextpointer context, string& inputText);

    /**
     * @brief Writes one welcome or response fragment.
     * @param[in,out] context Non-owning console context.
     * @param[in] text Text valid only for the callback duration.
     * @param[in] appendNewline Whether to append one newline.
     * @param[in] flushOutput Whether to flush output before returning.
     */
    using doubaoimagewritecallback = void (*)(contextpointer context,
                                              stringview text,
                                              boolean appendNewline,
                                              boolean flushOutput);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /** @brief Complete injected console table required by the Doubao example. */
    struct XWalkDoubaoImageExampleCallbacks
    {
            /** @brief Prints the prompt and reads one input line. */
            doubaoimagereadcallback readPrompt{nullptr};
            /** @brief Writes welcome text and model responses. */
            doubaoimagewritecallback write{nullptr};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Runs bounded image chat through caller-owned camera and model objects. */
    class XWalkDoubaoImageExample final
    {
        private:
            /** @brief Caller-owned camera that must outlive this example. */
            XWalkCamera* cameraObject;
            /** @brief Caller-owned language model that must outlive this example. */
            XWalkLanguageModel* languageModelObject;
            /** @brief Non-owning context forwarded to console callbacks. */
            contextpointer consoleContext;
            /** @brief Complete validated console callback table. */
            XWalkDoubaoImageExampleCallbacks callbacks;

        public:
            /**
             * @brief Binds the camera, language model, and console table.
             * @param[in,out] camera Caller-owned camera that must outlive this object.
             * @param[in,out] languageModel Caller-owned model that must outlive this object.
             * @param[in,out] context Non-owning console callback context.
             * @param[in] consoleCallbacks Table containing two non-null callbacks.
             * @throws std::invalid_argument If either console callback is null.
             */
            XWalkDoubaoImageExample(XWalkCamera& camera,
                                    XWalkLanguageModel& languageModel,
                                    contextpointer context,
                                    const XWalkDoubaoImageExampleCallbacks& consoleCallbacks);

            /** @brief Prevents copying of non-owning dependency bindings. */
            XWalkDoubaoImageExample(const XWalkDoubaoImageExample&) = delete;
            /** @brief Prevents moving of non-owning dependency bindings. */
            XWalkDoubaoImageExample(XWalkDoubaoImageExample&&) = delete;
            /** @brief Prevents copy assignment of non-owning dependency bindings. */
            XWalkDoubaoImageExample& operator=(const XWalkDoubaoImageExample&) = delete;
            /** @brief Prevents move assignment of non-owning dependency bindings. */
            XWalkDoubaoImageExample& operator=(XWalkDoubaoImageExample&&) = delete;

            /**
             * @brief Captures and submits an image for each bounded user prompt.
             * @param[in] maximumPrompts Prompt limit in the inclusive range one through 100.
             * @param[in] imagePath Non-empty destination reused for every captured JPEG.
             * @throws std::out_of_range If `maximumPrompts` is outside its range.
             * @warning Prompt operations may upload captured images to a remote provider.
             */
            void run(uint32 maximumPrompts, stringview imagePath);
    };

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_DOUBAO_IMAGE_EXAMPLE_H */
