#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_HOST_STUB_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_HOST_STUB_H
#include "xHal_Rpi5CarVoiceAssistant.h"
namespace xwalk::hal::sim
{
    /** @brief Supplies every dependency required by the device-free simulation. */
    class XWalkVoiceAssistantHostStub final
    {
        private:
            string instructionsValue{};
            string promptValue{};
            string spokenValue{};
            uint32 listenCountValue{};
            uint32 promptCountValue{};
            uint32 spokenCountValue{};
            uint32 stopCountValue{};
            boolean gpioValue{};

        public:
            static void configureGpio(
                contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);
            static boolean readGpio(contextpointer context, uint8 pin);
            static void writeGpio(contextpointer context, uint8 pin, boolean value);
            static void registerInterrupt(contextpointer context,
                                          uint8 pin,
                                          XWalkGpioEdge edge,
                                          uint32 debounceMs,
                                          contextpointer handlerContext,
                                          gpiointerrupthandler handler);
            static void cancelInterrupt(contextpointer context, uint8 pin);
            static XWalkGpioCallbacks gpioCallbacks();
            static boolean probeI2c(contextpointer context, uint8 address);
            static void writeI2c(contextpointer context, uint8 address, uint8 reg, const bytevector& data);
            static bytevector readI2c(contextpointer context, uint8 address, size length);
            static void primeSpeaker(contextpointer context, uint32 durationMs);
            static boolean ready(contextpointer context);
            static string listen(contextpointer context, uint32 timeoutMs);
            static string transcribe(contextpointer context, stringview path);
            static void stop(contextpointer context);
            static XWalkSpeechToTextCallbacks recognitionCallbacks();
            static void setInstructions(contextpointer context, stringview instructions);
            static void setWelcome(contextpointer context, stringview welcome);
            static void setMaximumMessages(contextpointer context, uint32 maximumMessages);
            static void
            addMessage(contextpointer context, XWalkLanguageModelRole role, stringview content, stringview imagePath);
            static string prompt(contextpointer context, stringview text, stringview imagePath);
            static XWalkLanguageModelCallbacks modelCallbacks();
            static void speak(contextpointer context, stringview text);
            boolean valid() const noexcept;
    };
} /* namespace xwalk::hal::sim */
#endif
