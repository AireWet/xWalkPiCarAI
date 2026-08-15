/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxSafetyGuard.h
 * @brief       Declares scope-bound PiCar-X emergency shutdown.
 *
 * @details
 * Provides a non-owning guard that latches actuator suppression and attempts to
 * stop both motors whenever one application command leaves its execution scope.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_PICARX_SAFETY_GUARD_H
#define XAGENT_RPI5CAR_PICARX_SAFETY_GUARD_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkPicarxSafetyGuard
     * @brief Applies non-throwing PiCar-X emergency shutdown when command scope ends.
     *
     * @details
     * Stores one non-owning PiCar-X pointer. The observed coordinator must outlive
     * this guard. Destruction never releases the coordinator or its HAL dependencies.
     */
    class XWalkPicarxSafetyGuard
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning PiCar-X pointer that is never null and must outlive this guard. */
            XWalkPicarx* picarxObject{nullptr};

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs an armed safety guard around one caller-owned coordinator.
             * @param[in] picarx PiCar-X coordinator that must outlive this guard.
             */
            explicit XWalkPicarxSafetyGuard(XWalkPicarx& picarx) noexcept;

            /**
             * @brief Latches actuator suppression and independently attempts to stop both motors.
             * @post The observed PiCar-X emergency-stop state is latched.
             */
            ~XWalkPicarxSafetyGuard() noexcept;

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve observed coordinator identity. */
            XWalkPicarxSafetyGuard(XWalkPicarxSafetyGuard&&) = delete;
            /** @brief Disables copying of the non-owning safety binding. */
            XWalkPicarxSafetyGuard(const XWalkPicarxSafetyGuard&) = delete;
            /** @brief Disables move assignment to preserve observed coordinator identity. */
            XWalkPicarxSafetyGuard& operator=(XWalkPicarxSafetyGuard&&) = delete;
            /** @brief Disables copying of the non-owning safety binding. */
            XWalkPicarxSafetyGuard& operator=(const XWalkPicarxSafetyGuard&) = delete;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_PICARX_SAFETY_GUARD_H */
