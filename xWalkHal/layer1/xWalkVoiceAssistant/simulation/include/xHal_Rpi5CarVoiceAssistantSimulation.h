#ifndef XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_H
#define XHAL_RPI5CAR_VOICE_ASSISTANT_SIMULATION_H
#include "xHal_Rpi5CarCommon.h"
namespace xwalk::hal::sim
{
/** @brief Exercises one complete assistant round without devices or network access. */
int32 runVoiceAssistantSimulation();
} /* namespace xwalk::hal::sim */
#endif
