/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceActiveCar.cpp
 * @brief       Implements the sensor-aware voice-active PiCar-X loop.
 *
 * @project     xWalk Firmware
 * @module      xWalkVoiceActiveCar
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceActiveCar.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"

namespace
{

XWalkHal::string trim(XWalkHal::stringview text)
{
    const XWalkHal::size first = text.find_first_not_of(" \t\r\n");
    if (first == XWalkHal::string::npos)
    {
        return {};
    }
    const XWalkHal::size last = text.find_last_not_of(" \t\r\n");
    return XWalkHal::string(text.substr(first, (last - first) + 1U));
}

} /* namespace */

namespace xwalk::agent
{

XWalkVoiceActiveCar::XWalkVoiceActiveCar(XWalkPicarx& picarx,
    XWalkSelfDrive& selfDrive, hal::XWalkVoiceAssistant& assistant,
    hal::XWalkLed& led, hal::contextpointer context,
    const XWalkVoiceActiveCarCallbacks& backendCallbacks,
    const XWalkVoiceActiveCarConfiguration& carConfiguration):
    picarxObject(&picarx), selfDriveObject(&selfDrive),
    assistantObject(&assistant), ledObject(&led), callbackContext(context),
    callbacks(backendCallbacks), configuration(carConfiguration)
{
    validate(callbacks, configuration);
}

hal::int32 XWalkVoiceActiveCar::run()
{
    selfDriveObject->start();
    ledObject->off();
    assistantObject->start();
    while (callbacks.shouldContinue(callbackContext))
    {
        hal::string prompt = sensorPrompt();
        hal::string imagePath;
        if (prompt.empty())
        {
            blink(2U, 100U, 800U);
            prompt = assistantObject->listen(configuration.listenTimeoutMs);
            if (prompt.empty())
            {
                continue;
            }
            if (configuration.withImage && (callbacks.captureImage != nullptr))
            {
                imagePath = callbacks.captureImage(callbackContext);
            }
        }
        blink(1U, 100U, 0U);
        const XWalkVoiceActiveCarResponse response =
            parseResponse(assistantObject->think(prompt, imagePath));
        dispatchActions(response.actions);
        ledObject->on();
        assistantObject->say(response.text);
        if (!selfDriveObject->waitActionsDone())
        {
            stop();
            return 1;
        }
        ledObject->off();
    }
    stop();
    return 0;
}

void XWalkVoiceActiveCar::stop()
{
    assistantObject->stop();
    selfDriveObject->stop();
    picarxObject->close();
    ledObject->off();
}

XWalkVoiceActiveCarResponse XWalkVoiceActiveCar::parseResponse(
    hal::stringview response)
{
    constexpr hal::stringview delimiter{"ACTIONS: "};
    const hal::size actionStart = response.find(delimiter);
    XWalkVoiceActiveCarResponse result;
    result.text = trim(response.substr(0U, actionStart));
    if (actionStart != hal::string::npos)
    {
        hal::string actions = trim(response.substr(actionStart + delimiter.size()));
        while (!actions.empty())
        {
            const hal::size separator = actions.find(", ");
            result.actions.push_back(trim(actions.substr(0U, separator)));
            if (separator == hal::string::npos)
            {
                actions.clear();
            }
            else
            {
                actions.erase(0U, separator + 2U);
            }
        }
    }
    if (result.actions.empty())
    {
        result.actions.emplace_back("stop");
    }
    return result;
}

void XWalkVoiceActiveCar::blink(hal::uint32 count,
    hal::uint32 toggleDelayMs, hal::uint32 pauseMs)
{
    for (hal::uint32 index = 0U; index < count; ++index)
    {
        ledObject->on();
        callbacks.delay(callbackContext, toggleDelayMs);
        ledObject->off();
        callbacks.delay(callbackContext, toggleDelayMs);
    }
    if (pauseMs > 0U)
    {
        callbacks.delay(callbackContext, pauseMs);
    }
}

void XWalkVoiceActiveCar::dispatchActions(const hal::stringvector& actions)
{
    for (const hal::string& action : actions)
    {
        if (action == "stop")
        {
            picarxObject->stop();
        }
        else if (!selfDriveObject->addAction(action))
        {
            callbacks.output(callbackContext,
                hal::string("Unsupported voice action: ") + action);
        }
    }
}

hal::string XWalkVoiceActiveCar::sensorPrompt()
{
    const hal::float64 distanceCm = picarxObject->distance();
    if ((distanceCm > 1.0) && (distanceCm < configuration.tooCloseCm))
    {
        static_cast<void>(selfDriveObject->addAction("backward"));
        const hal::string distance = hal::common::float64ToString(distanceCm);
        callbacks.output(callbackContext,
            hal::string("Ultrasonic sense too close: ") + distance + "cm");
        return hal::string("<<<Ultrasonic sense too close: ") + distance + "cm>>>";
    }
    return {};
}

void XWalkVoiceActiveCar::validate(
    const XWalkVoiceActiveCarCallbacks& backendCallbacks,
    const XWalkVoiceActiveCarConfiguration& carConfiguration)
{
    if ((backendCallbacks.output == nullptr) ||
        (backendCallbacks.shouldContinue == nullptr) ||
        (backendCallbacks.delay == nullptr))
    {
        XHAL_THROW_INVALID_ARGUMENT("Voice-active-car callbacks must be complete");
    }
    if ((carConfiguration.tooCloseCm <= 1.0) ||
        (carConfiguration.listenTimeoutMs == 0U) ||
        (carConfiguration.listenTimeoutMs >
            XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS))
    {
        XHAL_THROW_OUT_OF_RANGE("Voice-active-car configuration is outside its range");
    }
}

} /* namespace xwalk::agent */
