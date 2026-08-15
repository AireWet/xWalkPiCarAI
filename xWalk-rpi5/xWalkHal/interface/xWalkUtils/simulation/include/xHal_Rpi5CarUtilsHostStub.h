/******************************************************************************
 * @file        xHal_Rpi5CarUtilsHostStub.h
 * @brief       Declares the side-effect-free xWalkUtils host stub.
 * @details     Mirrors every utility callback in owned memory for host execution.
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Simulation
 * @author      Joxy John
 * @date        2026-08-10
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/
#ifndef XHAL_RPI5CAR_UTILS_HOST_STUB_H
#define XHAL_RPI5CAR_UTILS_HOST_STUB_H
#include "xHal_Rpi5CarUtils.h"
namespace xwalk::hal::sim
{
    /** @brief Mirrors utility platform services without external side effects. */
    class XWalkUtilsHostStub final
    {
        private:
            string messageValue;
            string commandValue;
            uint8 volumePercentValue{};
            uint32 outputCountValue{};

        protected:
            static void output(
                contextpointer context, XWalkUtilityColor color, stringview message, stringview ending, boolean flush);
            static void setVolume(contextpointer context, uint8 volumePercent);
            static XWalkCommandResult
            runCommand(contextpointer context, stringview command, stringview user, stringview group);
            static boolean executableExists(contextpointer context, stringview executable);
            static string ipAddress(contextpointer context, stringview interfaceName);
            static string username(contextpointer context);

        public:
            XWalkUtilsHostStub();
            ~XWalkUtilsHostStub();
            XWalkUtilsHostStub(const XWalkUtilsHostStub&) = delete;
            XWalkUtilsHostStub& operator=(const XWalkUtilsHostStub&) = delete;
            XWalkUtilsHostStub(XWalkUtilsHostStub&&) = delete;
            XWalkUtilsHostStub& operator=(XWalkUtilsHostStub&&) = delete;
            XWalkUtilsCallbacks callbacks() noexcept;
            string message() const;
            string command() const;
            uint8 volumePercent() const noexcept;
            uint32 outputCount() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif /* XHAL_RPI5CAR_UTILS_HOST_STUB_H */
