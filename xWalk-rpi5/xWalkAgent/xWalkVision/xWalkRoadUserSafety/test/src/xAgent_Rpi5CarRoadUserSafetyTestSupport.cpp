/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafetyTestSupport.cpp
 * @brief       Implements deterministic road-user safety scenario support.
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

#include <opencv2/imgproc.hpp>

#include <chrono>
#include <cmath>
#include <filesystem>

namespace xwalk::agent::test::road_user_safety
{

    namespace
    {
        constexpr int32 RECORDED_FRAME_WIDTH{64};
        constexpr int32 RECORDED_FRAME_HEIGHT{48};
        constexpr uint32 RECORDED_FRAME_COUNT{4U};

        /** @brief Applies the deterministic classifier contract to validated features. */
        XWalkRoadRiskClassification classifyFeatures(const XWalkRoadRiskFeatures& features) noexcept
        {
            const boolean immediatePathRisk =
                (features.absoluteLateralOffset <= 0.35) &&
                ((features.distanceMeters <= 1.5) ||
                 ((features.closingSpeedMetersPerSecond > 0.0) &&
                  ((features.distanceMeters / features.closingSpeedMetersPerSecond) <= 1.0)));
            if (immediatePathRisk)
            {
                return {XWalkRoadSafetyStatus::Ok, XWalkRoadRisk::Dangerous};
            }
            const boolean warning = (features.confidence < 0.5) ||
                                    ((features.absoluteLateralOffset <= 0.5) && (features.distanceMeters <= 6.0) &&
                                     (features.closingSpeedMetersPerSecond > 0.5));
            return {XWalkRoadSafetyStatus::Ok, warning ? XWalkRoadRisk::Warning : XWalkRoadRisk::Safe};
        }

        /** @brief Decodes the lossy-video-resistant marker stored in the frame corner. */
        uint32 recordedFrameIndex(const cv::Mat& frame) noexcept
        {
            const cv::Scalar marker = cv::mean(frame(cv::Rect(0, 0, 8, 8)));
            if (marker[0] < 40.0)
            {
                return 0U;
            }
            if (marker[0] < 80.0)
            {
                return 1U;
            }
            if (marker[0] < 120.0)
            {
                return 2U;
            }
            return 3U;
        }

        /** @brief Reports whether the bottom scan row contains the generated white line. */
        boolean recordedLinePresent(const cv::Mat& frame)
        {
            cv::Mat lineMask;
            cv::inRange(
                frame.row(frame.rows - 1), cv::Scalar(180.0, 180.0, 180.0), cv::Scalar(255.0, 255.0, 255.0), lineMask);
            return cv::countNonZero(lineMask) >= (frame.cols / 2);
        }

