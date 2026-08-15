/******************************************************************************
 * @file        xAgent_Rpi5CarVoiceControlledCar.cpp
 * @brief       Implements the example-16 recognition and movement loop.
 * @project     xWalk Firmware
 * @module      xWalkVoiceControlledCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#include "xAgent_Rpi5CarVoiceControlledCar.h"

#include "xAgent_Rpi5CarPicarxSafetyGuard.h"

#include "xHal_Rpi5CarTrace.h"
namespace xwalk::agent
{

    agent::int32 XWalkVoiceControlledCar::run()
    {
        XWalkPicarxSafetyGuard safetyGuard(*picarxObject);
        callbacks.output(callbackContext,
                         "Say \"hey robot\" to wake me up! Then say: forward / backward / "
                         "left / right. Say \"sleep\" to stop listening.");
        const agent::boolean processingLoopRequested{true};
        while (processingLoopRequested)
        {
            const agent::boolean operationMayContinue =
                static_cast<agent::boolean>(callbacks.shouldContinue(callbackContext));
            if (operationMayContinue == false)
            {
                break;
            }
            const agent::string wakeTranscript = speechToTextObject->listen(configuration.listenTimeoutMs);
            const agent::boolean wakeWordMissing =
                static_cast<agent::boolean>(!containsWakeWord(wakeTranscript, configuration.wakeWord));
            if (wakeWordMissing)
            {
                continue;
            }
            callbacks.output(callbackContext, "Wake word detected! Listening...");
            const agent::boolean listeningRequested{true};
            while (listeningRequested)
            {
                const agent::boolean listeningMayContinue =
                    static_cast<agent::boolean>(callbacks.shouldContinue(callbackContext));
                if (listeningMayContinue == false)
                {
                    break;
                }
                const agent::string transcript = normalize(speechToTextObject->listen(configuration.listenTimeoutMs));
                const agent::boolean transcriptEmpty = static_cast<agent::boolean>(transcript.empty());
                if (transcriptEmpty)
                {
                    continue;
                }
                callbacks.output(callbackContext, agent::string("Heard: ") + transcript);
                const XWalkVoiceControlledCarCommand command = classifyCommand(transcript, configuration.sleepWord);
                if (command == XWalkVoiceControlledCarCommand::Unknown)
                {
                    continue;
                }
                execute(command);
                if (command == XWalkVoiceControlledCarCommand::Sleep)
                {
                    callbacks.output(callbackContext, "Sleeping. Say the wake word to activate me again.");
                    break;
                }
            }
        }
        stop();
        callbacks.output(callbackContext, "Stopped and centered. Bye.");
        return 0;
    }

    void XWalkVoiceControlledCar::stop()
    {
        speechToTextObject->stop();
        picarxObject->stop();
        picarxObject->setDirectionServoAngle(0.0);
    }

    XWalkVoiceControlledCarCommand XWalkVoiceControlledCar::classifyCommand(agent::stringview transcript,
                                                                            agent::stringview sleepWord)
    {
        const agent::boolean sleepWordEmpty = static_cast<agent::boolean>(sleepWord.empty());
        if (sleepWordEmpty)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-controlled-car sleep word must not be empty");
        }
        const agent::string normalized = normalize(transcript);
        const agent::boolean normalizedNormalizeSleepWordDifferent =
            static_cast<agent::boolean>(normalized.find(normalize(sleepWord)) != agent::string::npos);
        if (normalizedNormalizeSleepWordDifferent)
        {
            return XWalkVoiceControlledCarCommand::Sleep;
        }
        const agent::boolean normalizedForwardDifferent =
            static_cast<agent::boolean>(normalized.find("forward") != agent::string::npos);
        if (normalizedForwardDifferent)
        {
            return XWalkVoiceControlledCarCommand::Forward;
        }
        const agent::boolean normalizedBackwardDifferent =
            static_cast<agent::boolean>(normalized.find("backward") != agent::string::npos);
        if (normalizedBackwardDifferent)
        {
            return XWalkVoiceControlledCarCommand::Backward;
        }
        const agent::boolean normalizedLeftDifferent =
            static_cast<agent::boolean>(normalized.find("left") != agent::string::npos);
        if (normalizedLeftDifferent)
        {
            return XWalkVoiceControlledCarCommand::Left;
        }
        const agent::boolean normalizedRightDifferent =
            static_cast<agent::boolean>(normalized.find("right") != agent::string::npos);
        if (normalizedRightDifferent)
        {
            return XWalkVoiceControlledCarCommand::Right;
        }
        return XWalkVoiceControlledCarCommand::Unknown;
    }

    agent::boolean XWalkVoiceControlledCar::containsWakeWord(agent::stringview transcript, agent::stringview wakeWord)
    {
        const agent::boolean wakeWordEmpty = static_cast<agent::boolean>(wakeWord.empty());
        if (wakeWordEmpty)
        {
            XWALK_RPIAGENT_ERROR(XWALK_INVAL, "Voice-controlled-car wake word must not be empty");
        }
        return normalize(transcript).find(normalize(wakeWord)) != agent::string::npos;
    }

    void XWalkVoiceControlledCar::execute(XWalkVoiceControlledCarCommand command)
    {
        if (command == XWalkVoiceControlledCarCommand::Sleep)
        {
            picarxObject->stop();
            picarxObject->setDirectionServoAngle(0.0);
            return;
        }
        if (command == XWalkVoiceControlledCarCommand::Forward)
        {
            picarxObject->setDirectionServoAngle(0.0);
            picarxObject->forward(configuration.speedPercent);
        }
        else if (command == XWalkVoiceControlledCarCommand::Backward)
        {
            picarxObject->setDirectionServoAngle(0.0);
            picarxObject->backward(configuration.speedPercent);
        }
        else if (command == XWalkVoiceControlledCarCommand::Left)
        {
            picarxObject->setDirectionServoAngle(-configuration.steeringAngle);
            picarxObject->forward(configuration.speedPercent);
        }
        else if (command == XWalkVoiceControlledCarCommand::Right)
        {
            picarxObject->setDirectionServoAngle(configuration.steeringAngle);
            picarxObject->forward(configuration.speedPercent);
        }
        else
        {
            return;
        }
        waitForMovement();
        picarxObject->stop();
        if ((command == XWalkVoiceControlledCarCommand::Left) || (command == XWalkVoiceControlledCarCommand::Right))
        {
            picarxObject->setDirectionServoAngle(0.0);
        }
    }

    void XWalkVoiceControlledCar::waitForMovement()
    {
        constexpr agent::uint32 maximumSliceMs{20U};
        agent::uint32 remainingMs = configuration.driveDurationMs;
        const agent::boolean movementWaitRequested{true};
        while (movementWaitRequested)
        {
            const agent::boolean movementMayContinue =
                static_cast<agent::boolean>((remainingMs > 0U) && callbacks.shouldContinue(callbackContext));
            if (movementMayContinue == false)
            {
                break;
            }
            const agent::uint32 sliceMs = (remainingMs < maximumSliceMs) ? remainingMs : maximumSliceMs;
            callbacks.delay(callbackContext, sliceMs);
            remainingMs -= sliceMs;
        }
    }

} /* namespace xwalk::agent */
