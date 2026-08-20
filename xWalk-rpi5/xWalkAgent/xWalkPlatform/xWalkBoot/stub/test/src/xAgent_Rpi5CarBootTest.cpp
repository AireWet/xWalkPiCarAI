/******************************************************************************
 * @file        xAgent_Rpi5CarBootTest.cpp
 * @brief       Verifies device-free xWalkBoot lifecycle behavior.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot Host Stub Test
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBootHostStub.h"
#include "xAgent_Rpi5CarBootTestStub.h"

#include "xHal_Rpi5CarTestFunctions.h"

#include <cassert>

namespace xwalk::agent::test
{

    /**
     * @brief Verifies that the exact simulated service table is forwarded.
     * @param[in,out] context Non-null pointer to an `XWalkBootTestState` object.
     * @param[in] services Simulated services supplied by the host boot stub.
     * @return Deterministic test status value seven.
     */
    agent::int32 runBootTestApplication(agent::contextpointer context, XWalkBootServices& services)
    {
        XWalkBootTestState& state = *static_cast<XWalkBootTestState*>(context);
        ++state.runCount;
        assert(services.doctorLines == nullptr);
        assert(services.picarx == nullptr);
        assert(services.spiTransfer == nullptr);
        assert(services.servoZeroing == nullptr);
        assert(services.lineTracking == nullptr);
        assert(services.selfDrive == nullptr);
        assert(services.music == nullptr);
        assert(services.localVoiceChatbot == nullptr);
        assert(services.treasureHunt == nullptr);
        assert(services.voiceActiveCar == nullptr);
        assert(services.voiceAssistant == nullptr);
        assert(services.voiceStatusLed == nullptr);
        assert(services.cameraCapture == nullptr);
        assert(services.computerVision == nullptr);
        assert(services.faceTracking == nullptr);
        assert(services.bullFight == nullptr);
        assert(services.videoRecording == nullptr);
        return 7;
    }

} /* namespace xwalk::agent::test */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs host-stub callback validation and one-shot lifecycle tests.
 * @return Zero after every assertion passes.
 */
int main()
{
    xwalk::agent::XWalkBootServices services{};
    xwalk::agent::XWalkBootHostStub boot(services);
    xwalk::agent::test::XWalkBootTestState state{};
    const xwalk::agent::xAgentContext invalidContext{&state, nullptr};
    const xwalk::agent::xAgentContext agentContext{&state, &xwalk::agent::test::runBootTestApplication};

    xwalk::hal::test::expectFailure(
        [&]()
        {
            static_cast<void>(boot.run(invalidContext));
        });
    assert(boot.run(agentContext) == 7);
    assert(state.runCount == 1U);

    xwalk::hal::test::expectFailure(
        [&]()
        {
            static_cast<void>(boot.run(agentContext));
        });
    assert(state.runCount == 1U);
    return 0;
}
