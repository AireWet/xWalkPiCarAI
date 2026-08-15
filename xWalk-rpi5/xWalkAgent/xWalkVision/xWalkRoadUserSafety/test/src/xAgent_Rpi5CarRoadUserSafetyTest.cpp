/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafetyTest.cpp
 * @brief       Verifies deterministic road-user risks and fail-safe transitions.
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

#include "xAgent_Rpi5CarRoadUserSafetyTestSupport.h"

#include <gtest/gtest.h>
#include <limits>

namespace
{
    using namespace xwalk::agent;
    using namespace xwalk::agent::test::road_user_safety;

    /** @brief Verifies empty road and distant-pedestrian scenarios remain safe. */
    TEST(XWalkRoadUserSafety, EmptyAndDistantRoadAreSafe)
    {
        ScenarioBackend empty;
        XWalkRoadUserSafety emptyPipeline(&empty, callbacks());
        EXPECT_EQ(emptyPipeline.evaluate().risk, XWalkRoadRisk::Safe);
        EXPECT_EQ(empty.stopCount, 0U);

        ScenarioBackend distant;
        distant.batch.detections.push_back(detection(XWalkRoadUserClass::Person, 0.95, 20.0, 0.2, 0.1, 0.01));
        XWalkRoadUserSafety distantPipeline(&distant, callbacks());
        const XWalkRoadUserSafetyResult result = distantPipeline.evaluate();
        EXPECT_EQ(result.risk, XWalkRoadRisk::Safe);
        EXPECT_EQ(result.detectionCount, 1U);
        EXPECT_EQ(distant.stopCount, 0U);
    }

    /** @brief Verifies approaching, low-confidence, and occluded detections warn. */
    TEST(XWalkRoadUserSafety, ApproachingOrUncertainRoadUserWarns)
    {
        ScenarioBackend backend;
        backend.batch.detections = {detection(XWalkRoadUserClass::Person, 0.9, 5.0, 1.0, 0.2, 0.08),
                                    detection(XWalkRoadUserClass::Bicycle, 0.35, 12.0, 0.0, 0.7, 0.02)};
        XWalkRoadUserSafety pipeline(&backend, callbacks());
        const XWalkRoadUserSafetyResult result = pipeline.evaluate();
        EXPECT_EQ(result.risk, XWalkRoadRisk::Warning);
        EXPECT_EQ(result.detectionCount, 2U);
        EXPECT_EQ(backend.alertCount, 1U);
        EXPECT_EQ(backend.stopCount, 0U);
    }

    /** @brief Verifies immediate and multi-user danger requests one motion stop. */
    TEST(XWalkRoadUserSafety, ImmediateOrAggregateDangerStopsMotion)
    {
        ScenarioBackend backend;
        backend.batch.detections = {detection(XWalkRoadUserClass::Car, 0.98, 15.0, 0.0, 0.8, 0.1),
                                    detection(XWalkRoadUserClass::Bus, 0.91, 1.2, 0.5, 0.1, 0.4),
                                    detection(XWalkRoadUserClass::Motorbike, 0.88, 8.0, 0.0, -0.6, 0.03)};
        XWalkRoadUserSafety pipeline(&backend, callbacks());
        const XWalkRoadUserSafetyResult result = pipeline.evaluate();
        EXPECT_EQ(result.risk, XWalkRoadRisk::Dangerous);
        EXPECT_EQ(result.detectionCount, 3U);
        EXPECT_EQ(backend.stopCount, 1U);
        EXPECT_EQ(backend.alertCount, 1U);
    }

    /** @brief Verifies camera, model, timeout, and classifier statuses fail safely. */
    TEST(XWalkRoadUserSafety, ProviderFailuresStopAndReportFault)
    {
        const XWalkRoadSafetyStatus statuses[] = {XWalkRoadSafetyStatus::CameraUnavailable,
                                                  XWalkRoadSafetyStatus::CameraTimeout,
                                                  XWalkRoadSafetyStatus::ModelUnavailable,
                                                  XWalkRoadSafetyStatus::ModelCorrupt,
                                                  XWalkRoadSafetyStatus::InvalidTensor,
                                                  XWalkRoadSafetyStatus::InferenceTimeout,
                                                  XWalkRoadSafetyStatus::ClassifierFailure};
        for (const XWalkRoadSafetyStatus status : statuses)
        {
            ScenarioBackend backend;
            backend.batch.status = status;
            XWalkRoadUserSafety pipeline(&backend, callbacks());
            const XWalkRoadUserSafetyResult result = pipeline.evaluate();
            EXPECT_EQ(result.risk, XWalkRoadRisk::FailSafeStop);
            EXPECT_EQ(result.status, status);
            EXPECT_EQ(backend.stopCount, 1U);
            EXPECT_EQ(backend.faultCount, 1U);
        }
    }

    /** @brief Verifies invalid classes and non-finite output cannot reach classification. */
    TEST(XWalkRoadUserSafety, InvalidDetectorOutputFailsSafe)
    {
        ScenarioBackend backend;
        backend.batch.detections.push_back(
            detection(XWalkRoadUserClass::Person, std::numeric_limits<float64>::quiet_NaN(), 4.0, 1.0, 0.0, 0.1));
        backend.batch.detections.push_back({99, 0.9, 3.0, 1.0, 0.0, 0.2});
        XWalkRoadUserSafety pipeline(&backend, callbacks());
        const XWalkRoadUserSafetyResult result = pipeline.evaluate();
        EXPECT_EQ(result.risk, XWalkRoadRisk::FailSafeStop);
        EXPECT_EQ(result.status, XWalkRoadSafetyStatus::InvalidOutput);
        EXPECT_EQ(backend.classifyCount, 0U);
        EXPECT_EQ(backend.stopCount, 1U);
    }

    /** @brief Verifies detector and classifier failure statuses cannot preserve movement. */
    TEST(XWalkRoadUserSafety, ProviderStatusFailuresFailSafe)
    {
        ScenarioBackend detectorFailure;
        detectorFailure.batch.status = XWalkRoadSafetyStatus::InvalidOutput;
        XWalkRoadUserSafety detectorPipeline(&detectorFailure, callbacks());
        EXPECT_EQ(detectorPipeline.evaluate().risk, XWalkRoadRisk::FailSafeStop);
        EXPECT_EQ(detectorFailure.stopCount, 1U);

        ScenarioBackend classifierFailure;
        classifierFailure.batch.detections.push_back(detection(XWalkRoadUserClass::Person, 0.9, 4.0, 1.0, 0.0, 0.1));
        classifierFailure.classifierStatus = XWalkRoadSafetyStatus::ClassifierFailure;
        XWalkRoadUserSafety classifierPipeline(&classifierFailure, callbacks());
        EXPECT_EQ(classifierPipeline.evaluate().risk, XWalkRoadRisk::FailSafeStop);
        EXPECT_EQ(classifierFailure.stopCount, 1U);
    }

} /* namespace */
