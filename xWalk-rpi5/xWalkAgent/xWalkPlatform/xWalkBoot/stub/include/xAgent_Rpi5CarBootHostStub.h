/******************************************************************************
 * @file        xAgent_Rpi5CarBootHostStub.h
 * @brief       Declares the device-free xWalkBoot host stub.
 *
 * @details
 * Forwards caller-supplied simulated services through the same one-shot
 * application boundary used by the Raspberry Pi composition.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot Host Stub
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

#ifndef XAGENT_RPI5CAR_BOOT_HOST_STUB_H
#define XAGENT_RPI5CAR_BOOT_HOST_STUB_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarBoot.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::agent
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /** @class XWalkBootHostStub @brief Provides one device-free boot execution. */
    class XWalkBootHostStub final : private XWalkBoot
    {
        private:
            /** @brief Non-owning simulated services retained for one host run. */
            XWalkBootServices services{};

        public:
            /**
             * @brief Constructs one host stub around caller-owned simulated services.
             * @param[in] simulatedServices Services that outlive this object.
             */
            explicit XWalkBootHostStub(const XWalkBootServices& simulatedServices) noexcept;

            /** @brief Releases the host stub without touching physical hardware. */
            ~XWalkBootHostStub() = default;

            XWalkBootHostStub(XWalkBootHostStub&&) = delete;
            XWalkBootHostStub(const XWalkBootHostStub&) = delete;
            XWalkBootHostStub& operator=(XWalkBootHostStub&&) = delete;
            XWalkBootHostStub& operator=(const XWalkBootHostStub&) = delete;

            /**
             * @brief Executes one application callback with simulated services.
             * @param[in] parameters Application context and non-null callback
             * retained through the synchronous host dispatch.
             * @return Status returned by the configured callback.
             * @throws std::invalid_argument If the configured callback is null.
             * @throws std::logic_error If this object already started once.
             */
            agent::int32 run(const xAgentContext& parameters);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BOOT_HOST_STUB_H */
