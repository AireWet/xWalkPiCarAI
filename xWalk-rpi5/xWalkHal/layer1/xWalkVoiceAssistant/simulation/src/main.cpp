#include "xHal_Rpi5CarTrace.h"
#include "xHal_Rpi5CarVoiceAssistantSimulation.h"
#include "xHal_Rpi5CarVoiceAssistantSimulationArguments.h"
#include "xHal_Rpi5CarVoiceAssistantSimulationConfig.h"
XWalkHal::int32 main(XWalkHal::int32 count, XWalkHal::charpointer values[])
{
    xwalk::hal::XWalkTrace::configureGlobal(XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_CONFIG_PATH,
                                            XWALK_VOICE_ASSISTANT_SIMULATION_TRACE_LOG_PATH);
    const xwalk::hal::sim::XWalkVoiceAssistantSimulationArguments arguments(count, values);
    if (!arguments.valid())
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "Invalid xWalkVoiceAssistant simulation arguments");
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", values[0]);
        return 2;
    }
    if (arguments.helpRequested())
    {
        XWALK_HAL_WARNING(XWALK_INVAL, "Usage: %s [--help | --trace <selector>]", values[0]);
        XWALK_HAL_WARNING(XWALK_LOGIC, "Trace selectors persist in XML and load on the next run");
        return 0;
    }
    if (!arguments.applyTraceUpdate())
    {
        XWALK_HAL_ERROR(XWALK_EXCEPTION, "The requested trace identifier is not present in the trace inventory");
        return 2;
    }
    XWALK_HAL_TRACE_UID0(RPI .380, "xWalkVoiceAssistant simulation started");
    return xwalk::hal::sim::runVoiceAssistantSimulation();
}
