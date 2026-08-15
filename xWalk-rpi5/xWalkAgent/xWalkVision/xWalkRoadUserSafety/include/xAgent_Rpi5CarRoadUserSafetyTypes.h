/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafetyTypes.h
 * @brief       Declares road-user detection and risk-classification contracts.
 *
 * @details
 * Defines a model-neutral boundary for a future camera, YOLO detector, feature
 * extractor, and Random Forest classifier without claiming a trained model.
 *
 * @project     xWalk Firmware
 * @module      xWalkRoadUserSafety
 *
 * @author      Joxy John
 * @date        2026-08-11
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_ROAD_USER_SAFETY_TYPES_H
#define XAGENT_RPI5CAR_ROAD_USER_SAFETY_TYPES_H

#include "xHal_Rpi5CarTypes.h"

#include <vector>

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /**
     * @enum XWalkRoadUserClass
     * @brief Identifies the five road-user classes accepted by the safety pipeline.
     */
    enum class XWalkRoadUserClass : agent::int32
    {
        /** @brief A detected person or pedestrian. */
        Person = 0,
        /** @brief A detected passenger car. */
        Car = 1,
        /** @brief A detected bicycle and rider. */
        Bicycle = 2,
        /** @brief A detected bus. */
        Bus = 3,
        /** @brief A detected motorcycle or motorbike. */
        Motorbike = 4
    };

    /**
     * @enum XWalkRoadRisk
     * @brief Reports the most severe validated risk for one camera sample.
     */
    enum class XWalkRoadRisk : agent::uint8
    {
        /** @brief No detector or classifier evidence currently requires intervention. */
        Safe = 0U,
        /** @brief Uncertain or approaching traffic requires an operator alert. */
        Warning = 1U,
        /** @brief Immediate risk requires motor stop and an operator alert. */
        Dangerous = 2U,
        /** @brief Pipeline failure requires motor stop and explicit recovery. */
        FailSafeStop = 3U
    };

    /**
     * @enum XWalkRoadSafetyStatus
     * @brief Describes detector, model, classifier, and camera outcomes.
     */
    enum class XWalkRoadSafetyStatus : agent::uint8
    {
        /** @brief The requested operation completed with a valid result. */
        Ok = 0U,
        /** @brief The configured camera is disconnected or unavailable. */
        CameraUnavailable,
        /** @brief Camera acquisition exceeded its configured deadline. */
        CameraTimeout,
        /** @brief A finite recorded source reached its normal end. */
        EndOfStream,
        /** @brief The current grayscale sample no longer contains a usable line. */
        LineLost,
        /** @brief A required detector or classifier model is missing. */
        ModelUnavailable,
        /** @brief A model artifact is unreadable or corrupt. */
        ModelCorrupt,
        /** @brief An inference tensor has an unexpected shape or type. */
        InvalidTensor,
        /** @brief Inference exceeded its configured deadline. */
        InferenceTimeout,
        /** @brief Detector output contains invalid numbers, classes, or geometry. */
        InvalidOutput,
        /** @brief Risk classification failed or returned an invalid result. */
        ClassifierFailure
    };

    /**
     * @struct XWalkRoadUserDetection
     * @brief Contains one backend-neutral detected road user and extracted geometry.
     */
    struct XWalkRoadUserDetection
    {
            /** @brief Raw class identifier; only zero through four are accepted. */
            agent::int32 classIdentifier{};
            /** @brief Detector confidence in the inclusive range zero through one. */
            agent::float64 confidence{};
            /** @brief Estimated forward distance in non-negative meters. */
            agent::float64 distanceMeters{};
            /** @brief Positive closing speed toward the robot in meters per second. */
            agent::float64 closingSpeedMetersPerSecond{};
            /** @brief Horizontal offset from the path center in the inclusive range minus one through one. */
            agent::float64 lateralOffsetNormalized{};
            /** @brief Bounding-box area divided by frame area in the inclusive range zero through one. */
            agent::float64 boundingAreaNormalized{};
    };

    /** @brief Owned list of detections produced for one camera sample. */
    using roaduserdetectionvector = std::vector<XWalkRoadUserDetection>;

    /**
     * @struct XWalkRoadUserDetectionBatch
     * @brief Reports detector status and detections for one bounded frame operation.
     */
    struct XWalkRoadUserDetectionBatch
    {
            /** @brief Detector and camera completion status. */
            XWalkRoadSafetyStatus status{XWalkRoadSafetyStatus::Ok};
            /** @brief Owned detections; empty is a valid successful result. */
            roaduserdetectionvector detections{};
    };

    /**
     * @struct XWalkRoadRiskFeatures
     * @brief Supplies validated detector features to a future risk classifier.
     */
    struct XWalkRoadRiskFeatures
    {
            /** @brief Validated road-user class. */
            XWalkRoadUserClass roadUserClass{XWalkRoadUserClass::Person};
            /** @brief Detector confidence in the inclusive range zero through one. */
            agent::float64 confidence{};
            /** @brief Estimated non-negative forward distance in meters. */
            agent::float64 distanceMeters{};
            /** @brief Closing speed in meters per second. */
            agent::float64 closingSpeedMetersPerSecond{};
            /** @brief Absolute normalized lateral distance from the path center. */
            agent::float64 absoluteLateralOffset{};
            /** @brief Bounding-box area divided by frame area. */
            agent::float64 boundingAreaNormalized{};
    };

    /**
     * @struct XWalkRoadRiskClassification
     * @brief Reports one classifier status and risk decision.
     */
    struct XWalkRoadRiskClassification
    {
            /** @brief Classifier completion status. */
            XWalkRoadSafetyStatus status{XWalkRoadSafetyStatus::Ok};
            /** @brief Risk assigned to the supplied validated feature vector. */
            XWalkRoadRisk risk{XWalkRoadRisk::Safe};
    };

    /**
     * @struct XWalkRoadUserSafetyResult
     * @brief Reports the aggregate pipeline result for one frame.
     */
    struct XWalkRoadUserSafetyResult
    {
            /** @brief Aggregate risk after validation and classification. */
            XWalkRoadRisk risk{XWalkRoadRisk::Safe};
            /** @brief Pipeline status; non-Ok results always use FailSafeStop risk. */
            XWalkRoadSafetyStatus status{XWalkRoadSafetyStatus::Ok};
            /** @brief Number of validated detections considered by the classifier. */
            agent::uint32 detectionCount{};
    };

    /** @brief Acquires one bounded model-neutral detection batch. */
    using roaduserdetectcallback = XWalkRoadUserDetectionBatch (*)(agent::contextpointer context) noexcept;
    /** @brief Classifies one validated road-user feature vector. */
    using roaduserriskclassifycallback =
        XWalkRoadRiskClassification (*)(agent::contextpointer context, const XWalkRoadRiskFeatures& features) noexcept;
    /** @brief Reports a warning or dangerous risk to an operator-owned alert sink. */
    using roaduseralertcallback = void (*)(agent::contextpointer context,
                                           XWalkRoadRisk risk,
                                           agent::uint32 detectionCount) noexcept;
    /** @brief Stops and disarms motion after danger or pipeline failure. */
    using roaduserstopcallback = void (*)(agent::contextpointer context) noexcept;
    /** @brief Reports one fail-safe pipeline fault without throwing. */
    using roaduserfaultcallback = void (*)(agent::contextpointer context, XWalkRoadSafetyStatus status) noexcept;

    /**
     * @struct XWalkRoadUserSafetyCallbacks
     * @brief Groups the complete detector, classifier, alert, and motion-safety boundary.
     */
    struct XWalkRoadUserSafetyCallbacks
    {
            /** @brief Non-null bounded detector operation. */
            roaduserdetectcallback detect{nullptr};
            /** @brief Non-null risk-classifier operation. */
            roaduserriskclassifycallback classify{nullptr};
            /** @brief Non-null non-throwing operator-alert operation. */
            roaduseralertcallback alert{nullptr};
            /** @brief Non-null non-throwing motor stop-and-disarm operation. */
            roaduserstopcallback stopMotion{nullptr};
            /** @brief Non-null non-throwing fault-report operation. */
            roaduserfaultcallback reportFault{nullptr};
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_ROAD_USER_SAFETY_TYPES_H */
