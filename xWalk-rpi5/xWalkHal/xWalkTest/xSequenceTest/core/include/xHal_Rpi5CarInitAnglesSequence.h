/******************************************************************************
 * @file        xHal_Rpi5CarInitAnglesSequence.h
 * @brief       Declares the three-servo initialization-angle sequence.
 *
 * @details
 * Coordinates MCU reset and Robot initialization for PWM channels 10, 11,
 * and 12 using the upstream angles 10, 45, and -45 degrees.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_INIT_ANGLES_SEQUENCE_H
#define XHAL_RPI5CAR_INIT_ANGLES_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarRobot.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Resets the Robot HAT and initializes three positional servos. */
    class XWalkInitAnglesSequence
    {
        private:
            /** @brief Caller-owned board controller used for the MCU reset pulse. */
            XWalkBoardControl* boardControlObject;
            /** @brief Caller-owned robot receiving the three servo registrations. */
            XWalkRobot* robotObject;
            /** @brief Caller-owned PWM channel 10 servo. */
            XWalkServo* firstServoObject;
            /** @brief Caller-owned PWM channel 11 servo. */
            XWalkServo* secondServoObject;
            /** @brief Caller-owned PWM channel 12 servo. */
            XWalkServo* thirdServoObject;

        public:
            /**
             * @brief Binds the board, robot, and three caller-owned servo objects.
             *
             * @pre
             * Every referenced object outlives this sequence.
             */
            XWalkInitAnglesSequence(XWalkBoardControl& boardControl,
                                    XWalkRobot& robot,
                                    XWalkServo& firstServo,
                                    XWalkServo& secondServo,
                                    XWalkServo& thirdServo) noexcept;

            XWalkInitAnglesSequence(const XWalkInitAnglesSequence&) = delete;
            XWalkInitAnglesSequence(XWalkInitAnglesSequence&&) = delete;
            XWalkInitAnglesSequence& operator=(const XWalkInitAnglesSequence&) = delete;
            XWalkInitAnglesSequence& operator=(XWalkInitAnglesSequence&&) = delete;

            /**
             * @brief Resets the MCU and initializes channels 10, 11, and 12.
             *
             * @post
             * The robot reports logical positions `{10.0, 45.0, -45.0}`.
             */
            void run();
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_INIT_ANGLES_SEQUENCE_H */