        /** @brief Extracts one person detection from the generated red rectangle. */
        boolean
        recordedPersonDetection(RecordedScenarioBackend& scenario, const cv::Mat& frame, XWalkRoadUserDetection& output)
        {
            cv::Mat personMask;
            cv::inRange(frame, cv::Scalar(0.0, 0.0, 150.0), cv::Scalar(90.0, 90.0, 255.0), personMask);
            std::vector<std::vector<cv::Point>> contours;
            cv::findContours(personMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
            if (contours.empty())
            {
                return false;
            }
            cv::Rect bounds;
            float64 largestArea{};
            for (const std::vector<cv::Point>& contour : contours)
            {
                const float64 area = cv::contourArea(contour);
                if (area > largestArea)
                {
                    largestArea = area;
                    bounds = cv::boundingRect(contour);
                }
            }
            const float64 frameArea = static_cast<float64>(frame.cols * frame.rows);
            const float64 boundingArea = static_cast<float64>(bounds.width * bounds.height) / frameArea;
            const float64 distance = 1.0 / std::sqrt(boundingArea);
            float64 closingSpeed{1.0};
            if (scenario.previousDistanceValid)
            {
                closingSpeed = scenario.previousDistanceMeters - distance;
            }
            scenario.previousDistanceValid = true;
            scenario.previousDistanceMeters = distance;
            const float64 centerX = static_cast<float64>(bounds.x) + (static_cast<float64>(bounds.width) / 2.0);
            const float64 lateralOffset =
                (centerX - (static_cast<float64>(frame.cols) / 2.0)) / (static_cast<float64>(frame.cols) / 2.0);
            output = detection(XWalkRoadUserClass::Person, 0.95, distance, closingSpeed, lateralOffset, boundingArea);
            return true;
        }

        /** @brief Acquires and detects one frame from the generated recorded scenario. */
        XWalkRoadUserDetectionBatch detectRecordedScenario(contextpointer context) noexcept
        {
            RecordedScenarioBackend& scenario = *static_cast<RecordedScenarioBackend*>(context);
            cv::Mat frame;
            if (!scenario.capture.read(frame) || frame.empty())
            {
                return {XWalkRoadSafetyStatus::EndOfStream, {}};
            }
            const uint32 frameIndex = recordedFrameIndex(frame);
            scenario.frameIndices.push_back(frameIndex);
            if (!recordedLinePresent(frame))
            {
                return {XWalkRoadSafetyStatus::LineLost, {}};
            }
            XWalkRoadUserDetection person;
            if (!recordedPersonDetection(scenario, frame, person))
            {
                return {XWalkRoadSafetyStatus::Ok, {}};
            }
            scenario.detectedObjects.push_back(person);
            return {XWalkRoadSafetyStatus::Ok, {person}};
        }

        /** @brief Records and classifies one image-derived feature vector. */
        XWalkRoadRiskClassification classifyRecordedScenario(contextpointer context,
                                                             const XWalkRoadRiskFeatures& features) noexcept
        {
            RecordedScenarioBackend& scenario = *static_cast<RecordedScenarioBackend*>(context);
            scenario.classifiedFeatures.push_back(features);
            return classifyFeatures(features);
        }

        /** @brief Activates simulated danger indicators for dangerous results. */
        void alertRecordedScenario(contextpointer context, XWalkRoadRisk risk, uint32 detectionCount) noexcept
        {
            RecordedScenarioBackend& scenario = *static_cast<RecordedScenarioBackend*>(context);
            ++scenario.alertCount;
            if ((risk == XWalkRoadRisk::Dangerous) && (detectionCount > 0U))
            {
                scenario.redLedActive = true;
                scenario.buzzerActive = true;
            }
        }

        /** @brief Establishes the simulated stopped and disarmed state. */
        void stopRecordedScenario(contextpointer context) noexcept
        {
            RecordedScenarioBackend& scenario = *static_cast<RecordedScenarioBackend*>(context);
            ++scenario.stopCount;
            scenario.robotMoving = false;
            scenario.motorsArmed = false;
        }

        /** @brief Records one recorded-pipeline failure status. */
        void faultRecordedScenario(contextpointer context, XWalkRoadSafetyStatus status) noexcept
        {
            RecordedScenarioBackend& scenario = *static_cast<RecordedScenarioBackend*>(context);
            scenario.faults.push_back(status);
        }
    } /* namespace */

    /** @brief Resolves one non-null test context. */
    ScenarioBackend& backend(contextpointer context)
    {
        return *static_cast<ScenarioBackend*>(context);
    }

    /** @brief Returns the configured synthetic detector result. */
    XWalkRoadUserDetectionBatch detect(contextpointer context) noexcept
    {
        ScenarioBackend& value = backend(context);
        ++value.detectCount;
        return value.batch;
    }

    /** @brief Applies deterministic distance, motion, confidence, and path rules. */
    XWalkRoadRiskClassification classify(contextpointer context, const XWalkRoadRiskFeatures& features) noexcept
    {
        ScenarioBackend& value = backend(context);
        ++value.classifyCount;
        if (value.classifierStatus != XWalkRoadSafetyStatus::Ok)
        {
            return {value.classifierStatus, XWalkRoadRisk::Safe};
        }
        return classifyFeatures(features);
    }

    /** @brief Records one non-safe operator alert. */
    void alert(contextpointer context, XWalkRoadRisk risk, uint32 detectionCount) noexcept
    {
        ScenarioBackend& value = backend(context);
        ++value.alertCount;
        value.lastAlert = risk;
        static_cast<void>(detectionCount);
    }

    /** @brief Records one stop-and-disarm request. */
    void stopMotion(contextpointer context) noexcept
    {
        ++backend(context).stopCount;
    }

    /** @brief Records one fail-safe status. */
    void reportFault(contextpointer context, XWalkRoadSafetyStatus status) noexcept
    {
        ScenarioBackend& value = backend(context);
        ++value.faultCount;
        value.lastFault = status;
    }

    XWalkRoadUserSafetyCallbacks callbacks() noexcept
    {
        return {&detect, &classify, &alert, &stopMotion, &reportFault};
    }

