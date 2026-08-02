/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDriveGestures.cpp
 * @brief       Implements the PiCar-X preset gesture sequences.
 *
 * @details
 * Preserves the ordered motor and servo commands and millisecond timing from
 * the upstream preset-actions module.
 *
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive
 *
 * @author      Joxy John
 * @date        2026-07-31
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
#include "xAgent_Rpi5CarSelfDrive.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/
/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Alternates the steering servo to imitate waving hands.
 *
 * @post
 * Steering is centered and camera tilt remains at twenty degrees.
 */
void XWalkSelfDrive::waveHands()
{
    picarxObject->reset();
    picarxObject->setCameraTiltAngle(20.0);
    for (hal::uint32 index = 0U; index < 2U; ++index)
    {
        picarxObject->setDirectionServoAngle(-25.0);
        delay(100U);
        picarxObject->setDirectionServoAngle(25.0);
        delay(100U);
    }
    picarxObject->setDirectionServoAngle(0.0);
}

/**
 * @brief Alternates steering and camera pan to imitate resistance.
 *
 * @post
 * Motors, steering, and camera pan are stopped or centered.
 */
void XWalkSelfDrive::resist()
{
    picarxObject->reset();
    picarxObject->setCameraTiltAngle(10.0);
    for (hal::uint32 index = 0U; index < 3U; ++index)
    {
        picarxObject->setDirectionServoAngle(-15.0);
        picarxObject->setCameraPanAngle(15.0);
        delay(100U);
        picarxObject->setDirectionServoAngle(15.0);
        picarxObject->setCameraPanAngle(-15.0);
        delay(100U);
    }
    picarxObject->stop();
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->setCameraPanAngle(0.0);
}

/**
 * @brief Alternates low-speed drive directions to imitate cute shaking.
 *
 * @post
 * Motors are stopped and camera tilt is centered.
 */
void XWalkSelfDrive::actCute()
{
    picarxObject->reset();
    picarxObject->setCameraTiltAngle(-20.0);
    for (hal::uint32 index = 0U; index < 15U; ++index)
    {
        picarxObject->forward(5.0);
        delay(20U);
        picarxObject->backward(5.0);
        delay(20U);
    }
    picarxObject->setCameraTiltAngle(0.0);
    picarxObject->stop();
}

/**
 * @brief Alternates small steering angles to imitate rubbing hands.
 *
 * @post
 * The PiCar-X coordinator has been reset.
 */
void XWalkSelfDrive::rubHands()
{
    picarxObject->reset();
    for (hal::uint32 index = 0U; index < 5U; ++index)
    {
        picarxObject->setDirectionServoAngle(-6.0);
        delay(500U);
        picarxObject->setDirectionServoAngle(6.0);
        delay(500U);
    }
    picarxObject->reset();
}

/**
 * @brief Executes the thinking pose without restoring the centered pose.
 *
 * @post
 * Pan, tilt, and steering remain at thirty, minus twenty, and twenty degrees.
 */
void XWalkSelfDrive::keepThink()
{
    picarxObject->reset();
    for (hal::uint32 index = 0U; index < 11U; ++index)
    {
        const hal::float64 indexValue = static_cast<hal::float64>(index);
        picarxObject->setCameraPanAngle(indexValue * 3.0);
        picarxObject->setCameraTiltAngle(indexValue * -2.0);
        picarxObject->setDirectionServoAngle(indexValue * 2.0);
        delay(50U);
    }
}

/**
 * @brief Runs the thinking pose and returns to the centered pose.
 *
 * @post
 * The PiCar-X coordinator has been reset.
 */
