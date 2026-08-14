/******************************************************************************
 * @file        xHal_Rpi5CarUltrasonicLifecycle.cpp
 * @brief       Implements ultrasonic sensor construction and destruction.
 *
 * @details
 * Binds caller-owned GPIO dependencies and establishes their trigger-output
 * and pull-down echo-input configurations.
 *
 * @project     xWalk Firmware
 * @module      xWalkUltrasonic
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

#include "xHal_Rpi5CarUltrasonic.h"

#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs a distance sensor from caller-owned GPIO objects.
 *
 * @param[in] trigger
 * GPIO object reconfigured as the trigger output; it must outlive this object.
 *
 * @param[in] echo
 * GPIO object reconfigured as a pull-down echo input; it must outlive this
 * object.
 *
 * @param[in] timeoutMicroseconds
 * Maximum echo acquisition interval in microseconds; zero requests an immediate
 * timeout.
 *
 * @post
 * `trigger` is configured as an output and `echo` as a pull-down input.
 */
XWalkUltrasonic::XWalkUltrasonic(XWalkGpio &trigger, XWalkGpio &echo,
                                 uint32 timeoutMicroseconds)
    : triggerPin(&trigger), echoPin(&echo),
      timeoutMicrosecondsValue(timeoutMicroseconds) {
  triggerPin->setup(XWalkGpioMode::Output, XWalkGpioPull::None);
  echoPin->setup(XWalkGpioMode::Input, XWalkGpioPull::Down);
  XWALK_HAL_TRACE_UID1(RPI .202,
                       "Ultrasonic sensor constructed with timeout %u us",
                       timeoutMicrosecondsValue);
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/**
 * @brief Destroys the sensor without releasing its non-owning GPIO
 * dependencies.
 */
XWalkUltrasonic::~XWalkUltrasonic() = default;

} /* namespace xwalk::hal */
