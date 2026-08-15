/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafetyAssetTest.cpp
 * @brief       Verifies committed recorded-scenario media decoding.
 * @project     xWalk Firmware
 * @module      xWalkRoadUserSafetyTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarRoadUserSafetyTestSupport.h"

#include <gtest/gtest.h>

#include <array>

namespace xwalk::agent::test::road_user_safety
{

    namespace
    {
        constexpr std::array<const char*, 12U> SCENARIOS{"pedestrian_entering_crosswalk",
                                                         "pedestrian_standing_safely",
                                                         "vehicle_approaching_pedestrian",
                                                         "bicycle_crossing",
                                                         "multiple_road_users",
                                                         "partial_occlusion",
                                                         "poor_lighting",
                                                         "motion_blur",
                                                         "empty_road",
                                                         "false_positive_challenge",
                                                         "camera_interruption",
                                                         "end_of_video"};

        filesystempath assetRoot()
        {
            return filesystempath(XWALK_RECORDED_ASSET_DIRECTORY);
        }
    } /* namespace */

    TEST(XWalkRoadUserSafetyAssets, AllReviewedScenariosDecodeDeterministically)
    {
        for (const char* scenario : SCENARIOS)
        {
            const filesystempath path = assetRoot() / "recorded_scenarios" / scenario / "scenario.avi";
            const RecordedAssetSummary first = decodeRecordedAsset(path);
            const RecordedAssetSummary second = decodeRecordedAsset(path);
            ASSERT_TRUE(first.opened) << scenario;
            EXPECT_EQ(first.frameCount, 5U) << scenario;
            EXPECT_EQ(first.width, 320U) << scenario;
            EXPECT_EQ(first.height, 240U) << scenario;
            EXPECT_TRUE(first.monotonicOrder) << scenario;
            EXPECT_TRUE(first.cleanEndOfVideo) << scenario;
            EXPECT_EQ(first.frameCount, second.frameCount) << scenario;
            EXPECT_EQ(first.contentHash, second.contentHash) << scenario;
        }
    }

    TEST(XWalkRoadUserSafetyAssets, MalformedImagesFailWithoutProducingFrames)
    {
        for (const char* filename : {"truncated.jpg", "text-as-image.jpg"})
        {
            const RecordedAssetSummary summary = decodeRecordedAsset(assetRoot() / "malformed_images" / filename);
            EXPECT_EQ(summary.frameCount, 0U) << filename;
        }
    }

} /* namespace xwalk::agent::test::road_user_safety */