void XWalkSelfDrive::think()
{
    keepThink();
    delay(1'000U);
    picarxObject->setCameraPanAngle(15.0);
    picarxObject->setCameraTiltAngle(-10.0);
    picarxObject->setDirectionServoAngle(10.0);
    delay(100U);
    picarxObject->reset();
}

/**
 * @brief Oscillates drive, steering, and camera pan together.
 *
 * @post
 * Motors, steering, and camera pan are stopped or centered.
 */
void XWalkSelfDrive::twistBody()
{
    picarxObject->reset();
    for (hal::uint32 index = 0U; index < 3U; ++index)
    {
        picarxObject->setMotorSpeed(1U, 20.0);
        picarxObject->setMotorSpeed(2U, 20.0);
        picarxObject->setCameraPanAngle(-20.0);
        picarxObject->setDirectionServoAngle(-10.0);
        delay(100U);

        picarxObject->setMotorSpeed(1U, 0.0);
        picarxObject->setMotorSpeed(2U, 0.0);
        picarxObject->setCameraPanAngle(0.0);
        picarxObject->setDirectionServoAngle(0.0);
        delay(100U);

        picarxObject->setMotorSpeed(1U, -20.0);
        picarxObject->setMotorSpeed(2U, -20.0);
        picarxObject->setCameraPanAngle(20.0);
        picarxObject->setDirectionServoAngle(10.0);
        delay(100U);

        picarxObject->setMotorSpeed(1U, 0.0);
        picarxObject->setMotorSpeed(2U, 0.0);
        picarxObject->setCameraPanAngle(0.0);
        picarxObject->setDirectionServoAngle(0.0);
        delay(100U);
    }
}

/**
 * @brief Runs the mirrored steering and camera celebration sequence.
 *
 * @post
 * Steering and camera pan are centered while tilt remains at twenty degrees.
 */
void XWalkSelfDrive::celebrate()
{
    picarxObject->reset();
    picarxObject->setCameraTiltAngle(20.0);

    picarxObject->setDirectionServoAngle(30.0);
    picarxObject->setCameraPanAngle(60.0);
    delay(300U);
    picarxObject->setDirectionServoAngle(10.0);
    picarxObject->setCameraPanAngle(30.0);
    delay(100U);
    picarxObject->setDirectionServoAngle(30.0);
    picarxObject->setCameraPanAngle(60.0);
    delay(300U);
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->setCameraPanAngle(0.0);
    delay(200U);

    picarxObject->setDirectionServoAngle(-30.0);
    picarxObject->setCameraPanAngle(-60.0);
    delay(300U);
    picarxObject->setDirectionServoAngle(-10.0);
    picarxObject->setCameraPanAngle(-30.0);
    delay(100U);
    picarxObject->setDirectionServoAngle(-30.0);
    picarxObject->setCameraPanAngle(-60.0);
    delay(300U);
    picarxObject->setDirectionServoAngle(0.0);
    picarxObject->setCameraPanAngle(0.0);
    delay(200U);
}

/**
 * @brief Runs the downward camera-tilt sequence and resets the car.
 *
 * @post
 * The PiCar-X coordinator has been reset.
 */
void XWalkSelfDrive::depressed()
{
    picarxObject->reset();
    picarxObject->setCameraTiltAngle(0.0);
    picarxObject->setCameraTiltAngle(20.0);
    delay(220U);
    picarxObject->setCameraTiltAngle(-22.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(10.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-22.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(0.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-22.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-10.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-22.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-15.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-22.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-19.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-22.0);
    delay(100U);
    delay(1'500U);
    picarxObject->reset();
}

/**
 * @brief Runs the decreasing camera-pan head-shake sequence.
 *
 * @post
 * Motors are stopped and camera pan is centered.
 */
void XWalkSelfDrive::shakeHead()
{
    picarxObject->stop();
    picarxObject->setCameraPanAngle(0.0);
    picarxObject->setCameraPanAngle(60.0);
    delay(200U);
    picarxObject->setCameraPanAngle(-50.0);
    delay(100U);
    picarxObject->setCameraPanAngle(40.0);
    delay(100U);
    picarxObject->setCameraPanAngle(-30.0);
    delay(100U);
    picarxObject->setCameraPanAngle(20.0);
    delay(100U);
    picarxObject->setCameraPanAngle(-10.0);
    delay(100U);
    picarxObject->setCameraPanAngle(10.0);
    delay(100U);
    picarxObject->setCameraPanAngle(-5.0);
    delay(100U);
    picarxObject->setCameraPanAngle(0.0);
}

/**
 * @brief Runs the repeated camera-tilt nod sequence.
 *
 * @post
 * Camera tilt is centered.
 */
void XWalkSelfDrive::nod()
{
    picarxObject->reset();
    picarxObject->setCameraTiltAngle(0.0);
    picarxObject->setCameraTiltAngle(5.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-30.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(5.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(-30.0);
    delay(100U);
    picarxObject->setCameraTiltAngle(0.0);
}

} /* namespace xwalk::agent */