    XWalkRoadUserDetection detection(XWalkRoadUserClass roadUserClass,
                                     float64 confidence,
                                     float64 distanceMeters,
                                     float64 closingSpeedMetersPerSecond,
                                     float64 lateralOffsetNormalized,
                                     float64 boundingAreaNormalized)
    {
        return {static_cast<int32>(roadUserClass),
                confidence,
                distanceMeters,
                closingSpeedMetersPerSecond,
                lateralOffsetNormalized,
                boundingAreaNormalized};
    }

    boolean createRecordedScenario(RecordedScenarioBackend& scenario) noexcept
    {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();
        std::error_code error;
        const filesystempath temporaryDirectory = std::filesystem::temp_directory_path(error);
        if (error)
        {
            return false;
        }
        scenario.directory = temporaryDirectory / (string("xwalk-road-scenario-") + std::to_string(timestamp));
        scenario.videoPath = scenario.directory / "road-user-safety.avi";
        if (!std::filesystem::create_directories(scenario.directory, error) || error)
        {
            return false;
        }
        cv::VideoWriter writer(scenario.videoPath.string(),
                               cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                               10.0,
                               cv::Size(RECORDED_FRAME_WIDTH, RECORDED_FRAME_HEIGHT));
        if (!writer.isOpened())
        {
            return false;
        }
        for (uint32 index = 0U; index < RECORDED_FRAME_COUNT; ++index)
        {
            const float64 background = 20.0 + (40.0 * static_cast<float64>(index));
            cv::Mat frame(
                RECORDED_FRAME_HEIGHT, RECORDED_FRAME_WIDTH, CV_8UC3, cv::Scalar(background, background, background));
            if (index != 3U)
            {
                cv::rectangle(frame,
                              cv::Rect(0, RECORDED_FRAME_HEIGHT - 2, RECORDED_FRAME_WIDTH, 2),
                              cv::Scalar(255.0, 255.0, 255.0),
                              cv::FILLED);
            }
            if (index == 1U)
            {
                cv::rectangle(frame, cv::Rect(27, 14, 10, 12), cv::Scalar(0.0, 0.0, 255.0), cv::FILLED);
            }
            else if (index == 2U)
            {
                cv::rectangle(frame, cv::Rect(15, 3, 34, 42), cv::Scalar(0.0, 0.0, 255.0), cv::FILLED);
            }
            writer.write(frame);
        }
        writer.release();
        scenario.capture.open(scenario.videoPath.string(), cv::CAP_ANY);
        return scenario.capture.isOpened();
    }

    void destroyRecordedScenario(RecordedScenarioBackend& scenario) noexcept
    {
        scenario.capture.release();
        std::error_code error;
        static_cast<void>(std::filesystem::remove_all(scenario.directory, error));
    }

    XWalkRoadUserSafetyCallbacks recordedScenarioCallbacks() noexcept
    {
        return {&detectRecordedScenario,
                &classifyRecordedScenario,
                &alertRecordedScenario,
                &stopRecordedScenario,
                &faultRecordedScenario};
    }

    RecordedAssetSummary decodeRecordedAsset(const filesystempath& path) noexcept
    {
        RecordedAssetSummary summary;
        cv::VideoCapture capture(path.string(), cv::CAP_ANY);
        summary.opened = capture.isOpened();
        if (!summary.opened)
        {
            return summary;
        }
        float64 previousPosition{-1.0};
        cv::Mat frame;
        while (capture.read(frame) && !frame.empty())
        {
            const float64 position = capture.get(cv::CAP_PROP_POS_FRAMES);
            if (position <= previousPosition)
            {
                summary.monotonicOrder = false;
            }
            previousPosition = position;
            if (summary.frameCount == 0U)
            {
                summary.width = static_cast<uint32>(frame.cols);
                summary.height = static_cast<uint32>(frame.rows);
            }
            else if ((summary.width != static_cast<uint32>(frame.cols)) ||
                     (summary.height != static_cast<uint32>(frame.rows)))
            {
                summary.monotonicOrder = false;
            }
            const size dataBytes = frame.total() * frame.elemSize();
            const uint8* data = frame.ptr<uint8>(0);
            for (size index = 0U; index < dataBytes; ++index)
            {
                summary.contentHash ^= data[index];
                summary.contentHash *= 1'099'511'628'211ULL;
            }
            ++summary.frameCount;
        }
        summary.cleanEndOfVideo = frame.empty();
        capture.release();
        return summary;
    }

} /* namespace xwalk::agent::test::road_user_safety */
