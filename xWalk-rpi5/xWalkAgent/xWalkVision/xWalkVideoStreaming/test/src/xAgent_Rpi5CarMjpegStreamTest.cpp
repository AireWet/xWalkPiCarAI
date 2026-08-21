/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegStreamTest.cpp
 * @brief       Verifies bounded in-process MJPEG lifecycle and backpressure.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreamingTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegStreamTestSupport.h"

#include <gtest/gtest.h>
#include <thread>

namespace xwalk::agent::test::mjpeg_stream
{

    TEST(XWalkVideoStreaming, CoordinatesCameraAndHttpLifecycle)
    {
        XWalkVideoStreamingTestState state;
        XWalkMjpegHttpConfiguration configuration;
        configuration.stream.port = availableLoopbackPort();
        ASSERT_NE(configuration.stream.port, 0U);
        hal::XWalkCameraStream camera(&state, videoStreamingCallbacks());
        XWalkVideoStreaming streaming(camera, &videoClock, configuration);
        EXPECT_TRUE(streaming.start());
        EXPECT_TRUE(streaming.started());
        EXPECT_EQ(streaming.port(), configuration.stream.port);
        EXPECT_TRUE(streaming.step());
        EXPECT_EQ(state.capturedFrames, 1U);
        state.captureAvailable = false;
        EXPECT_FALSE(streaming.step());
        EXPECT_FALSE(streaming.started());
        EXPECT_FALSE(state.cameraStarted);
        EXPECT_EQ(state.stopCount, 1U);
        streaming.stop();
        EXPECT_EQ(state.stopCount, 1U);
    }

    TEST(XWalkVideoStreaming, CleansCameraStartupAndHttpListenerFailures)
    {
        XWalkVideoStreamingTestState startupFailureState;
        startupFailureState.startAvailable = false;
        XWalkMjpegHttpConfiguration startupFailureConfiguration;
        startupFailureConfiguration.stream.port = availableLoopbackPort();
        ASSERT_NE(startupFailureConfiguration.stream.port, 0U);
        hal::XWalkCameraStream startupFailureCamera(&startupFailureState, videoStreamingCallbacks());
        XWalkVideoStreaming startupFailureStreaming(startupFailureCamera, &videoClock, startupFailureConfiguration);
        EXPECT_FALSE(startupFailureStreaming.start());
        EXPECT_FALSE(startupFailureState.cameraStarted);
        EXPECT_GE(startupFailureState.stopCount, 1U);

        XWalkVideoStreamingTestState firstState;
        XWalkVideoStreamingTestState collisionState;
        XWalkMjpegHttpConfiguration collisionConfiguration;
        collisionConfiguration.stream.port = availableLoopbackPort();
        ASSERT_NE(collisionConfiguration.stream.port, 0U);
        hal::XWalkCameraStream firstCamera(&firstState, videoStreamingCallbacks());
        hal::XWalkCameraStream collisionCamera(&collisionState, videoStreamingCallbacks());
        XWalkVideoStreaming firstStreaming(firstCamera, &videoClock, collisionConfiguration);
        XWalkVideoStreaming collisionStreaming(collisionCamera, &videoClock, collisionConfiguration);
        ASSERT_TRUE(firstStreaming.start());
        EXPECT_FALSE(collisionStreaming.start());
        EXPECT_FALSE(collisionState.cameraStarted);
        EXPECT_EQ(collisionState.stopCount, 1U);
        EXPECT_TRUE(firstStreaming.started());
        firstStreaming.stop();
    }

    TEST(XWalkVideoStreaming, RunsUntilCancellationAndCleansOnDestruction)
    {
        XWalkVideoStreamingTestState state;
        XWalkMjpegHttpConfiguration configuration;
        configuration.stream.port = availableLoopbackPort();
        ASSERT_NE(configuration.stream.port, 0U);
        hal::XWalkCameraStream camera(&state, videoStreamingCallbacks());
        {
            XWalkVideoStreaming streaming(camera, &videoClock, configuration);
            ASSERT_TRUE(streaming.start());
            EXPECT_TRUE(streaming.step());
            EXPECT_TRUE(streaming.step());
            EXPECT_TRUE(streaming.started());
            EXPECT_TRUE(state.cameraStarted);
        }
        EXPECT_FALSE(state.cameraStarted);
        EXPECT_EQ(state.stopCount, 1U);
    }

