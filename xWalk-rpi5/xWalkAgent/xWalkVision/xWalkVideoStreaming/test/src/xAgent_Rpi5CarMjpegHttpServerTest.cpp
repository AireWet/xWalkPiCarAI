/******************************************************************************
 * @file        xAgent_Rpi5CarMjpegHttpServerTest.cpp
 * @brief       Verifies socket-level bounded non-blocking HTTP streaming.
 * @project     xWalk Firmware
 * @module      xWalkVideoStreamingTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xAgent_Rpi5CarMjpegStreamTestSupport.h"

#include <gtest/gtest.h>

#include <thread>

namespace xwalk::agent::test::mjpeg_stream
{

    namespace
    {
        XWalkMjpegHttpConfiguration loopbackConfiguration()
        {
            XWalkMjpegHttpConfiguration configuration;
            configuration.stream.port = availableLoopbackPort();
            return configuration;
        }

        boolean rejectAuthentication(contextpointer, stringview, stringview) noexcept
        {
            return false;
        }
    } /* namespace */

    TEST(XWalkMjpegHttpServer, ValidatesBoundsAndExternalAuthenticationPolicy)
    {
        XWalkMjpegHttpConfiguration configuration = loopbackConfiguration();
        ASSERT_NE(configuration.stream.port, 0U);
        EXPECT_EQ(validateMjpegHttpConfiguration(configuration), XWalkMjpegHttpStatus::Ok);
        configuration.maximumRequestBytes = 63U;
        EXPECT_EQ(validateMjpegHttpConfiguration(configuration), XWalkMjpegHttpStatus::InvalidConfiguration);
        configuration.maximumRequestBytes = 1'024U;
        configuration.stream.bindAddress = "::1";
        EXPECT_EQ(validateMjpegHttpConfiguration(configuration), XWalkMjpegHttpStatus::InvalidConfiguration);
        configuration.stream.bindAddress = "0.0.0.0";
        configuration.stream.allowExternalBind = true;
        EXPECT_EQ(validateMjpegHttpConfiguration(configuration), XWalkMjpegHttpStatus::InvalidConfiguration);
        configuration.authenticationReference = "secret-store://xwalk/mjpeg";
        configuration.authenticate = &rejectAuthentication;
        EXPECT_EQ(validateMjpegHttpConfiguration(configuration), XWalkMjpegHttpStatus::Ok);
    }

    TEST(XWalkMjpegHttpServer, ServesHealthAndStatusOverLoopbackSockets)
    {
        XWalkMjpegStreamState stream;
        XWalkMjpegHttpServer server;
        const XWalkMjpegHttpConfiguration configuration = loopbackConfiguration();
        ASSERT_NE(configuration.stream.port, 0U);
        ASSERT_EQ(startMjpegHttpServer(server, stream, configuration), XWalkMjpegHttpStatus::Ok);
        EXPECT_EQ(mjpegHttpServerPort(server), configuration.stream.port);
        uint64 now{};

        int health = connectLoopback(configuration.stream.port);
        ASSERT_GE(health, 0);
        ASSERT_TRUE(sendRequest(health, "GET /health HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const string healthResponse = collectResponse(server, health, now, 8U);
        EXPECT_NE(healthResponse.find("HTTP/1.1 200 OK"), string::npos);
        EXPECT_NE(healthResponse.find("{\"healthy\":true}"), string::npos);
        closeTestDescriptor(health);

        int status = connectLoopback(configuration.stream.port);
        ASSERT_GE(status, 0);
        ASSERT_TRUE(sendRequest(status, "GET /status HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        const string statusResponse = collectResponse(server, status, now, 8U);
        EXPECT_NE(statusResponse.find("\"camera_available\":true"), string::npos);
        EXPECT_NE(statusResponse.find("\"accepted_clients\":2"), string::npos);
        closeTestDescriptor(status);
        EXPECT_EQ(stopMjpegHttpServer(server), XWalkMjpegHttpStatus::Ok);
        EXPECT_EQ(stopMjpegHttpServer(server), XWalkMjpegHttpStatus::Ok);
    }

    TEST(XWalkMjpegHttpServer, StreamsMultipartFramesWithBoundedQueue)
    {
        XWalkMjpegStreamState stream;
        XWalkMjpegHttpServer server;
        XWalkMjpegHttpConfiguration configuration = loopbackConfiguration();
        configuration.stream.queueCapacity = 2U;
        ASSERT_EQ(startMjpegHttpServer(server, stream, configuration), XWalkMjpegHttpStatus::Ok);
        int client = connectLoopback(configuration.stream.port);
        ASSERT_GE(client, 0);
        ASSERT_TRUE(sendRequest(client, "GET /stream HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        uint64 now{};
        const string header = collectResponse(server, client, now, 4U);
        EXPECT_NE(header.find("multipart/x-mixed-replace"), string::npos);
        ASSERT_EQ(publishMjpegFrame(stream, jpegFrame(0x44U, 32U)), XWalkMjpegStreamStatus::Ok);
        const string frame = collectResponse(server, client, now, 4U);
        EXPECT_NE(frame.find("--xwalk-frame"), string::npos);
        EXPECT_NE(frame.find("Content-Length: 32"), string::npos);
        closeTestDescriptor(client);
        static_cast<void>(pumpMjpegHttpServer(server, ++now));
        EXPECT_EQ(stopMjpegHttpServer(server), XWalkMjpegHttpStatus::Ok);
    }

    TEST(XWalkMjpegHttpServer, RejectsOversizedRequestsAndTimesOutIncompleteHeaders)
    {
        XWalkMjpegStreamState stream;
        XWalkMjpegHttpServer server;
        XWalkMjpegHttpConfiguration configuration = loopbackConfiguration();
        configuration.maximumRequestBytes = 64U;
        configuration.headerTimeoutMilliseconds = 10U;
        ASSERT_EQ(startMjpegHttpServer(server, stream, configuration), XWalkMjpegHttpStatus::Ok);
        uint64 now{};
        int oversized = connectLoopback(configuration.stream.port);
        ASSERT_GE(oversized, 0);
        ASSERT_TRUE(sendRequest(oversized, string(96U, 'x')));
        const string response = collectResponse(server, oversized, now, 4U);
        EXPECT_NE(response.find("431 Request Header Fields Too Large"), string::npos);
        closeTestDescriptor(oversized);

        int incomplete = connectLoopback(configuration.stream.port);
        ASSERT_GE(incomplete, 0);
        ASSERT_EQ(pumpMjpegHttpServer(server, 20U), XWalkMjpegHttpStatus::Ok);
        ASSERT_EQ(pumpMjpegHttpServer(server, 31U), XWalkMjpegHttpStatus::Ok);
        EXPECT_EQ(server.timedOutClients, 1U);
        closeTestDescriptor(incomplete);
        EXPECT_EQ(stopMjpegHttpServer(server), XWalkMjpegHttpStatus::Ok);
    }

    TEST(XWalkMjpegHttpServer, EnforcesClientLimitAndReportsCameraLoss)
    {
        XWalkMjpegStreamState stream;
        XWalkMjpegHttpServer server;
        XWalkMjpegHttpConfiguration configuration = loopbackConfiguration();
        configuration.stream.maximumClients = 1U;
        ASSERT_EQ(startMjpegHttpServer(server, stream, configuration), XWalkMjpegHttpStatus::Ok);
        int first = connectLoopback(configuration.stream.port);
        ASSERT_GE(first, 0);
        ASSERT_EQ(pumpMjpegHttpServer(server, 1U), XWalkMjpegHttpStatus::Ok);
        int second = connectLoopback(configuration.stream.port);
        ASSERT_GE(second, 0);
        ASSERT_EQ(pumpMjpegHttpServer(server, 2U), XWalkMjpegHttpStatus::Ok);
        EXPECT_EQ(server.rejectedClients, 1U);
        closeTestDescriptor(second);
        closeTestDescriptor(first);
        ASSERT_EQ(pumpMjpegHttpServer(server, 3U), XWalkMjpegHttpStatus::Ok);

        EXPECT_EQ(reportMjpegCameraLoss(stream), XWalkMjpegStreamStatus::CameraUnavailable);
        int health = connectLoopback(configuration.stream.port);
        ASSERT_GE(health, 0);
        ASSERT_TRUE(sendRequest(health, "GET /health HTTP/1.1\r\nHost: localhost\r\n\r\n"));
        uint64 now{3U};
        const string response = collectResponse(server, health, now, 8U);
        EXPECT_NE(response.find("503 Service Unavailable"), string::npos);
        closeTestDescriptor(health);
        EXPECT_EQ(stopMjpegHttpServer(server), XWalkMjpegHttpStatus::Ok);
    }

    TEST(XWalkMjpegHttpServer, ReportsPortCollisionAndCleansConnectedClients)
    {
        XWalkMjpegStreamState firstStream;
        XWalkMjpegStreamState secondStream;
        XWalkMjpegHttpServer firstServer;
        XWalkMjpegHttpServer secondServer;
        const XWalkMjpegHttpConfiguration configuration = loopbackConfiguration();
        ASSERT_EQ(startMjpegHttpServer(firstServer, firstStream, configuration), XWalkMjpegHttpStatus::Ok);
        EXPECT_EQ(startMjpegHttpServer(secondServer, secondStream, configuration), XWalkMjpegHttpStatus::SocketFailure);
        int client = connectLoopback(configuration.stream.port);
        ASSERT_GE(client, 0);
        ASSERT_EQ(pumpMjpegHttpServer(firstServer, 1U), XWalkMjpegHttpStatus::Ok);
        EXPECT_EQ(firstServer.clients.size(), 1U);
        EXPECT_EQ(stopMjpegHttpServer(firstServer), XWalkMjpegHttpStatus::Ok);
        EXPECT_TRUE(firstServer.clients.empty());
        EXPECT_FALSE(firstStream.started);
        closeTestDescriptor(client);
    }

    TEST(XWalkFailureObservability, ConcurrentProductionIsBoundedOrderedAndResettable)
    {
        ::xwalk::XWalkFailureObservability observability;
        const auto produce = [&observability](uint64 base)
        {
            for (uint64 index = 0U; index < 100U; ++index)
            {
                ::xwalk::recordFailureEvent(
                    observability, ::xwalk::XWalkFailureEventId::DroppedFrame, base + index, index);
            }
        };
        std::thread first(produce, 0U);
        std::thread second(produce, 1'000U);
        std::thread third(produce, 2'000U);
        std::thread fourth(produce, 3'000U);
        first.join();
        second.join();
        third.join();
        fourth.join();
        const ::xwalk::XWalkFailureSnapshot snapshot = ::xwalk::failureSnapshot(observability);
        ASSERT_EQ(snapshot.eventCount, ::xwalk::XWALK_FAILURE_EVENT_CAPACITY);
        EXPECT_EQ(snapshot.counters[static_cast<size>(::xwalk::XWalkFailureEventId::DroppedFrame)], 400U);
        for (size index = 1U; index < snapshot.eventCount; ++index)
        {
            EXPECT_EQ(snapshot.events[index].sequence, snapshot.events[index - 1U].sequence + 1U);
        }
        ::xwalk::resetFailureObservability(observability);
        const ::xwalk::XWalkFailureSnapshot reset = ::xwalk::failureSnapshot(observability);
        EXPECT_EQ(reset.eventCount, 0U);
        EXPECT_EQ(reset.nextSequence, 1U);
    }

} /* namespace xwalk::agent::test::mjpeg_stream */
