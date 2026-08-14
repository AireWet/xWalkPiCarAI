/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafetyRecordedTest.cpp
 * @brief       Verifies the complete deterministic recorded-video safety flow.
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
#include <cmath>

namespace xwalk::agent::test::road_user_safety
{

/** @brief Verifies recorded frames drive detection, risk, alerts, and safe stopping. */
TEST(XWalkRoadUserSafetyRecorded, CompleteScenarioEndsStoppedAndDisarmed)
{
    RecordedScenarioBackend scenario;
    ASSERT_TRUE(createRecordedScenario(scenario));
    XWalkRoadUserSafety pipeline(&scenario, recordedScenarioCallbacks());

    const XWalkRoadUserSafetyResult emptyRoad = pipeline.evaluate();
    const XWalkRoadUserSafetyResult approachingPerson = pipeline.evaluate();
    const XWalkRoadUserSafetyResult immediateDanger = pipeline.evaluate();
    const XWalkRoadUserSafetyResult lineLoss = pipeline.evaluate();
    const XWalkRoadUserSafetyResult endOfVideo = pipeline.evaluate();

    EXPECT_EQ(scenario.frameIndices, (std::vector<uint32>{0U, 1U, 2U, 3U}));
    ASSERT_EQ(scenario.detectedObjects.size(), 2U);
    EXPECT_EQ(scenario.detectedObjects[0U].classIdentifier,
        static_cast<int32>(XWalkRoadUserClass::Person));
    EXPECT_EQ(scenario.detectedObjects[1U].classIdentifier,
        static_cast<int32>(XWalkRoadUserClass::Person));

    ASSERT_EQ(scenario.classifiedFeatures.size(), 2U);
    EXPECT_GT(scenario.classifiedFeatures[0U].boundingAreaNormalized, 0.03);
    EXPECT_LT(scenario.classifiedFeatures[0U].boundingAreaNormalized, 0.07);
    EXPECT_NEAR(scenario.classifiedFeatures[0U].distanceMeters,
        1.0 / std::sqrt(scenario.classifiedFeatures[0U].boundingAreaNormalized),
        0.000'001);
    EXPECT_NEAR(scenario.classifiedFeatures[0U].absoluteLateralOffset, 0.0, 0.05);
    EXPECT_GT(scenario.classifiedFeatures[1U].boundingAreaNormalized, 0.45);
    EXPECT_LT(scenario.classifiedFeatures[1U].boundingAreaNormalized, 0.55);
    EXPECT_NEAR(scenario.classifiedFeatures[1U].distanceMeters,
        1.0 / std::sqrt(scenario.classifiedFeatures[1U].boundingAreaNormalized),
        0.000'001);
    EXPECT_NEAR(scenario.classifiedFeatures[1U].closingSpeedMetersPerSecond,
        scenario.classifiedFeatures[0U].distanceMeters -
            scenario.classifiedFeatures[1U].distanceMeters,
        0.000'001);

    EXPECT_EQ(emptyRoad.risk, XWalkRoadRisk::Safe);
    EXPECT_EQ(approachingPerson.risk, XWalkRoadRisk::Warning);
    EXPECT_EQ(immediateDanger.risk, XWalkRoadRisk::Dangerous);
    EXPECT_EQ(lineLoss.risk, XWalkRoadRisk::FailSafeStop);
    EXPECT_EQ(lineLoss.status, XWalkRoadSafetyStatus::LineLost);
    EXPECT_EQ(endOfVideo.risk, XWalkRoadRisk::FailSafeStop);
    EXPECT_EQ(endOfVideo.status, XWalkRoadSafetyStatus::EndOfStream);

    EXPECT_TRUE(scenario.redLedActive);
    EXPECT_TRUE(scenario.buzzerActive);
    EXPECT_EQ(scenario.stopCount, 3U);
    EXPECT_FALSE(scenario.robotMoving);
    EXPECT_FALSE(scenario.motorsArmed);
    ASSERT_EQ(scenario.faults.size(), 2U);
    EXPECT_EQ(scenario.faults[0U], XWalkRoadSafetyStatus::LineLost);
    EXPECT_EQ(scenario.faults[1U], XWalkRoadSafetyStatus::EndOfStream);

    destroyRecordedScenario(scenario);
}

} /* namespace xwalk::agent::test::road_user_safety */