    TEST(XWalkMjpegStream, ValidatesConfigurationAndIdempotentLifecycle)
    {
        XWalkMjpegStreamState state;
        XWalkMjpegStreamConfiguration configuration;
        EXPECT_EQ(startMjpegStream(state, configuration), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(startMjpegStream(state, configuration), XWalkMjpegStreamStatus::Ok);
        EXPECT_TRUE(state.started);
        EXPECT_EQ(stopMjpegStream(state), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(stopMjpegStream(state), XWalkMjpegStreamStatus::Ok);
        EXPECT_FALSE(state.started);

        configuration.port = 0U;
        EXPECT_EQ(validateMjpegStreamConfiguration(configuration), XWalkMjpegStreamStatus::InvalidConfiguration);
        configuration.port = 8080U;
        configuration.bindAddress = "0.0.0.0";
        EXPECT_EQ(validateMjpegStreamConfiguration(configuration), XWalkMjpegStreamStatus::InvalidConfiguration);
        configuration.allowExternalBind = true;
        EXPECT_EQ(validateMjpegStreamConfiguration(configuration), XWalkMjpegStreamStatus::Ok);
        configuration.bindAddress = "bad address";
        EXPECT_EQ(validateMjpegStreamConfiguration(configuration), XWalkMjpegStreamStatus::InvalidConfiguration);
    }

    TEST(XWalkMjpegStream, DeliversOrderedMultipartFramesToMultipleClients)
    {
        XWalkMjpegStreamState state;
        XWalkMjpegStreamConfiguration configuration;
        ASSERT_EQ(startMjpegStream(state, configuration), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(addMjpegStreamClient(state, 1U), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(addMjpegStreamClient(state, 2U), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(publishMjpegFrame(state, jpegFrame(0x11U, 16U)), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(publishMjpegFrame(state, jpegFrame(0x22U, 20U)), XWalkMjpegStreamStatus::Ok);

        bytevector multipart;
        uint64 sequence{};
        EXPECT_EQ(popMjpegMultipartFrame(state, 1U, multipart, sequence), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(sequence, 1U);
        EXPECT_NE(multipartText(multipart).find("Content-Length: 16"), string::npos);
        EXPECT_EQ(popMjpegMultipartFrame(state, 1U, multipart, sequence), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(sequence, 2U);
        EXPECT_EQ(popMjpegMultipartFrame(state, 2U, multipart, sequence), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(sequence, 1U);
        EXPECT_NE(mjpegHttpResponseHeader().find("multipart/x-mixed-replace"), string::npos);
    }

    TEST(XWalkMjpegStream, DropsOldestFramesForSlowClientsWithoutBlockingOthers)
    {
        XWalkMjpegStreamState state;
        XWalkMjpegStreamConfiguration configuration;
        configuration.queueCapacity = 2U;
        ASSERT_EQ(startMjpegStream(state, configuration), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(addMjpegStreamClient(state, 7U), XWalkMjpegStreamStatus::Ok);
        for (uint8 marker = 1U; marker <= 4U; ++marker)
        {
            ASSERT_EQ(publishMjpegFrame(state, jpegFrame(marker, 12U)), XWalkMjpegStreamStatus::Ok);
        }
        ASSERT_EQ(state.clients.size(), 1U);
        EXPECT_EQ(state.clients[0U].pending.size(), 2U);
        EXPECT_EQ(state.clients[0U].droppedFrames, 2U);
        bytevector multipart;
        uint64 sequence{};
        EXPECT_EQ(popMjpegMultipartFrame(state, 7U, multipart, sequence), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(sequence, 3U);
    }

    TEST(XWalkMjpegStream, ConcurrentPublishersRemainBoundedAndSequenceOrdered)
    {
        XWalkMjpegStreamState state;
        XWalkMjpegStreamConfiguration configuration;
        configuration.queueCapacity = 16U;
        ASSERT_EQ(startMjpegStream(state, configuration), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(addMjpegStreamClient(state, 9U), XWalkMjpegStreamStatus::Ok);

        const auto publishFrames = [&state](uint8 marker)
        {
            for (uint32 index = 0U; index < 100U; ++index)
            {
                EXPECT_EQ(publishMjpegFrame(state, jpegFrame(marker, 12U)), XWalkMjpegStreamStatus::Ok);
            }
        };
        std::thread firstPublisher(publishFrames, 0x31U);
        std::thread secondPublisher(publishFrames, 0x32U);
        firstPublisher.join();
        secondPublisher.join();

        ASSERT_EQ(state.clients.size(), 1U);
        EXPECT_EQ(state.clients[0U].pending.size(), 16U);
        EXPECT_EQ(state.clients[0U].droppedFrames, 184U);
        uint64 previousSequence{};
        bytevector multipart;
        for (uint32 index = 0U; index < 16U; ++index)
        {
            uint64 sequence{};
            ASSERT_EQ(popMjpegMultipartFrame(state, 9U, multipart, sequence), XWalkMjpegStreamStatus::Ok);
            EXPECT_GT(sequence, previousSequence);
            previousSequence = sequence;
        }
        EXPECT_EQ(previousSequence, 200U);
    }

    TEST(XWalkMjpegStream, HandlesDisconnectCameraLossAndInvalidFramesSafely)
    {
        XWalkMjpegStreamState state;
        EXPECT_EQ(publishMjpegFrame(state, jpegFrame(1U, 8U)), XWalkMjpegStreamStatus::NotStarted);
        XWalkMjpegStreamConfiguration configuration;
        ASSERT_EQ(startMjpegStream(state, configuration), XWalkMjpegStreamStatus::Ok);
        ASSERT_EQ(addMjpegStreamClient(state, 3U), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(publishMjpegFrame(state, bytevector{1U, 2U, 3U, 4U}), XWalkMjpegStreamStatus::InvalidFrame);
        ASSERT_EQ(publishMjpegFrame(state, jpegFrame(3U, 8U)), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(reportMjpegCameraLoss(state), XWalkMjpegStreamStatus::CameraUnavailable);
        EXPECT_TRUE(state.clients[0U].pending.empty());
        EXPECT_EQ(publishMjpegFrame(state, jpegFrame(4U, 8U)), XWalkMjpegStreamStatus::CameraUnavailable);
        EXPECT_EQ(removeMjpegStreamClient(state, 3U), XWalkMjpegStreamStatus::Ok);
        EXPECT_EQ(removeMjpegStreamClient(state, 3U), XWalkMjpegStreamStatus::ClientNotFound);
        EXPECT_EQ(stopMjpegStream(state), XWalkMjpegStreamStatus::Ok);
    }

} /* namespace xwalk::agent::test::mjpeg_stream */
