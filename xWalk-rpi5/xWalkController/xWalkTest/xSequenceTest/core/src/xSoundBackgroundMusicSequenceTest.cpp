/******************************************************************************
 * @file        xSoundBackgroundMusicSequenceTest.cpp
 * @brief       Verifies every interactive sound-and-music key transition.
 * @project     xWalk Firmware
 * @module      xWalk CLI Sequence Test
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xControllerCommandTestSupport.h"
#include "xControllerCommands.h"

#include <cassert>

namespace
{

void testSoundBackgroundMusic(
    xwalk::agent::test::ControllerCommandTestContext& context)
{
    context.state->inputLines = {"q", " ", "c", "q", "z", "x"};
    const ctrl::int32 status =
        xwalk::ctrl::XWALK_runControllerCommand(*context.soundBackgroundMusicController,
            {"sound-background-music"});

    assert(status == 0);
    assert(context.state->soundVolume.has_value());
    assert(*context.state->soundVolume == 0.2);
    assert(context.state->backgroundMusicFile.find(
        "slow-trail-Ahjay_Stelino.mp3") != ctrl::string::npos);
    assert(context.state->delays.size() == 6U);
    assert(context.state->delays[0U] == 20U);
    assert(context.state->delays[1U] == 20U);
    assert(context.state->delays[2U] == 10U);

    assert(xwalk::agent::test::containsOrderedEvents(context.state->eventLog,
        {"hal.music.volume", "hal.music.play", "hal.music.sound",
            "controller.delay", "hal.music.sound", "controller.delay",
            "hal.music.control"}));
}

} /* namespace */

int xWalkSoundBackgroundMusicCommandSequenceHostTest(int argc, char* argv[])
{
    return xwalk::agent::test::runControllerCommandHostTest(
        argc, argv, &testSoundBackgroundMusic);
}
