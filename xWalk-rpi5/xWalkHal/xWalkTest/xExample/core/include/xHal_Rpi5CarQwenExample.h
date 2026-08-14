/******************************************************************************
 * @file        xHal_Rpi5CarQwenExample.h
 * @brief       Declares the bounded interactive Qwen example flow.
 *
 * @details
 * Preserves the upstream instructions, welcome text, model-history limit, and
 * prompt loop through a provider-neutral language model and injected console.
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

#ifndef XHAL_RPI5CAR_QWEN_EXAMPLE_H
#define XHAL_RPI5CAR_QWEN_EXAMPLE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModel.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Highest bounded prompt count accepted by the interactive example. */
#define XHAL_RPI5CAR_QWEN_EXAMPLE_MAXIMUM_PROMPTS 100U

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
using qwenexamplereadcallback = boolean (*)(contextpointer context,
    string& inputText);

/**
 * @brief Writes one welcome or response fragment.
 * @param[in,out] context Non-owning console context.
 * @param[in] text Text valid only for the callback duration.
 * @param[in] appendNewline Whether to append one newline.
 * @param[in] flushOutput Whether to flush output before returning.
 */
using qwenexamplewritecallback = void (*)(contextpointer context,
    stringview text, boolean appendNewline, boolean flushOutput);

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Complete injected console table required by the Qwen example. */
struct XWalkQwenExampleCallbacks
{
    /** @brief Prints the prompt and reads one input line. */
    qwenexamplereadcallback readPrompt{nullptr};
    /** @brief Writes welcome text and model responses. */
    qwenexamplewritecallback write{nullptr};
};

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/** @brief Runs a bounded provider-neutral version of the Qwen chat example. */
class XWalkQwenExample final
{
private:

    /** @brief Caller-owned language model that must outlive this example. */
    XWalkLanguageModel* languageModelObject;
    /** @brief Non-owning context forwarded to console callbacks. */
    contextpointer consoleContext;
    /** @brief Complete validated console callback table. */
    XWalkQwenExampleCallbacks callbacks;

public:

    /**
     * @brief Binds one language model and complete console table.
     * @param[in,out] languageModel Caller-owned model that must outlive this object.
     * @param[in,out] context Non-owning console callback context.
     * @param[in] consoleCallbacks Table containing two non-null callbacks.
     * @throws std::invalid_argument If either callback is null.
     */
    XWalkQwenExample(XWalkLanguageModel& languageModel,
        contextpointer context, const XWalkQwenExampleCallbacks& consoleCallbacks);

    /** @brief Prevents copying of non-owning dependency bindings. */
    XWalkQwenExample(const XWalkQwenExample&) = delete;
    /** @brief Prevents moving of non-owning dependency bindings. */
    XWalkQwenExample(XWalkQwenExample&&) = delete;
    /** @brief Prevents copy assignment of non-owning dependency bindings. */
    XWalkQwenExample& operator=(const XWalkQwenExample&) = delete;
    /** @brief Prevents move assignment of non-owning dependency bindings. */
    XWalkQwenExample& operator=(XWalkQwenExample&&) = delete;

    /**
     * @brief Runs up to the requested number of interactive prompts.
     * @param[in] maximumPrompts Prompt limit in the inclusive range one through 100.
     * @throws std::out_of_range If `maximumPrompts` is outside its range.
     * @warning Prompt callbacks may perform remote network requests.
     */
    void run(uint32 maximumPrompts);
};

} /* namespace xwalk::hal::example */

#endif /* XHAL_RPI5CAR_QWEN_EXAMPLE_H */
