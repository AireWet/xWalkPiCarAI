/******************************************************************************
 * @file        xHal_Rpi5CarPwmTimerState.h
 * @brief       Declares synchronized shared PWM timer-period state.
 *
 * @details
 * Stores one period value for each of the seven Robot HAT PWM timers and
 * serializes access shared by PWM channel objects.
 *
 * @project     xWalk Firmware
 * @module      xWalkPwm
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_PWM_TIMER_STATE_H
#define XHAL_RPI5CAR_PWM_TIMER_STATE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkPwmTimerState
 * @brief Maintains thread-safe period values for seven shared PWM timers.
 *
 * @details
 * Each PWM channel uses one of seven timer indices. This object allows multiple
 * channel objects to observe the same period value while serializing reads and
 * updates with an internal mutex.
 */
class XWalkPwmTimerState
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Timer periods in timer-count units, indexed from zero to six. */
        pwmperiodarray periods{};

        /** @brief Mutex protecting every read and mutation of `periods`. */
        mutable mutexhandle mutex;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /** @brief Constructs shared state with every timer period set to one. */
        XWalkPwmTimerState();

        /** @brief Destroys the timer state after all users have released it. */
        ~XWalkPwmTimerState();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction because the mutex is non-movable. */
        XWalkPwmTimerState(XWalkPwmTimerState&&) = delete;
        /** @brief Disables copying of synchronized shared state. */
        XWalkPwmTimerState(const XWalkPwmTimerState&) = delete;
        /** @brief Disables move assignment because the mutex is non-movable. */
        XWalkPwmTimerState& operator=(XWalkPwmTimerState&&) = delete;
        /** @brief Disables copy assignment of synchronized shared state. */
        XWalkPwmTimerState& operator=(const XWalkPwmTimerState&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Updates one shared timer period under mutual exclusion.
         *
         * @param[in] timerIndex
         * Timer index in the inclusive range 0 to 6.
         *
         * @param[in] period
         * New period in timer-count units. This storage function does not
         * independently validate the period range.
         *
         * @post
         * The selected timer contains `period` when the function returns.
         *
         * @throws std::out_of_range
         * If `timerIndex` exceeds six.
         */
        void updatePeriod(uint32 timerIndex, uint32 period);

        /**
         * @brief Reads one shared timer period under mutual exclusion.
         *
         * @param[in] timerIndex
         * Timer index in the inclusive range 0 to 6.
         *
         * @return
         * Stored period in timer-count units.
         *
         * @throws std::out_of_range
         * If `timerIndex` exceeds six.
         */
        uint32 getPeriod(uint32 timerIndex) const;
};

/******************************************************************************
 * Type definitions
 ******************************************************************************/

/**
 * @brief Non-owning pointer to shared PWM timer state.
 *
 * @note
 * `XWalkPwm` initializes this pointer from a required constructor reference, so
 * it is non-null after construction and must remain valid for the PWM lifetime.
 */
using pwmtimerstatepointer = XWalkPwmTimerState*;

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_PWM_TIMER_STATE_H */
