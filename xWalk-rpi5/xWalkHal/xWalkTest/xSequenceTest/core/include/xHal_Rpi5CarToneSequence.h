/******************************************************************************
 * @file        xHal_Rpi5CarToneSequence.h
 * @brief       Declares the Robot HAT tone melody sequence.
 *
 * @details
 * Preserves the enabled melody from robot-hat/tests/tone_test.py while using
 * the xWalkMusic abstraction and an injected measure-reporting callback.
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

#ifndef XHAL_RPI5CAR_TONE_SEQUENCE_H
#define XHAL_RPI5CAR_TONE_SEQUENCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarMusic.h"

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
     * Structure declarations
     ******************************************************************************/

    /** @brief Describes one ordered note from the ported tone melody. */
    struct XWalkToneEvent
    {
            /** @brief One-based source measure number. */
            uint8 measureNumber{};
            /** @brief Note name accepted by `XWalkMusic::noteFrequencyHz()`. */
            stringview noteName{};
            /** @brief Whole-note fraction accepted by `XWalkMusic::beatDurationSeconds()`. */
            float64 beatValue{};
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /** @brief Complete fixed 72-note melody from the enabled Python sequence. */
    using tonesequenceeventarray = fixedarray<XWalkToneEvent, 72U>;

    /**
     * @brief Reports entry into one source measure.
     *
     * @param[in,out] context
     * Non-owning callback context whose nullability is implementation-defined.
     *
     * @param[in] measureNumber
     * One-based measure number in the inclusive range one through 17.
     */
    using tonesequencemeasurecallback = void (*)(contextpointer context, uint8 measureNumber);

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @brief Plays the ported 17-measure melody through caller-owned music. */
    class XWalkToneSequence
    {
        private:
            /** @brief Caller-owned music controller that must outlive the sequence. */
            XWalkMusic* musicObject;
            /** @brief Non-owning context forwarded to the measure callback. */
            contextpointer measureContext;
            /** @brief Non-null callback invoked once before every measure. */
            tonesequencemeasurecallback measureCallback;

        public:
            /**
             * @brief Binds the music controller and measure-reporting operation.
             *
             * @param[in,out] music
             * Caller-owned music controller that must outlive this sequence.
             *
             * @param[in,out] context
             * Non-owning context forwarded to `reportMeasure`.
             *
             * @param[in] reportMeasure
             * Non-null callback invoked before the first note in every measure.
             *
             * @throws std::invalid_argument
             * If `reportMeasure` is null.
             */
            XWalkToneSequence(XWalkMusic& music, contextpointer context, tonesequencemeasurecallback reportMeasure);

            /** @brief Prevents copying of non-owning dependency bindings. */
            XWalkToneSequence(const XWalkToneSequence&) = delete;
            /** @brief Prevents moving of non-owning dependency bindings. */
            XWalkToneSequence(XWalkToneSequence&&) = delete;
            /** @brief Prevents copy assignment of non-owning dependency bindings. */
            XWalkToneSequence& operator=(const XWalkToneSequence&) = delete;
            /** @brief Prevents move assignment of non-owning dependency bindings. */
            XWalkToneSequence& operator=(XWalkToneSequence&&) = delete;

            /**
             * @brief Returns the immutable melody definition.
             *
             * @return
             * Stable 72-event sequence grouped into 17 measures.
             */
            static const tonesequenceeventarray& melody() noexcept;

            /**
             * @brief Sets the source volume and tempo, then plays the complete melody.
             *
             * @post
             * Music volume is 80 percent, tempo is 60 quarter-note beats per minute,
             * and all 72 tone events have been submitted synchronously.
             */
            void run();
    };

} /* namespace xwalk::hal::test */

#endif /* XHAL_RPI5CAR_TONE_SEQUENCE_H */
