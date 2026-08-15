#include "xHal_Rpi5CarVoiceAssistantHostStub.h"
namespace xwalk::hal::sim
{
    void XWalkVoiceAssistantHostStub::configureGpio(
        contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue)
    {
        static_cast<XWalkVoiceAssistantHostStub*>(context)->gpioValue = initialValue;
        static_cast<void>(pin);
        static_cast<void>(mode);
        static_cast<void>(pull);
    }
    boolean XWalkVoiceAssistantHostStub::readGpio(contextpointer context, uint8 pin)
    {
        static_cast<void>(pin);
        return static_cast<XWalkVoiceAssistantHostStub*>(context)->gpioValue;
    }
    void XWalkVoiceAssistantHostStub::writeGpio(contextpointer context, uint8 pin, boolean value)
    {
        static_cast<XWalkVoiceAssistantHostStub*>(context)->gpioValue = value;
        static_cast<void>(pin);
    }
    void XWalkVoiceAssistantHostStub::registerInterrupt(contextpointer context,
                                                        uint8 pin,
                                                        XWalkGpioEdge edge,
                                                        uint32 debounceMs,
                                                        contextpointer handlerContext,
                                                        gpiointerrupthandler handler)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
        static_cast<void>(edge);
        static_cast<void>(debounceMs);
        static_cast<void>(handlerContext);
        static_cast<void>(handler);
    }
    void XWalkVoiceAssistantHostStub::cancelInterrupt(contextpointer context, uint8 pin)
    {
        static_cast<void>(context);
        static_cast<void>(pin);
    }
    XWalkGpioCallbacks XWalkVoiceAssistantHostStub::gpioCallbacks()
    {
        return {&configureGpio, &readGpio, &writeGpio, &registerInterrupt, &cancelInterrupt};
    }
    boolean XWalkVoiceAssistantHostStub::probeI2c(contextpointer context, uint8 address)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return true;
    }
    void XWalkVoiceAssistantHostStub::writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        static_cast<void>(reg);
        static_cast<void>(data);
    }
    bytevector XWalkVoiceAssistantHostStub::readI2c(contextpointer context, uint8 address, size length)
    {
        static_cast<void>(context);
        static_cast<void>(address);
        return bytevector(length, 0U);
    }
    void XWalkVoiceAssistantHostStub::primeSpeaker(contextpointer context, uint32 durationMs)
    {
        static_cast<void>(context);
        static_cast<void>(durationMs);
    }
    boolean XWalkVoiceAssistantHostStub::ready(contextpointer context)
    {
        static_cast<void>(context);
        return true;
    }
    string XWalkVoiceAssistantHostStub::listen(contextpointer context, uint32 timeoutMs)
    {
        ++static_cast<XWalkVoiceAssistantHostStub*>(context)->listenCountValue;
        static_cast<void>(timeoutMs);
        return "simulated question";
    }
    string XWalkVoiceAssistantHostStub::transcribe(contextpointer context, stringview path)
    {
        static_cast<void>(context);
        static_cast<void>(path);
        return "simulated transcription";
    }
    void XWalkVoiceAssistantHostStub::stop(contextpointer context)
    {
        ++static_cast<XWalkVoiceAssistantHostStub*>(context)->stopCountValue;
    }
    XWalkSpeechToTextCallbacks XWalkVoiceAssistantHostStub::recognitionCallbacks()
    {
        return {&ready, &listen, &transcribe, &stop};
    }
    void XWalkVoiceAssistantHostStub::setInstructions(contextpointer context, stringview instructions)
    {
        static_cast<XWalkVoiceAssistantHostStub*>(context)->instructionsValue = string(instructions);
    }
    void XWalkVoiceAssistantHostStub::setWelcome(contextpointer context, stringview welcome)
    {
        static_cast<void>(context);
        static_cast<void>(welcome);
    }
    void XWalkVoiceAssistantHostStub::setMaximumMessages(contextpointer context, uint32 maximumMessages)
    {
        static_cast<void>(context);
        static_cast<void>(maximumMessages);
    }
    void XWalkVoiceAssistantHostStub::addMessage(contextpointer context,
                                                 XWalkLanguageModelRole role,
                                                 stringview content,
                                                 stringview imagePath)
    {
        static_cast<void>(context);
        static_cast<void>(role);
        static_cast<void>(content);
        static_cast<void>(imagePath);
    }
    string XWalkVoiceAssistantHostStub::prompt(contextpointer context, stringview text, stringview imagePath)
    {
        XWalkVoiceAssistantHostStub& backend = *static_cast<XWalkVoiceAssistantHostStub*>(context);
        ++backend.promptCountValue;
        backend.promptValue = string(text);
        static_cast<void>(imagePath);
        return "simulated answer";
    }
    XWalkLanguageModelCallbacks XWalkVoiceAssistantHostStub::modelCallbacks()
    {
        return {&setInstructions, &setWelcome, &setMaximumMessages, &addMessage, &prompt};
    }
    void XWalkVoiceAssistantHostStub::speak(contextpointer context, stringview text)
    {
        XWalkVoiceAssistantHostStub& backend = *static_cast<XWalkVoiceAssistantHostStub*>(context);
        ++backend.spokenCountValue;
        backend.spokenValue = string(text);
    }
    boolean XWalkVoiceAssistantHostStub::valid() const noexcept
    {
        return (instructionsValue == "Answer briefly") && (promptValue == "simulated question") &&
               (spokenValue == "simulated answer") && (listenCountValue == 1U) && (promptCountValue == 1U) &&
               (spokenCountValue == 1U) && (stopCountValue == 1U);
    }
} /* namespace xwalk::hal::sim */
