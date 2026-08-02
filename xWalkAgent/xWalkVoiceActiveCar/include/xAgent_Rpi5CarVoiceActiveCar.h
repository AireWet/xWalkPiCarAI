/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCar.h
 * @brief       Declares the sensor-aware voice-active PiCar-X coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_H
#define XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_H

#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarVoiceActiveCarTypes.h"
#include "xHal_Rpi5CarLed.h"
#include "xHal_Rpi5CarVoiceAssistant.h"

namespace xwalk::agent
{

class XWalkVoiceActiveCar
{
    private:
        XWalkPicarx* picarxObject{nullptr};
        XWalkSelfDrive* selfDriveObject{nullptr};
        hal::XWalkVoiceAssistant* assistantObject{nullptr};
        hal::XWalkLed* ledObject{nullptr};
        hal::contextpointer callbackContext{nullptr};
        XWalkVoiceActiveCarCallbacks callbacks{};
        XWalkVoiceActiveCarConfiguration configuration{};

        void blink(hal::uint32 count, hal::uint32 toggleDelayMs,
            hal::uint32 pauseMs);
        void dispatchActions(const hal::stringvector& actions);
        hal::string sensorPrompt();

    protected:
        static void validate(const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoiceActiveCarConfiguration& carConfiguration);

    public:
        XWalkVoiceActiveCar(XWalkPicarx& picarx, XWalkSelfDrive& selfDrive,
            hal::XWalkVoiceAssistant& assistant, hal::XWalkLed& led,
            hal::contextpointer context,
            const XWalkVoiceActiveCarCallbacks& backendCallbacks,
            const XWalkVoiceActiveCarConfiguration& carConfiguration = {});
        ~XWalkVoiceActiveCar() = default;

        XWalkVoiceActiveCar(XWalkVoiceActiveCar&&) = delete;
        XWalkVoiceActiveCar(const XWalkVoiceActiveCar&) = delete;
        XWalkVoiceActiveCar& operator=(XWalkVoiceActiveCar&&) = delete;
        XWalkVoiceActiveCar& operator=(const XWalkVoiceActiveCar&) = delete;

        /** @brief Runs sensor-aware voice rounds until cancellation. */
        hal::int32 run();
        /** @brief Stops voice, action, vehicle, and LED activity. */
        void stop();
        /** @brief Parses `RESPONSE_TEXT\nACTIONS: ...` model output. */
        static XWalkVoiceActiveCarResponse parseResponse(hal::stringview response);
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VOICE_ACTIVE_CAR_H */
