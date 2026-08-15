/******************************************************************************
 * @file        xHal_Rpi5CarSequenceTestRunner.cpp
 * @brief       Implements centralized physical-sequence CLI dispatch.
 *
 * @details
 * Parses bounded numeric arguments, validates selector-specific arity, creates
 * required Linux objects, and invokes the selected physical sequence adapter.
 *
 * @project     xWalk Firmware
 * @module      xSequenceTest Hardware
 *
 * @author      Joxy John
 * @date        2026-08-03
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSequenceTestRunner.h"

#include "xHal_Rpi5CarButtonEventSequenceLinux.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarInitAnglesSequenceLinux.h"
#include "xHal_Rpi5CarMotorSequenceLinux.h"
#include "xHal_Rpi5CarRobotHat5MotorSequenceLinux.h"
#include "xHal_Rpi5CarServoHatSequenceLinux.h"
#include "xHal_Rpi5CarServoSequenceLinux.h"
#include "xHal_Rpi5CarToneSequenceLinux.h"

#include <charconv>
#include <iostream>
#include <yaml-cpp/yaml.h>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::test
{

    /** @brief Prints every supported selector and its required arguments. */
    void XWalkSequenceTestRunner::printUsage()
    {
        std::cerr << "usage:\n"
                     "  xSequenceTest [--config <yaml-file>] <sequence-name>\n"
                     "  xSequenceTest [--config=<yaml-file>] <sequence-name>\n"
                     "  formal-argument compatibility forms:\n"
                     "  xSequenceTest button-event <duration-seconds> <gpio-device> "
                     "<chip-name> <chip-label>\n"
                     "  xSequenceTest init-angles <i2c-device> <gpio-device> "
                     "<chip-name> <chip-label> <config-file>\n"
                     "  xSequenceTest motor <cycles> <i2c-device> <gpio-device> "
                     "<chip-name> <chip-label>\n"
                     "  xSequenceTest motor-robothat5 <cycles> <i2c-device>\n"
                     "  xSequenceTest servo-hat <samples> <i2c-device> <gpio-device> "
                     "<chip-name> <chip-label>\n"
                     "  xSequenceTest servo <cycles> <i2c-device>\n"
                     "  xSequenceTest tone <pcm-device> <mixer-device> <mixer-element>\n";
    }

    /**
     * @brief Validates and runs the button-event selector.
     *
     * @param[in] argumentCount Required value of six.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after monitoring, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runButtonEvent(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 6)
        {
            printUsage();
            return 2;
        }

        const stringview durationText(argumentValues[2U]);
        uint32 duration{};
        const std::from_chars_result parseResult =
            std::from_chars(durationText.data(), durationText.data() + durationText.size(), duration);
        const hal::boolean durationInvalid = static_cast<hal::boolean>(
            (parseResult.ec != std::errc{}) || (parseResult.ptr != durationText.data() + durationText.size()) ||
            (duration == 0U) || (duration > 3'600U));
        if (durationInvalid)
        {
            std::cerr << "duration-seconds must be an integer from 1 to 3600\n";
            return 2;
        }

        XWalkGpioLinux backend(argumentValues[3U], argumentValues[4U], argumentValues[5U], 18U);
        const XWalkGpioCallbacks callbacks = XHAL_GPIO_CALLBACKS(XWalkGpioLinux);
        XWalkGpio gpio(&backend, callbacks, "D0", XWalkGpioMode::Input, XWalkGpioPull::Up);
        XWalkButtonEventSequenceLinux linuxSequence;
        linuxSequence.run(gpio, duration);
        return 0;
    }

    /**
     * @brief Validates and runs the initialization-angle selector.
     *
     * @param[in] argumentCount Required value of seven.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runInitAngles(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 7)
        {
            printUsage();
            return 2;
        }

        XWalkInitAnglesSequenceLinux linuxSequence;
        linuxSequence.run(
            argumentValues[2U], argumentValues[3U], argumentValues[4U], argumentValues[5U], argumentValues[6U]);
        return 0;
    }

    /**
     * @brief Validates and runs the Robot HAT v5 motor selector.
     *
     * @param[in] argumentCount Required value of four.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runRobotHat5Motor(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 4)
        {
            printUsage();
            return 2;
        }

        const stringview cycleText(argumentValues[2U]);
        uint32 cycleCount{};
        const std::from_chars_result parseResult =
            std::from_chars(cycleText.data(), cycleText.data() + cycleText.size(), cycleCount);
        const hal::boolean parseResultEcPtrCycleTextCycleCountInvalid = static_cast<hal::boolean>(
            (parseResult.ec != std::errc{}) || (parseResult.ptr != cycleText.data() + cycleText.size()) ||
            (cycleCount == 0U) || (cycleCount > XHAL_RPI5CAR_ROBOTHAT5_MOTOR_MAX_CYCLES));
        if (parseResultEcPtrCycleTextCycleCountInvalid)
        {
            std::cerr << "cycles must be an integer from 1 to 100\n";
            return 2;
        }

        XWalkRobotHat5MotorSequenceLinux linuxSequence;
        linuxSequence.run(argumentValues[3U], cycleCount);
        return 0;
    }

    /**
     * @brief Validates and runs the two-motor selector.
     *
     * @param[in] argumentCount Required value of seven.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runMotor(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 7)
        {
            printUsage();
            return 2;
        }

        const stringview cycleText(argumentValues[2U]);
        uint32 cycleCount{};
        const std::from_chars_result parseResult =
            std::from_chars(cycleText.data(), cycleText.data() + cycleText.size(), cycleCount);
        const hal::boolean cycleCountInvalid = static_cast<hal::boolean>(
            (parseResult.ec != std::errc{}) || (parseResult.ptr != cycleText.data() + cycleText.size()) ||
            (cycleCount == 0U) || (cycleCount > XHAL_RPI5CAR_MOTOR_SEQUENCE_MAX_CYCLES));
        if (cycleCountInvalid)
        {
            std::cerr << "cycles must be an integer from 1 to 100\n";
            return 2;
        }

        XWalkMotorSequenceLinux linuxSequence;
        linuxSequence.run(argumentValues[3U], argumentValues[4U], argumentValues[5U], argumentValues[6U], cycleCount);
        return 0;
    }

    /**
     * @brief Validates and runs the Servo HAT selector.
     *
     * @param[in] argumentCount Required value of seven.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runServoHat(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 7)
        {
            printUsage();
            return 2;
        }

        const stringview sampleText(argumentValues[2U]);
        uint32 sampleCount{};
        const std::from_chars_result parseResult =
            std::from_chars(sampleText.data(), sampleText.data() + sampleText.size(), sampleCount);
        const hal::boolean parseResultEcPtrSampleTextSampleCountInvalid = static_cast<hal::boolean>(
            (parseResult.ec != std::errc{}) || (parseResult.ptr != sampleText.data() + sampleText.size()) ||
            (sampleCount == 0U) || (sampleCount > XHAL_RPI5CAR_SERVO_HAT_MAX_SAMPLES));
        if (parseResultEcPtrSampleTextSampleCountInvalid)
        {
            std::cerr << "samples must be an integer from 1 to 3600\n";
            return 2;
        }

        XWalkServoHatSequenceLinux linuxSequence;
        linuxSequence.run(argumentValues[3U], argumentValues[4U], argumentValues[5U], argumentValues[6U], sampleCount);
        return 0;
    }

    /**
     * @brief Validates and runs the 12-channel servo selector.
     *
     * @param[in] argumentCount Required value of four.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runServo(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 4)
        {
            printUsage();
            return 2;
        }

        const stringview cycleText(argumentValues[2U]);
        uint32 cycleCount{};
        const std::from_chars_result parseResult =
            std::from_chars(cycleText.data(), cycleText.data() + cycleText.size(), cycleCount);
        const hal::boolean cycleCountInvalid = static_cast<hal::boolean>(
            (parseResult.ec != std::errc{}) || (parseResult.ptr != cycleText.data() + cycleText.size()) ||
            (cycleCount == 0U) || (cycleCount > XHAL_RPI5CAR_SERVO_SEQUENCE_MAX_CYCLES));
        if (cycleCountInvalid)
        {
            std::cerr << "cycles must be an integer from 1 to 100\n";
            return 2;
        }

        XWalkServoSequenceLinux linuxSequence;
        linuxSequence.run(argumentValues[3U], cycleCount);
        return 0;
    }

    /**
     * @brief Validates and runs the tone selector.
     *
     * @param[in] argumentCount Required value of five.
     * @param[in] argumentValues Selector arguments in documented order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runTone(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount != 5)
        {
            printUsage();
            return 2;
        }

        XWalkToneSequenceLinux linuxSequence;
        linuxSequence.run(argumentValues[2U], argumentValues[3U], argumentValues[4U]);
        return 0;
    }

    /**
     * @brief Resolves and runs one physical sequence selector.
     *
     * @param[in] argumentCount Executable, selector, and selector arguments.
     * @param[in] argumentValues Process arguments in selector-specific order.
     * @return Zero after completion, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::runSelection(int32 argumentCount, char* argumentValues[])
    {
        if (argumentCount < 2)
        {
            printUsage();
            return 2;
        }

        const stringview selection(argumentValues[1U]);
        if (selection == "button-event")
        {
            return runButtonEvent(argumentCount, argumentValues);
        }
        if (selection == "init-angles")
        {
            return runInitAngles(argumentCount, argumentValues);
        }
        if (selection == "motor-robothat5")
        {
            return runRobotHat5Motor(argumentCount, argumentValues);
        }
        if (selection == "motor")
        {
            return runMotor(argumentCount, argumentValues);
        }
        if (selection == "servo-hat")
        {
            return runServoHat(argumentCount, argumentValues);
        }
        if (selection == "servo")
        {
            return runServo(argumentCount, argumentValues);
        }
        if (selection == "tone")
        {
            return runTone(argumentCount, argumentValues);
        }

        std::cerr << "unknown sequence: " << selection << '\n';
        printUsage();
        return 2;
    }

    /**
     * @brief Loads and runs one sequence from a YAML argument list.
     *
     * @param[in] executable Executable name retained as argument zero.
     * @param[in] selection Exact key below the YAML `sequences` mapping.
     * @param[in] configurationPath Readable YAML configuration path.
     * @return Selected sequence status, or two for invalid YAML or arguments.
     */
    int32
    XWalkSequenceTestRunner::runConfigured(stringview executable, stringview selection, stringview configurationPath)
    {
        const YAML::Node root = YAML::LoadFile(string(configurationPath));
        const hal::boolean mapNotMatched = static_cast<hal::boolean>(!root.IsMap());
        if (mapNotMatched)
        {
            std::cerr << "xSequenceTest YAML configuration is invalid for '" << selection << "': " << configurationPath
                      << '\n';
            return 2;
        }
        const YAML::Node schemaVersion = root["schema_version"];
        const YAML::Node sequences = root["sequences"];
        YAML::Node sequence;
        const hal::boolean mapMatched = static_cast<hal::boolean>(sequences.IsMap());
        if (mapMatched)
        {
            for (YAML::const_iterator iterator = sequences.begin(); iterator != sequences.end(); ++iterator)
            {
                const hal::boolean selectionMatched = static_cast<hal::boolean>(
                    iterator->first.IsScalar() && (iterator->first.as<string>() == selection));
                if (selectionMatched)
                {
                    sequence = iterator->second;
                    break;
                }
            }
        }
        const YAML::Node arguments = sequence.IsMap() ? sequence["arguments"] : YAML::Node();
        const hal::boolean argumentsInvalid =
            static_cast<hal::boolean>(!schemaVersion.IsScalar() || (schemaVersion.as<uint32>() != 1U) ||
                                      !sequences.IsMap() || !sequence.IsMap() || !arguments.IsSequence());
        if (argumentsInvalid)
        {
            std::cerr << "xSequenceTest YAML configuration is invalid for '" << selection << "': " << configurationPath
                      << '\n';
            return 2;
        }

        stringvector values{string(executable), string(selection)};
        for (const YAML::Node& argument : arguments)
        {
            const hal::boolean scalarNotMatched = static_cast<hal::boolean>(!argument.IsScalar());
            if (scalarNotMatched)
            {
                std::cerr << "xSequenceTest YAML arguments must be scalar values: " << configurationPath << '\n';
                return 2;
            }
            values.push_back(argument.as<string>());
        }

        charpointervector argumentPointers;
        argumentPointers.reserve(values.size() + 1U);
        for (string& value : values)
        {
            argumentPointers.push_back(value.data());
        }
        argumentPointers.push_back(nullptr);
        return runSelection(static_cast<int32>(values.size()), argumentPointers.data());
    }

    /**
     * @brief Resolves optional YAML configuration and runs one sequence.
     *
     * @param[in] argumentCount Executable, optional YAML path, selector, and optional formal arguments.
     * @param[in] argumentValues Mutable process argument array.
     * @return Selected sequence status, or two for invalid input.
     */
    int32 XWalkSequenceTestRunner::run(int32 argumentCount, char* argumentValues[])
    {
        string configurationPath{XHAL_RPI5CAR_SEQUENCE_TEST_YAML_PATH};
        stringvector values;
        boolean configurationSeen = false;
        for (int32 index = 0; index < argumentCount; ++index)
        {
            const string argument(argumentValues[index]);
            if ((index > 0) && (argument == "--config"))
            {
                if (configurationSeen || ((index + 1) >= argumentCount))
                {
                    std::cerr << "--config requires one YAML path and may appear once\n";
                    return 2;
                }
                configurationPath = argumentValues[index + 1];
                configurationSeen = true;
                ++index;
                continue;
            }
            const hal::boolean configAssignmentMatched =
                static_cast<hal::boolean>((index > 0) && (argument.rfind("--config=", 0U) == 0U));
            if (configAssignmentMatched)
            {
                const hal::boolean configurationInvalid =
                    static_cast<hal::boolean>(configurationSeen || (argument.size() == 9U));
                if (configurationInvalid)
                {
                    std::cerr << "--config requires one YAML path and may appear once\n";
                    return 2;
                }
                configurationPath = argument.substr(9U);
                configurationSeen = true;
                continue;
            }
            values.push_back(argument);
        }

        const hal::boolean valuesTooSmall = static_cast<hal::boolean>(values.size() < 2U);
        if (valuesTooSmall)
        {
            printUsage();
            return 2;
        }
        const hal::boolean valuesMatched = static_cast<hal::boolean>(values.size() == 2U);
        if (valuesMatched)
        {
            return runConfigured(values[0U], values[1U], configurationPath);
        }

        charpointervector argumentPointers;
        argumentPointers.reserve(values.size() + 1U);
        for (string& value : values)
        {
            argumentPointers.push_back(value.data());
        }
        argumentPointers.push_back(nullptr);
        return runSelection(static_cast<int32>(values.size()), argumentPointers.data());
    }

} /* namespace xwalk::hal::test */
