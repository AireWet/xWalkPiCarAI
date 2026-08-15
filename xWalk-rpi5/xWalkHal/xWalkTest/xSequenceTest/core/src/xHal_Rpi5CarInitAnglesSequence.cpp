/******************************************************************************
 * @file        xHal_Rpi5CarInitAnglesSequence.cpp
 * @brief       Implements the three-servo initialization-angle sequence.
 *
 * @details
 * Reproduces the upstream MCU-reset delay and ordered three-servo Robot
 * initialization through hardware-independent HAL abstractions.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarInitAnglesSequence.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

    XWalkInitAnglesSequence::XWalkInitAnglesSequence(XWalkBoardControl& boardControl,
                                                     XWalkRobot& robot,
                                                     XWalkServo& firstServo,
                                                     XWalkServo& secondServo,
                                                     XWalkServo& thirdServo) noexcept
        : boardControlObject(&boardControl), robotObject(&robot), firstServoObject(&firstServo),
          secondServoObject(&secondServo), thirdServoObject(&thirdServo)
    {
    }

    void XWalkInitAnglesSequence::run()
    {
        boardControlObject->resetMcu();
        common::sleepMilliseconds(10U);
        robotObject->addServo(*firstServoObject, 10.0);
        robotObject->addServo(*secondServoObject, 45.0);
        robotObject->addServo(*thirdServoObject, -45.0);
        robotObject->initialize();
    }

} /* namespace xwalk::hal::test */
