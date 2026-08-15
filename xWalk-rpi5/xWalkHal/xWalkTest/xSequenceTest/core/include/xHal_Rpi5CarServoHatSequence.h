/******************************************************************************
 * @file        xHal_Rpi5CarServoHatSequence.h
 * @brief       Declares the bounded Robot HAT servo and ADC sequence.
 *
 * @details
 * Coordinates an MCU reset, a 16-channel servo sweep, and bounded sampling of
 * ADC channels zero through four through caller-owned HAL objects.
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

#ifndef XHAL_RPI5CAR_SERVO_HAT_SEQUENCE_H
#define XHAL_RPI5CAR_SERVO_HAT_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarServo.h"

/******************************************************************************
 * Object-like macros
 ******************************************************************************/

/** @brief Maximum bounded ADC sample count accepted by the sequence. */
#define XHAL_RPI5CAR_SERVO_HAT_MAX_SAMPLES 3600U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal::test
 * @brief Contains host-testable and physical HAL sequence behavior.
 */
namespace xwalk::hal::test
{

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Non-owning ordered servo pointers for PWM channels zero through 15. */
    using servohatservoarray = fixedarray<XWalkServo*, 16U>;

    /** @brief Non-owning ordered ADC pointers for channels zero through four. */
    using servohatadcarray = fixedarray<XWalkAdc*, 5U>;

    /** @brief One ordered set of raw ADC samples for channels zero through four. */
    using servohatreadings = fixedarray<uint16, 5U>;

    /**
     * @brief Wait operation injected for host and hardware execution.
     *
     * @param[in,out] context
     * Non-owning callback context whose nullability is implementation-defined.
     *
     * @param[in] durationMilliseconds
     * Requested wait duration in milliseconds.
     */
    using servohatwaitcallback = void (*)(contextpointer context, uint32 durationMilliseconds);

    /**
     * @brief Reports the servo channel about to be moved to ten degrees.
     *
     * @param[in,out] context
     * Non-owning callback context whose nullability is implementation-defined.
     *
     * @param[in] channel
     * Servo PWM channel in the inclusive range zero through 15.
     */
    using servohatservocallback = void (*)(contextpointer context, uint8 channel);

    /**
     * @brief Reports one complete ordered five-channel ADC sample.
     *
     * @param[in,out] context
     * Non-owning callback context whose nullability is implementation-defined.
     *
     * @param[in] readings
     * Raw samples ordered by ADC channels zero through four. The callback must not
     * retain the reference beyond the call.
     */
    using servohatadccallback = void (*)(contextpointer context, const servohatreadings& readings);

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @brief Runs the fixed servo sweep followed by bounded ADC monitoring.
     *
     * @details
     * Stores only non-owning pointers and callback context. The board controller,
     * servos, ADC inputs, and callback implementation must outlive the sequence.
     */
    class XWalkServoHatSequence
    {
        private:
            /** @brief Caller-owned board controller used for the MCU reset pulse. */
            XWalkBoardControl* boardControlObject;
            /** @brief Caller-owned servos ordered by PWM channel zero through 15. */
            servohatservoarray servoObjects;
            /** @brief Caller-owned ADC objects ordered by channel zero through four. */
            servohatadcarray adcObjects;
            /** @brief Non-owning context forwarded to every injected callback. */
            contextpointer callbackContext;
            /** @brief Non-null bounded wait operation. */
            servohatwaitcallback waitCallback;
            /** @brief Non-null servo-channel reporting operation. */
            servohatservocallback servoCallback;
            /** @brief Non-null ADC-sample reporting operation. */
            servohatadccallback adcCallback;

        public:
            /**
             * @brief Binds the board, servos, ADC inputs, and reporting operations.
             *
             * @param[in] boardControl
             * Caller-owned board controller that must outlive the sequence.
             *
             * @param[in] servos
             * Non-null servo pointers ordered by PWM channel zero through 15. Every
             * servo must outlive the sequence.
             *
             * @param[in] adcInputs
             * Non-null ADC pointers ordered by channel zero through four. Every ADC
             * object must outlive the sequence.
             *
             * @param[in,out] context
             * Non-owning context forwarded to all callbacks; nullability is
             * callback-defined.
             *
             * @param[in] wait
             * Non-null wait callback receiving durations in milliseconds.
             *
             * @param[in] reportServo
             * Non-null callback receiving each servo channel before movement.
             *
             * @param[in] reportAdc
             * Non-null callback receiving every ordered ADC sample.
             *
             * @throws std::invalid_argument
             * If a callback or dependency pointer is null.
             */
            XWalkServoHatSequence(XWalkBoardControl& boardControl,
                                  const servohatservoarray& servos,
                                  const servohatadcarray& adcInputs,
                                  contextpointer context,
                                  servohatwaitcallback wait,
                                  servohatservocallback reportServo,
                                  servohatadccallback reportAdc);

            /** @brief Prevents copying of non-owning dependency bindings. */
            XWalkServoHatSequence(const XWalkServoHatSequence&) = delete;
            /** @brief Prevents moving of non-owning dependency bindings. */
            XWalkServoHatSequence(XWalkServoHatSequence&&) = delete;
            /** @brief Prevents copy assignment of non-owning dependency bindings. */
            XWalkServoHatSequence& operator=(const XWalkServoHatSequence&) = delete;
            /** @brief Prevents move assignment of non-owning dependency bindings. */
            XWalkServoHatSequence& operator=(XWalkServoHatSequence&&) = delete;

            /**
             * @brief Resets the MCU, sweeps all servos, and samples all ADC inputs.
             *
             * @param[in] sampleCount
             * Number of five-channel ADC samples, in the inclusive range 1 through
             * 3600. Each sample is followed by a one-second wait.
             *
             * @post
             * On normal completion, every servo's last requested angle is zero degrees.
             *
             * @throws std::out_of_range
             * If `sampleCount` is outside its supported range.
             */
            void run(uint32 sampleCount);
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_SERVO_HAT_SEQUENCE_H */
