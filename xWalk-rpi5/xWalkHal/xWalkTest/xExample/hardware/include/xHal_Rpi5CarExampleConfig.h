/******************************************************************************
 * @file        xHal_Rpi5CarExampleConfig.h
 * @brief       Defines source-visible xExample configuration defaults.
 *
 * @details
 * Supports editor and direct-source parsing. Configured CMake targets prepend
 * their generated build-local header with the same name.
 *
 * @project     xWalk Firmware
 * @module      xExample
 *
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_EXAMPLE_CONFIG_H
#define XHAL_RPI5CAR_EXAMPLE_CONFIG_H

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Source-visible camera-chat output used outside a configured target. */
#define XHAL_RPI5CAR_EXAMPLE_IMAGE_PATH "llm-img.jpg"

/** @brief Source-visible selector configuration used outside a configured target. */
#define XHAL_RPI5CAR_EXAMPLE_YAML_PATH "xHal_Rpi5CarExampleConfig.yml"

/** @brief Source-visible native Ollama endpoint used outside a configured target. */
#define XHAL_RPI5CAR_EXAMPLE_OLLAMA_ENDPOINT "http://localhost:11434/api/chat"

/** @brief Source-visible text-only Ollama endpoint used outside a configured target. */
#define XHAL_RPI5CAR_EXAMPLE_OLLAMA_TEXT_ENDPOINT "http://localhost:11434/api/chat"

/** @brief Source-visible Qwen endpoint used outside a configured target. */
#define XHAL_RPI5CAR_EXAMPLE_QWEN_ENDPOINT "https://dashscope-intl.aliyuncs.com/compatible-mode/v1/chat/completions"

#endif /* XHAL_RPI5CAR_EXAMPLE_CONFIG_H */
