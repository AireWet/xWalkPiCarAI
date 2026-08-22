/******************************************************************************
 * @file        xControllerSchedulerSignalTest.cpp
 * @brief       Verifies native CBB signals and module dispatch.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Scheduler Test
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#include "xAgentCbb.h"
#include "xControllerCbb.h"
#include "xControllerSchedulerTestSupport.h"
#include "xHalCbb.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <cstring>
#include <gtest/gtest.h>

namespace xwalk::ctrl::test::scheduler
{

    static ::ctrl::int32 writeModule(NativeChainContext* context, ::ctrl::uint32 signalNumber) noexcept
    {
        if (context == nullptr)
        {
            return XWALK_SCHEDULER_INVALID_ARGUMENT;
        }
        const ::ctrl::uint32 event[2U]{static_cast<::ctrl::uint32>(context->moduleId), signalNumber};
        return ::write(context->eventDescriptor, event, sizeof(event)) == static_cast<ssize_t>(sizeof(event))
                   ? XWALK_SCHEDULER_OK
                   : XWALK_SCHEDULER_IPC_FAILURE;
    }

    static ::ctrl::int32 ctrlHandler(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        auto* chain = static_cast<NativeChainContext*>(context);
        const ::ctrl::uint32 signalNumber = cxx_xWalkCtrlGetSignal_LPP(signal);
        const ::ctrl::int32 writeStatus = writeModule(chain, signalNumber);
        if (writeStatus != XWALK_SCHEDULER_OK)
        {
            return XWALK_SCHEDULER_IPC_FAILURE;
        }
        const xWalkEncodedPayload payload{signal->payload, signal->payloadSize};
        return cxx_xWalkAgentSend_LPP(&payload, CXX_XWALK_NO_ARG_REQ);
    }

    static ::ctrl::int32 agentHandler(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        auto* chain = static_cast<NativeChainContext*>(context);
        const ::ctrl::uint32 signalNumber = cxx_xWalkAgentGetSignal_LPP(signal);
        const ::ctrl::int32 writeStatus = writeModule(chain, signalNumber);
        if (writeStatus != XWALK_SCHEDULER_OK)
        {
            return XWALK_SCHEDULER_IPC_FAILURE;
        }
        const xWalkEncodedPayload payload{signal->payload, signal->payloadSize};
        return cxx_xWalkHalSend_LPP(&payload, CXX_XWALK_I2C_REQ);
    }

    static ::ctrl::int32 halHandler(::ctrl::contextpointer context, const XWalkSignal* signal) noexcept
    {
        auto* chain = static_cast<NativeChainContext*>(context);
        return writeModule(chain, cxx_xWalkHalGetSignal_LPP(signal));
    }

    TEST(XWalkSchedulerSignalTest, StableRegistryValuesRemainExact)
    {
        EXPECT_EQ(CXX_XWALK_SIGNAL_UNSPECIFIED, 0U);
        EXPECT_EQ(CXX_XWALK_I2C_REQ, 0x1081U);
        EXPECT_EQ(CXX_XWALK_I2C_CFM, 0x1082U);
        EXPECT_EQ(CXX_XWALK_I2C_REJ, 0x1083U);
        EXPECT_EQ(CXX_XWALK_CMD_MOVE_REQ, 0x200AU);
        EXPECT_EQ(CXX_XWALK_CMD_MOVE_CFM, 0x210AU);
        EXPECT_EQ(CXX_XWALK_CMD_MOVE_REJ, 0x220AU);
        EXPECT_EQ(CXX_XWALK_MOVE_REQ, 0x2086U);
        EXPECT_EQ(CXX_XWALK_MOVE_CFM, 0x2186U);
        EXPECT_EQ(CXX_XWALK_MOVE_REJ, 0x2286U);

        for (::ctrl::uint32 request = CXX_XWALK_CMD_UNKNOWN_REQ; request <= CXX_XWALK_CMD_VIDEO_STREAM_REQ; ++request)
        {
            EXPECT_EQ(cxx_xWalkGetConfirmationSignal_LPP(request), request + 0x100U);
            EXPECT_EQ(cxx_xWalkGetRejectionSignal_LPP(request), request + 0x200U);
        }
        for (::ctrl::uint32 request = CXX_XWALK_CTRL_CMD_REQ; request <= CXX_XWALK_SOUND_REQ; ++request)
        {
            EXPECT_EQ(cxx_xWalkGetConfirmationSignal_LPP(request), request + 0x100U);
            EXPECT_EQ(cxx_xWalkGetRejectionSignal_LPP(request), request + 0x200U);
        }
        EXPECT_EQ(cxx_xWalkGetConfirmationSignal_LPP(CXX_XWALK_APP_CFG_REQ), CXX_XWALK_SIGNAL_UNSPECIFIED);
        EXPECT_EQ(cxx_xWalkGetRejectionSignal_LPP(CXX_XWALK_SERVO_CAL_CFG_REQ), CXX_XWALK_SIGNAL_UNSPECIFIED);
    }

    TEST(XWalkSchedulerSignalTest, ModuleGetSetAndBinaryPayloadAreNative)
    {
        XWalkSignal signal{};
        const xClientAddress address{XWALK_AGENT_MAILBOX_ID, "agent", 7U, 2U};
        ASSERT_EQ(cxx_xWalkAgentSetClientInfo_LPP(&signal, &address), XWALK_SCHEDULER_OK);
        ASSERT_EQ(cxx_xWalkAgentSetSignal_LPP(&signal, CXX_XWALK_MOVE_REQ), XWALK_SCHEDULER_OK);
        const ::ctrl::uint8 bytes[]{0x41U, 0x00U, 0x42U};
        const xWalkEncodedPayload input{bytes, sizeof(bytes)};
        ASSERT_EQ(cxx_xWalkAgentSetPayload_LPP(&signal, &input), XWALK_SCHEDULER_OK);
        EXPECT_EQ(cxx_xWalkAgentGetSignal_LPP(&signal), CXX_XWALK_MOVE_REQ);

        ::ctrl::uint8 outputBytes[sizeof(bytes)]{};
        xWalkPayloadBuffer output{outputBytes, sizeof(outputBytes), 0U};
        ASSERT_EQ(cxx_xWalkAgentGetPayload_LPP(&signal, &output), XWALK_SCHEDULER_OK);
        EXPECT_EQ(output.size, sizeof(bytes));
        EXPECT_EQ(std::memcmp(bytes, outputBytes, sizeof(bytes)), 0);
        EXPECT_EQ(cxx_xWalkCtrlGetSignal_LPP(&signal), CXX_XWALK_SIGNAL_UNSPECIFIED);
    }

    TEST(XWalkSchedulerSignalTest, ControllerAndHalInterfacesRejectWrongModuleAndSmallBuffers)
    {
        XWalkSignal controller{};
        const xClientAddress controllerAddress{XWALK_CTRL_MAILBOX_ID, "controller", 1U, 1U};
        ASSERT_EQ(cxx_xWalkCtrlSetClientInfo_LPP(&controller, &controllerAddress), XWALK_SCHEDULER_OK);
        ASSERT_EQ(cxx_xWalkCtrlSetSignal_LPP(&controller, CXX_XWALK_CMD_CAMERA_REQ), XWALK_SCHEDULER_OK);
        EXPECT_EQ(cxx_xWalkHalSetSignal_LPP(&controller, CXX_XWALK_I2C_REQ), XWALK_SCHEDULER_INVALID_ARGUMENT);

        const ::ctrl::uint8 bytes[]{0x10U, 0x00U, 0x20U};
        const xWalkEncodedPayload input{bytes, sizeof(bytes)};
        ASSERT_EQ(cxx_xWalkCtrlSetPayload_LPP(&controller, &input), XWALK_SCHEDULER_OK);
        ::ctrl::uint8 tooSmall[2U]{};
        xWalkPayloadBuffer output{tooSmall, sizeof(tooSmall), 0U};
        EXPECT_EQ(cxx_xWalkCtrlGetPayload_LPP(&controller, &output), XWALK_SCHEDULER_BUFFER_TOO_SMALL);
        EXPECT_EQ(output.size, sizeof(bytes));

        XWalkSignal hal{};
        const xClientAddress halAddress{XWALK_HAL_MAILBOX_ID, "hal", 2U, 3U};
        ASSERT_EQ(cxx_xWalkHalSetClientInfo_LPP(&hal, &halAddress), XWALK_SCHEDULER_OK);
        ASSERT_EQ(cxx_xWalkHalSetSignal_LPP(&hal, CXX_XWALK_I2C_REQ), XWALK_SCHEDULER_OK);
        EXPECT_EQ(cxx_xWalkHalGetSignal_LPP(&hal), CXX_XWALK_I2C_REQ);
        EXPECT_EQ(cxx_xWalkAgentGetSignal_LPP(&hal), CXX_XWALK_SIGNAL_UNSPECIFIED);
    }

    TEST(XWalkSchedulerSignalTest, NativeControllerAgentHalChainUsesScheduler)
    {
        ::ctrl::int32 eventPipe[2U]{-1, -1};
        ASSERT_EQ(::pipe2(eventPipe, O_CLOEXEC), 0);
        NativeChainContext ctrlContext{eventPipe[1U], XWALK_MODULE_CTRL};
        NativeChainContext agentContext{eventPipe[1U], XWALK_MODULE_AGENT};
        NativeChainContext halContext{eventPipe[1U], XWALK_MODULE_HAL};

        ASSERT_EQ(cxx_xWalkCtrlInit_LPP(&ctrlContext, &ctrlHandler, 1U), XWALK_SCHEDULER_OK);
        ASSERT_EQ(cxx_xWalkAgentInit_LPP(&agentContext, &agentHandler, 2U), XWALK_SCHEDULER_OK);
        ASSERT_EQ(cxx_xWalkHalInit_LPP(&halContext, &halHandler, 3U), XWALK_SCHEDULER_OK);
        const ::ctrl::uint8 value[]{0x5AU};
        const xWalkEncodedPayload payload{value, sizeof(value)};
        ASSERT_EQ(cxx_xWalkCtrlSend_LPP(&payload, CXX_XWALK_CMD_MOVE_REQ), XWALK_SCHEDULER_OK);

        ::ctrl::int32 result{0};
        EXPECT_EQ(cxx_xWalkWait_LPP(XWALK_CTRL_MAILBOX_ID, CXX_XWALK_CMD_MOVE_REQ, &result), XWALK_SCHEDULER_OK);
        EXPECT_EQ(cxx_xWalkWait_LPP(XWALK_AGENT_MAILBOX_ID, CXX_XWALK_NO_ARG_REQ, &result), XWALK_SCHEDULER_OK);
        EXPECT_EQ(cxx_xWalkWait_LPP(XWALK_HAL_MAILBOX_ID, CXX_XWALK_I2C_REQ, &result), XWALK_SCHEDULER_OK);

        const ::ctrl::uint32 expected[3U][2U]{{XWALK_MODULE_CTRL, CXX_XWALK_CMD_MOVE_REQ},
                                              {XWALK_MODULE_AGENT, CXX_XWALK_NO_ARG_REQ},
                                              {XWALK_MODULE_HAL, CXX_XWALK_I2C_REQ}};
        for (::ctrl::size index = 0U; index < 3U; ++index)
        {
            ::ctrl::uint32 event[2U]{};
            ASSERT_EQ(::read(eventPipe[0U], event, sizeof(event)), static_cast<ssize_t>(sizeof(event)));
            EXPECT_EQ(event[0U], expected[index][0U]);
            EXPECT_EQ(event[1U], expected[index][1U]);
        }
        cxx_xWalkClose_LPP();
        static_cast<void>(::close(eventPipe[0U]));
        static_cast<void>(::close(eventPipe[1U]));
    }

} /* namespace xwalk::ctrl::test::scheduler */
