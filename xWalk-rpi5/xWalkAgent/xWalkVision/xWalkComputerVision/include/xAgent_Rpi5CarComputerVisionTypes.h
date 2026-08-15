/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionTypes.h
 * @brief       Declares computer-vision commands, observations, and callbacks.
 *
 * @details
 * Defines the backend-neutral boundary used to port the interactive behavior
 * from `example/7.computer_vision.py` without owning camera resources.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_COMPUTER_VISION_TYPES_H
#define XAGENT_RPI5CAR_COMPUTER_VISION_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Enumeration declarations
     ******************************************************************************/

    /**
     * @enum XWalkComputerVisionColor
     * @brief Selects the source example's color-detection modes.
     */
    enum class XWalkComputerVisionColor : agent::uint8
    {
        /** @brief Disables color detection. */
        Close = 0U,
        /** @brief Detects red regions. */
        Red,
        /** @brief Detects orange regions. */
        Orange,
        /** @brief Detects yellow regions. */
        Yellow,
        /** @brief Detects green regions. */
        Green,
        /** @brief Detects blue regions. */
        Blue,
        /** @brief Detects purple regions. */
        Purple
    };

    /**
     * @enum XWalkComputerVisionEvent
     * @brief Identifies the primary outcome of one interactive key.
     */
    enum class XWalkComputerVisionEvent : agent::uint8
    {
        /** @brief The key has no source-compatible action. */
        Ignored = 0U,
        /** @brief A JPEG photograph was captured. */
        PhotoCaptured,
        /** @brief Color detection was changed. */
        ColorChanged,
        /** @brief Face detection was toggled. */
        FaceChanged,
        /** @brief QR detection was toggled. */
        QrChanged,
        /** @brief Current color and face observations were sampled. */
        ObjectsShown,
        /** @brief Cancellation interrupted the post-command wait. */
        Cancelled
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkComputerVisionDetection
     * @brief Describes one detected object in camera pixel coordinates.
     */
    struct XWalkComputerVisionDetection
    {
            /** @brief Number of objects detected in the current frame. */
            agent::uint32 count{};
            /** @brief Horizontal coordinate of the selected object's center. */
            agent::int32 centerX{};
            /** @brief Vertical coordinate of the selected object's center. */
            agent::int32 centerY{};
            /** @brief Width of the selected object's bounding rectangle. */
            agent::uint32 width{};
            /** @brief Height of the selected object's bounding rectangle. */
            agent::uint32 height{};
    };

    /**
     * @struct XWalkComputerVisionObservation
     * @brief Contains the latest color, face, and QR observations.
     */
    struct XWalkComputerVisionObservation
    {
            /** @brief Latest selected-color observation. */
            XWalkComputerVisionDetection color{};
            /** @brief Latest frontal-face observation. */
            XWalkComputerVisionDetection face{};
            /** @brief Latest decoded QR text, or empty when none is detected. */
            agent::string qrData{};
    };

    /**
     * @struct XWalkComputerVisionResult
     * @brief Reports one key action and its observable provider state.
     */
    struct XWalkComputerVisionResult
    {
            /** @brief Primary action completed for the supplied key. */
            XWalkComputerVisionEvent event{XWalkComputerVisionEvent::Ignored};
            /** @brief Active color mode after the action. */
            XWalkComputerVisionColor color{XWalkComputerVisionColor::Close};
            /** @brief Face-detection state after the action. */
            agent::boolean faceEnabled{};
            /** @brief QR-detection state after the action. */
            agent::boolean qrEnabled{};
            /** @brief Captured path for PhotoCaptured, or empty otherwise. */
            agent::string photoPath{};
            /** @brief Observation returned for object display or QR polling. */
            XWalkComputerVisionObservation observation{};
            /** @brief True when `observation.qrData` differs from the last reported QR text. */
            agent::boolean qrChanged{};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Starts the caller-owned vision provider. */
    using computervisionstartcallback = agent::boolean (*)(agent::contextpointer context);
    /** @brief Stops the caller-owned vision provider without throwing. */
    using computervisionstopcallback = void (*)(agent::contextpointer context) noexcept;
    /** @brief Captures one photograph and returns its complete destination path. */
    using computervisioncapturecallback = agent::string (*)(agent::contextpointer context);
    /** @brief Selects one color-detection mode. */
    using computervisioncolorcallback = void (*)(agent::contextpointer context, XWalkComputerVisionColor color);
    /** @brief Enables or disables one binary detector. */
    using computervisionswitchcallback = void (*)(agent::contextpointer context, agent::boolean enabled);
    /** @brief Samples current detection results from one camera frame. */
    using computervisionobservecallback = XWalkComputerVisionObservation (*)(agent::contextpointer context);
    /** @brief Suspends execution for a bounded interval. */
    using computervisiondelaycallback = void (*)(agent::contextpointer context, agent::uint32 durationMs);
    /** @brief Reports whether interactive execution may continue. */
    using computervisioncontinuecallback = agent::boolean (*)(agent::contextpointer context);

    /**
     * @struct XWalkComputerVisionCallbacks
     * @brief Groups the complete synchronous provider and scheduling boundary.
     */
    struct XWalkComputerVisionCallbacks
    {
            /** @brief Non-null provider-start operation. */
            computervisionstartcallback start{nullptr};
            /** @brief Non-null non-throwing provider-stop operation. */
            computervisionstopcallback stop{nullptr};
            /** @brief Non-null photograph operation. */
            computervisioncapturecallback capture{nullptr};
            /** @brief Non-null color-selection operation. */
            computervisioncolorcallback setColor{nullptr};
            /** @brief Non-null face-detector switch. */
            computervisionswitchcallback setFace{nullptr};
            /** @brief Non-null QR-detector switch. */
            computervisionswitchcallback setQr{nullptr};
            /** @brief Non-null observation operation. */
            computervisionobservecallback observe{nullptr};
            /** @brief Non-null timing operation. */
            computervisiondelaycallback delay{nullptr};
            /** @brief Non-null cancellation query. */
            computervisioncontinuecallback continueOperation{nullptr};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_COMPUTER_VISION_TYPES_H */
