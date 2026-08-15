/******************************************************************************
 * @file        xHal_Rpi5CarPwmLifecycle.cpp
 * @brief       Implements PWM channel construction and destruction.
 *
 * @details
 * Validates numeric channels, stores pointers to caller-created dependencies,
 * resolves I2C addressing, maps timers, and performs the initial 50 Hertz
 * timer configuration.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarPwm.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs a PWM output from a numeric channel index.
     *
     * @param[in] i2c
     * Non-owning I2C interface used for probing and register writes.
     *
     * @param[in] channel
     * PWM channel in the inclusive range 0 to 19.
     *
     * @param[in] address
     * Optional explicit seven-bit I2C address; omission enables probing.
     *
     * @param[in] sharedTimerState
     * Shared timer state created by the caller and passed by reference.
     *
     * @pre
     * `i2c` and `sharedTimerState` outlive this object.
     *
     * @post
     * The selected channel is mapped to a timer and configured for a requested
     * frequency of 50 Hertz.
     *
     * @throws std::out_of_range
     * If `channel` exceeds 19, `address` exceeds the seven-bit range, or the
     * default frequency is not representable.
     */
    XWalkPwm::XWalkPwm(XWalkI2c& i2c, uint32 channel, optionaluint8 address, XWalkPwmTimerState& sharedTimerState)
        : i2cObject(&i2c), timerState(&sharedTimerState), channelValue(channel)
    {
        if (channelValue > XHAL_RPI5CAR_PWM_MAX_CHANNEL)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "PWM channel must be in range 0..19");
        }

        addressValue = selectAddress(address);
        timerIndexValue = timerForChannel(channelValue);
        setFrequency(XHAL_RPI5CAR_PWM_DEFAULT_FREQUENCY_HZ);
        XWALK_HAL_TRACE_UID3(RPI .159,
                             "PWM channel %u constructed at address 0x%02X with timer %u",
                             channelValue,
                             addressValue,
                             timerIndexValue);
    }

    /**
     * @brief Constructs a PWM output from a textual channel designation.
     *
     * @param[in] i2c
     * Non-owning I2C interface used for probing and register writes.
     *
     * @param[in] channel
     * Channel text in the form `P0` through `P19`.
     *
     * @param[in] address
     * Optional explicit seven-bit I2C address; omission enables probing.
     *
     * @param[in] sharedTimerState
     * Shared timer state created by the caller and passed by reference.
     *
     * @pre
     * `i2c` and `sharedTimerState` outlive this object.
     *
     * @post
     * Successful construction produces the same state as the numeric overload
     * for the parsed channel.
     *
     * @throws std::invalid_argument
     * If the channel text is malformed.
     *
     * @throws std::out_of_range
     * If the parsed channel exceeds 19, `address` exceeds the seven-bit range, or
     * the default frequency is not representable.
     */
    XWalkPwm::XWalkPwm(XWalkI2c& i2c, stringview channel, optionaluint8 address, XWalkPwmTimerState& sharedTimerState)
        : XWalkPwm(i2c, parseChannel(channel), address, sharedTimerState)
    {
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the PWM output object.
     *
     * @note
     * The referenced I2C interface and timer-state pointer are non-owning and
     * are not released.
     */
    XWalkPwm::~XWalkPwm() = default;

} /* namespace xwalk::hal */
