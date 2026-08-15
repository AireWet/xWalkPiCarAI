/******************************************************************************
 * @file        xAgent_Rpi5CarBoot.h
 * @brief       Declares the shared one-shot xWalkBoot lifecycle guard.
 *
 * @details
 * Provides callback validation and per-object single-run state reused by host
 * and Raspberry Pi boot implementations without depending on the CLI layer.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot
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

#ifndef XAGENT_RPI5CAR_BOOT_H
#define XAGENT_RPI5CAR_BOOT_H

/******************************************************************************
 * Includes
 ******************************************************************************/
#include "xAgent_Rpi5CarBootTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkBoot
     * @brief Guards one platform boot implementation against repeated execution.
     */
    class XWalkBoot
    {
        private:
            /** @brief Records whether this object consumed its one run attempt. */
            agent::boolean started{};

        protected:
            /** @brief Constructs an unstarted boot lifecycle. */
            XWalkBoot() noexcept = default;

            /** @brief Releases lifecycle state without touching platform resources. */
            ~XWalkBoot() = default;

            XWalkBoot(XWalkBoot&&) = delete;
            XWalkBoot(const XWalkBoot&) = delete;
            XWalkBoot& operator=(XWalkBoot&&) = delete;
            XWalkBoot& operator=(const XWalkBoot&) = delete;

            /**
             * @brief Validates and consumes this object's one boot attempt.
             * @param[in] callback Non-null synchronous application callback.
             * @throws std::invalid_argument If `callback` is null.
             * @throws std::logic_error If this object already started once.
             */
            void begin(bootapplicationcallback callback);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_H */
