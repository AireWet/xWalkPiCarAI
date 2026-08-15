/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafetyTestSupport.h
 * @brief       Declares deterministic road-user safety scenario support.
 *
 * @project     xWalk Firmware
 * @module      xWalkRoadUserSafetyTest
 *
 * @author      Joxy John
 * @date        2026-08-11
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_ROAD_USER_SAFETY_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_ROAD_USER_SAFETY_TEST_SUPPORT_H

#include "xAgent_Rpi5CarRoadUserSafety.h"

#include <opencv2/videoio.hpp>

#include <vector>

namespace xwalk::agent::test::road_user_safety
{

    /** @brief Stores one generated scenario and every observable safety response. */
    struct ScenarioBackend
    {
            XWalkRoadUserDetectionBatch batch{};
            XWalkRoadSafetyStatus classifierStatus{XWalkRoadSafetyStatus::Ok};
            uint32 detectCount{};
            uint32 classifyCount{};
            uint32 alertCount{};
            uint32 stopCount{};
            uint32 faultCount{};
            XWalkRoadRisk lastAlert{XWalkRoadRisk::Safe};
            XWalkRoadSafetyStatus lastFault{XWalkRoadSafetyStatus::Ok};
    };

    /** @brief Stores one generated recorded scenario and all observable outputs. */
    struct RecordedScenarioBackend
    {
            filesystempath directory{};                /**< Isolated directory containing the generated AVI. */
            filesystempath videoPath{};                /**< Complete generated recorded-video path. */
            cv::VideoCapture capture{};                /**< OpenCV reader owned by the test scenario. */
            std::vector<uint32> frameIndices{};        /**< Decoded frame markers in processing order. */
            roaduserdetectionvector detectedObjects{}; /**< Image-derived detections in order. */
            std::vector<XWalkRoadRiskFeatures> classifiedFeatures{}; /**< Classifier inputs in order. */
            std::vector<XWalkRoadSafetyStatus> faults{};             /**< Fail-safe statuses in order. */
            uint32 alertCount{};              /**< Number of warning or dangerous alert requests. */
            uint32 stopCount{};               /**< Number of stop-and-disarm requests. */
            boolean redLedActive{};           /**< Simulated red status LED state. */
            boolean buzzerActive{};           /**< Simulated buzzer state. */
            boolean motorsArmed{true};        /**< Simulated motor-controller armed state. */
            boolean robotMoving{true};        /**< Simulated current movement state. */
            boolean previousDistanceValid{};  /**< Whether a prior person distance is available. */
            float64 previousDistanceMeters{}; /**< Prior distance used for closing-speed extraction. */
    };

    /** @brief Summarizes one deterministic OpenCV asset decode. */
    struct RecordedAssetSummary
    {
            boolean opened{};                                  /**< True when OpenCV opened the media container. */
            uint32 frameCount{};                               /**< Number of non-empty decoded frames. */
            uint32 width{};                                    /**< Width shared by every decoded frame. */
            uint32 height{};                                   /**< Height shared by every decoded frame. */
            uint64 contentHash{14'695'981'039'346'656'037ULL}; /**< FNV-1a decoded-pixel hash. */
            boolean monotonicOrder{true};                      /**< False if decoder position did not increase. */
            boolean cleanEndOfVideo{};                         /**< True only when exhaustion returns no frame. */
    };

    /** @brief Returns callbacks implementing a deterministic synthetic classifier. */
    XWalkRoadUserSafetyCallbacks callbacks() noexcept;
    /** @brief Creates one supported detection with explicit synthetic geometry. */
    XWalkRoadUserDetection detection(XWalkRoadUserClass roadUserClass,
                                     float64 confidence,
                                     float64 distanceMeters,
                                     float64 closingSpeedMetersPerSecond,
                                     float64 lateralOffsetNormalized,
                                     float64 boundingAreaNormalized);
    /** @brief Creates and opens the deterministic four-frame recorded scenario. */
    boolean createRecordedScenario(RecordedScenarioBackend& scenario) noexcept;
    /** @brief Releases and removes every recorded-scenario test artifact. */
    void destroyRecordedScenario(RecordedScenarioBackend& scenario) noexcept;
    /** @brief Returns callbacks for the complete recorded-video safety pipeline. */
    XWalkRoadUserSafetyCallbacks recordedScenarioCallbacks() noexcept;
    /** @brief Decodes one committed media fixture without invoking a detector. */
    RecordedAssetSummary decodeRecordedAsset(const filesystempath& path) noexcept;

} /* namespace xwalk::agent::test::road_user_safety */

#endif /* XAGENT_RPI5CAR_ROAD_USER_SAFETY_TEST_SUPPORT_H */
