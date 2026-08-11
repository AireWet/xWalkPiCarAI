/******************************************************************************
 * @file        xHal_Rpi5CarSimulationArgumentsTest.cpp
 * @brief       Verifies common host-simulation trace argument boundaries.
 *
 * @details
 * Applies the shared command-line contract to every production simulation
 * argument parser without invoking physical hardware.
 *
 * @project     xWalk Firmware
 * @module      xGoogleTest
 *
 * @author      Joxy John
 * @date        2026-08-11
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarAdcSimulationArguments.h"
#include "xHal_Rpi5CarAdxl345SimulationArguments.h"
#include "xHal_Rpi5CarAudioSimulationArguments.h"
#include "xHal_Rpi5CarBoardControlSimulationArguments.h"
#include "xHal_Rpi5CarBuzzerSimulationArguments.h"
#include "xHal_Rpi5CarCameraSimulationArguments.h"
#include "xHal_Rpi5CarConfigSimulationArguments.h"
#include "xHal_Rpi5CarGpioSimulationArguments.h"
#include "xHal_Rpi5CarGptSimulationArguments.h"
#include "xHal_Rpi5CarI2cSimulationArguments.h"
#include "xHal_Rpi5CarLanguageModelSimulationArguments.h"
#include "xHal_Rpi5CarLedSimulationArguments.h"
#include "xHal_Rpi5CarLineTrackerSimulationArguments.h"
#include "xHal_Rpi5CarMotorSimulationArguments.h"
#include "xHal_Rpi5CarMusicSimulationArguments.h"
#include "xHal_Rpi5CarPwmSimulationArguments.h"
#include "xHal_Rpi5CarRobotSimulationArguments.h"
#include "xHal_Rpi5CarServoSimulationArguments.h"
#include "xHal_Rpi5CarSpeakerSimulationArguments.h"
#include "xHal_Rpi5CarSpiSimulationArguments.h"
#include "xHal_Rpi5CarUltrasonicSimulationArguments.h"
#include "xHal_Rpi5CarUserButtonSimulationArguments.h"
#include "xHal_Rpi5CarUtilsSimulationArguments.h"
#include "xHal_Rpi5CarVoiceAssistantSimulationArguments.h"

#include <gtest/gtest.h>

namespace
{

template <typename ArgumentsType>
void verifyTraceArgumentBoundaries()
{
    using xwalk::hal::charpointer;

    char binary[] = "simulation";
    char help[] = "--help";
    char shortHelp[] = "-h";
    char trace[] = "--trace";
    char unknownOption[] = "--unknown";
    char invalidSuffix[] = "RPI";
    char emptyTarget[] = ".enable";
    char prefixOnly[] = "RPI..enable";
    char invalidPrefix[] = "HAL.1.enable";
    char invalidFirstDigit[] = "RPI.x1.enable";
    char invalidLastDigit[] = "RPI.1x.disable";
    char validUid[] = "RPI.123.enable";
    char disableAll[] = "all.disable";
    char json[] = "missing-trace.json";

    charpointer defaultValues[]{binary};
    charpointer noOptionValue[]{binary, nullptr};
    charpointer helpValues[]{binary, help};
    charpointer shortHelpValues[]{binary, shortHelp};
    charpointer nullTraceValue[]{binary, nullptr, validUid};
    charpointer nullSelectorValue[]{binary, trace, nullptr};
    charpointer unknownValues[]{binary, unknownOption, validUid};
    charpointer invalidSuffixValues[]{binary, trace, invalidSuffix};
    charpointer emptyTargetValues[]{binary, trace, emptyTarget};
    charpointer prefixOnlyValues[]{binary, trace, prefixOnly};
    charpointer invalidPrefixValues[]{binary, trace, invalidPrefix};
    charpointer invalidFirstDigitValues[]{binary, trace, invalidFirstDigit};
    charpointer invalidLastDigitValues[]{binary, trace, invalidLastDigit};
    charpointer validUidValues[]{binary, trace, validUid};
    charpointer disableAllValues[]{binary, trace, disableAll};
    charpointer jsonValues[]{binary, trace, json};

    const ArgumentsType defaultArguments(1, defaultValues);
    const ArgumentsType nullHelpArguments(2, nullptr);
    const ArgumentsType noOptionArguments(2, noOptionValue);
    const ArgumentsType helpArguments(2, helpValues);
    const ArgumentsType shortHelpArguments(2, shortHelpValues);
    const ArgumentsType nullTraceArguments(3, nullTraceValue);
    const ArgumentsType nullSelectorArguments(3, nullSelectorValue);
    const ArgumentsType unknownArguments(3, unknownValues);
    const ArgumentsType invalidSuffixArguments(3, invalidSuffixValues);
    const ArgumentsType emptyTargetArguments(3, emptyTargetValues);
    const ArgumentsType prefixOnlyArguments(3, prefixOnlyValues);
    const ArgumentsType invalidPrefixArguments(3, invalidPrefixValues);
    const ArgumentsType invalidFirstDigitArguments(3, invalidFirstDigitValues);
    const ArgumentsType invalidLastDigitArguments(3, invalidLastDigitValues);
    const ArgumentsType validUidArguments(3, validUidValues);
    const ArgumentsType disableAllArguments(3, disableAllValues);
    const ArgumentsType jsonArguments(3, jsonValues);

    EXPECT_TRUE(defaultArguments.valid());
    EXPECT_TRUE(defaultArguments.applyTraceUpdate());
    EXPECT_FALSE(nullHelpArguments.valid());
    EXPECT_FALSE(noOptionArguments.valid());
    EXPECT_TRUE(helpArguments.valid());
    EXPECT_TRUE(shortHelpArguments.valid());
    EXPECT_TRUE(shortHelpArguments.helpRequested());
    EXPECT_FALSE(nullTraceArguments.valid());
    EXPECT_FALSE(nullSelectorArguments.valid());
    EXPECT_FALSE(unknownArguments.valid());
    EXPECT_FALSE(invalidSuffixArguments.valid());
    EXPECT_FALSE(emptyTargetArguments.valid());
    EXPECT_FALSE(prefixOnlyArguments.valid());
    EXPECT_FALSE(invalidPrefixArguments.valid());
    EXPECT_FALSE(invalidFirstDigitArguments.valid());
    EXPECT_FALSE(invalidLastDigitArguments.valid());
    EXPECT_TRUE(validUidArguments.valid());
    EXPECT_TRUE(validUidArguments.applyTraceUpdate());
    EXPECT_TRUE(disableAllArguments.valid());
    EXPECT_TRUE(disableAllArguments.applyTraceUpdate());
    EXPECT_TRUE(jsonArguments.valid());
    EXPECT_FALSE(jsonArguments.applyTraceUpdate());
}

} /* namespace */

TEST(TEST_SUITE_XWALK_SIMULATION, TraceArgumentBoundaries)
{
    using namespace xwalk::hal::sim;

    verifyTraceArgumentBoundaries<XWalkAdcSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkAdxl345SimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkAudioSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkBoardControlSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkBuzzerSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkCameraSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkConfigSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkGpioSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkGptSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkI2cSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkLanguageModelSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkLedSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkLineTrackerSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkMotorSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkMusicSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkPwmSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkRobotSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkServoSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkSpeakerSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkSpiSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkUltrasonicSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkUserButtonSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkUtilsSimulationArguments>();
    verifyTraceArgumentBoundaries<XWalkVoiceAssistantSimulationArguments>();
}
