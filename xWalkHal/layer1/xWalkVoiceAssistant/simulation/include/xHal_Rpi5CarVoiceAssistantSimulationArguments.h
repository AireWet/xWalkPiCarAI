#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_ARGUMENTS_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_ARGUMENTS_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Validates and applies one optional persistent trace selector. */
class XWalkVoiceAssistantSimulationArguments final
{
    private:
        string targetValue; boolean enabledValue; boolean updateValue;
        boolean validValue; boolean helpValue;
    protected:
        static boolean targetIsValid(stringview target) noexcept;
        void parseSelector(stringview selector);
    public:
        XWalkVoiceAssistantSimulationArguments(int32 count, charpointer values[]);
        ~XWalkVoiceAssistantSimulationArguments();
        XWalkVoiceAssistantSimulationArguments(
            const XWalkVoiceAssistantSimulationArguments&) = delete;
        XWalkVoiceAssistantSimulationArguments& operator=(
            const XWalkVoiceAssistantSimulationArguments&) = delete;
        XWalkVoiceAssistantSimulationArguments(
            XWalkVoiceAssistantSimulationArguments&&) = delete;
        XWalkVoiceAssistantSimulationArguments& operator=(
            XWalkVoiceAssistantSimulationArguments&&) = delete;
        boolean valid() const noexcept;
        boolean helpRequested() const noexcept;
        boolean applyTraceUpdate() const;
};
} /* namespace xwalk::hal::sim */
#endif
