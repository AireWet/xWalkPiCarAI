/******************************************************************************
 * @file        xHal_Rpi5CarMotorSequence.h
 * @brief       Declares the bounded two-motor Robot HAT sequence.
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

#ifndef XHAL_RPI5CAR_MOTOR_SEQUENCE_H
#define XHAL_RPI5CAR_MOTOR_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMotor.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Maximum bounded repetitions accepted by the physical sequence. */
#define XHAL_RPI5CAR_MOTOR_SEQUENCE_MAX_CYCLES 100U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal::test
{

    /** @brief Wait operation injected for host and hardware execution. */
    using motortestwaitcallback = void (*)(contextpointer context, uint32 durationMilliseconds);

    /** @brief Alternates two PWM-and-direction motors and guarantees a final stop. */
    class XWalkMotorSequence
    {
        private:
            /** @brief Caller-owned motors corresponding to P13/D4 and P12/D5. */
            fixedarray<XWalkMotor*, 2U> motorObjects;
            /** @brief Non-owning context forwarded to the wait callback. */
            contextpointer waitContext;
            /** @brief Non-null bounded wait operation. */
            motortestwaitcallback waitCallback;

            /** @brief Applies one signed speed to both motors. */
            void setBoth(float64 speedPercent);
            /** @brief Makes an independent zero-speed or fallback stop attempt. */
            void stopBothSafely() noexcept;

        public:
            /**
             * @brief Binds two caller-owned motors and one wait operation.
             *
             * @throws std::invalid_argument
             * If `wait` is null.
             */
            XWalkMotorSequence(XWalkMotor& firstMotor,
                               XWalkMotor& secondMotor,
                               contextpointer context,
                               motortestwaitcallback wait);

            XWalkMotorSequence(const XWalkMotorSequence&) = delete;
            XWalkMotorSequence(XWalkMotorSequence&&) = delete;
            XWalkMotorSequence& operator=(const XWalkMotorSequence&) = delete;
            XWalkMotorSequence& operator=(XWalkMotorSequence&&) = delete;

            /**
             * @brief Runs the requested number of reverse/forward/stop cycles.
             *
             * @param[in] cycleCount
             * Inclusive cycle count from one through 100.
             *
             * @throws std::out_of_range
             * If `cycleCount` is outside its supported range.
             */
            void run(uint32 cycleCount);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_MOTOR_SEQUENCE_H */
