/******************************************************************************
 * @file        xWalk_Rpi5CarAgentConfigType.h
 * @brief       Declares shared Agent composition configuration types.
 *
 * @details
 * Defines the application callback and non-owning configuration bundle shared
 * by Agent composition stages without depending on an Agent module header.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibrary common
 *
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XWALK_RPI5CAR_AGENT_CONFIG_TYPE_H
#define XWALK_RPI5CAR_AGENT_CONFIG_TYPE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Forward declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{
    class XWalkBoardControl;
    class XWalkConfigStore;
    class XWalkGrayscaleModule;
    class XWalkI2c;
    class XWalkMotors;
    class XWalkServo;
    class XWalkUltrasonic;
    struct XWalkGpioCallbacks;
} /* namespace xwalk::hal */

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{
    class XWalkPicarx;
    struct XWalkBootServices;

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Executes one application operation while boot services remain alive.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in,out] services Non-owning services valid only for this callback.
     * @return Application-defined process status.
     */
    using bootapplicationcallback = agent::int32 (*)(agent::contextpointer context, XWalkBootServices& services);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct xAgentContext
     * @brief Groups dependencies shared by Agent vehicle-mode composition.
     *
     * @details
     * Every pointer is non-owning. Each composition stage requires its relevant
     * pointers to remain non-null and valid until that synchronous stage returns.
     * GPIO text views must remain valid for the same interval. The structure
     * releases no resources.
     */
    struct xAgentContext
    {
            /** @brief Nullable caller-owned application state forwarded unchanged. */
            agent::contextpointer appContext{nullptr};
            /** @brief Non-null synchronous application callback. */
            bootapplicationcallback callback{nullptr};
            /** @brief Non-owning configuration pointer valid for the complete dispatch. */
            hal::XWalkConfigStore* config{nullptr};
            /** @brief Non-owning board-controller pointer valid for the complete dispatch. */
            hal::XWalkBoardControl* board{nullptr};
            /** @brief Non-owning PiCar-X pointer valid for the complete dispatch. */
            XWalkPicarx* picarx{nullptr};
            /** @brief Configured GPIO character-device path retained by the caller. */
            agent::stringview gpioDevice{};
            /** @brief Optional exact GPIO chip name retained by the caller. */
            agent::stringview chipName{};
            /** @brief Optional exact GPIO chip label retained by the caller. */
            agent::stringview chipLabel{};
            /** @brief Required minimum GPIO line count. */
            agent::uint32 minLines{};
            /** @brief Non-owning Linux GPIO callback-table pointer valid for the complete dispatch. */
            const hal::XWalkGpioCallbacks* gpioOps{nullptr};
            /** @brief Optional non-owning I2C pointer used by servo-zeroing composition. */
            hal::XWalkI2c* i2c{nullptr};
            /** @brief Non-owning paired-motor pointer used by PiCar-X composition. */
            hal::XWalkMotors* motors{nullptr};
            /** @brief Non-owning steering-servo pointer used by PiCar-X composition. */
            hal::XWalkServo* dirServo{nullptr};
            /** @brief Non-owning camera-pan-servo pointer used by PiCar-X composition. */
            hal::XWalkServo* panServo{nullptr};
            /** @brief Non-owning camera-tilt-servo pointer used by PiCar-X composition. */
            hal::XWalkServo* tiltServo{nullptr};
            /** @brief Non-owning grayscale-module pointer used by PiCar-X composition. */
            hal::XWalkGrayscaleModule* grayscale{nullptr};
            /** @brief Non-owning ultrasonic-sensor pointer used by PiCar-X composition. */
            hal::XWalkUltrasonic* ultrasonic{nullptr};
    };

} /* namespace xwalk::agent */

#endif /* XWALK_RPI5CAR_AGENT_CONFIG_TYPE_H */
