#ifndef XAGENT_RPI5CAR_COMPUTER_VISION_OPENCV_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_COMPUTER_VISION_OPENCV_TEST_SUPPORT_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent::test::computer_vision_opencv
{
    /** @brief Owns an isolated recorded-video fixture directory. */
    struct RecordedVideoFixture
    {
            agent::filesystempath directory{};
            agent::filesystempath video{};
            explicit RecordedVideoFixture(agent::boolean colorSequence = false);
            ~RecordedVideoFixture() noexcept;
            RecordedVideoFixture(const RecordedVideoFixture&) = delete;
            RecordedVideoFixture& operator=(const RecordedVideoFixture&) = delete;
    };
} /* namespace xwalk::agent::test::computer_vision_opencv */

#endif /* XAGENT_RPI5CAR_COMPUTER_VISION_OPENCV_TEST_SUPPORT_H */
