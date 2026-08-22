/******************************************************************************
 * @file        xHal_Rpi5CarRobotHatSoak.cpp
 * @brief       Runs a finite seeded logical Robot HAT simulator soak.
 * @project     xWalk Firmware
 * @module      xWalkRobotHatSimulationTest
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#include "xHal_Rpi5CarLogicalModels.h"

#include <dirent.h>
#include <unistd.h>

#include <array>
#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <limits>
#include "xHal_Rpi5CarRobotHatSoakTypes.h"

/******************************************************************************
 * Translation-unit type aliases
 ******************************************************************************/

using Options = ::xwalk::source_types::xhal_rpi5carrobothatsoak::Options;
using ProcessSnapshot = ::xwalk::source_types::xhal_rpi5carrobothatsoak::ProcessSnapshot;

namespace
{
    using namespace xwalk::hal;
    using namespace xwalk::hal::simulation;

    constexpr size FAULT_TYPE_COUNT{13U};

    boolean parseUnsigned(const char* text, uint64& value) noexcept
    {
        if ((text == nullptr) || (*text == '\0') || (*text == '-'))
        {
            return false;
        }
        errno = 0;
        char* end{};
        const unsigned long long parsed = std::strtoull(text, &end, 10);
        if ((errno != 0) || (end == text) || (*end != '\0'))
        {
            return false;
        }
        value = static_cast<uint64>(parsed);
        return true;
    }

    boolean parseRate(const char* text, float64& value) noexcept
    {
        errno = 0;
        char* end{};
        const float64 parsed = std::strtod(text, &end);
        if ((errno != 0) || (end == text) || (*end != '\0') || (parsed < 0.0) || (parsed > 1.0))
        {
            return false;
        }
        value = parsed;
        return true;
    }

    boolean parseOptions(int argumentCount, char** arguments, Options& options) noexcept
    {
        for (int index = 1; index < argumentCount; index += 2)
        {
            if (index + 1 >= argumentCount)
            {
                return false;
            }
            const stringview option(arguments[index]);
            if (option == "--seed")
            {
                const boolean seedValid = parseUnsigned(arguments[index + 1], options.seed);
                if (!seedValid)
                {
                    return false;
                }
            }
            else if (option == "--iterations")
            {
                const boolean iterationsValid = parseUnsigned(arguments[index + 1], options.iterations);
                if (!iterationsValid)
                {
                    return false;
                }
            }
            else if (option == "--logical-duration")
            {
                const boolean durationValid = parseUnsigned(arguments[index + 1], options.logicalDuration);
                if (!durationValid)
                {
                    return false;
                }
            }
            else if (option == "--fault-rate")
            {
                const boolean faultRateValid = parseRate(arguments[index + 1], options.faultRate);
                if (!faultRateValid)
                {
                    return false;
                }
            }
            else if (option == "--report")
            {
                options.report = arguments[index + 1];
            }
            else
            {
                return false;
            }
        }
        return (options.iterations > 0U) && (options.logicalDuration > 0U) && !options.report.empty() &&
               (options.report.size() <= 4'096U);
    }

    uint64 directoryEntries(const char* path) noexcept
    {
        DIR* directory = ::opendir(path);
        if (directory == nullptr)
        {
            return 0U;
        }
        uint64 count{};
        dirent* entry = ::readdir(directory);
        while (entry != nullptr)
        {
            ++count;
            entry = ::readdir(directory);
        }
        static_cast<void>(::closedir(directory));
        return count >= 2U ? count - 2U : 0U;
    }

    ProcessSnapshot processSnapshot()
    {
        ProcessSnapshot snapshot;
        std::ifstream status("/proc/self/statm");
        uint64 totalPages{};
        uint64 residentPages{};
        if (status >> totalPages >> residentPages)
        {
            const long pageSize = ::sysconf(_SC_PAGESIZE);
            if (pageSize > 0)
            {
                snapshot.residentBytes = residentPages * static_cast<uint64>(pageSize);
            }
        }
        snapshot.descriptors = directoryEntries("/proc/self/fd");
        snapshot.threads = directoryEntries("/proc/self/task");
        return snapshot;
    }

    uint64 nextRandom(uint64& state) noexcept
    {
        state ^= state << 13U;
        state ^= state >> 7U;
        state ^= state << 17U;
        return state;
    }

    float64 randomUnit(uint64& state) noexcept
    {
        return static_cast<float64>(nextRandom(state) & 0xFFFFFFU) / static_cast<float64>(0x1000000U);
    }

    boolean writeReport(const Options& options,
                        const ProcessSnapshot& baseline,
                        const ProcessSnapshot& final,
                        const std::array<uint64, FAULT_TYPE_COUNT>& faults,
                        uint64 safeStops,
                        uint64 maximumEvents,
                        boolean passed)
    {
        std::ofstream output(options.report, std::ios::trunc);
        if (!output)
        {
            return false;
        }
        output << "{\n  \"seed\": " << options.seed << ",\n  \"iterations\": " << options.iterations
               << ",\n  \"logical_duration\": " << options.logicalDuration
               << ",\n  \"fault_rate\": " << options.faultRate << ",\n  \"safe_stop_transitions\": " << safeStops
               << ",\n  \"maximum_event_count\": " << maximumEvents
               << ",\n  \"resident_bytes_baseline\": " << baseline.residentBytes
               << ",\n  \"resident_bytes_final\": " << final.residentBytes
               << ",\n  \"file_descriptors_baseline\": " << baseline.descriptors
               << ",\n  \"file_descriptors_final\": " << final.descriptors
               << ",\n  \"threads_baseline\": " << baseline.threads << ",\n  \"threads_final\": " << final.threads
               << ",\n  \"fault_counts\": [";
        for (size index = 0U; index < faults.size(); ++index)
        {
            output << (index == 0U ? "" : ",") << faults[index];
        }
        output << "],\n  \"passed\": " << (passed ? "true" : "false") << "\n}\n";
        return static_cast<boolean>(output);
    }
} /* namespace */

int main(int argumentCount, char** arguments)
{
    Options options;
    const boolean optionsValid = parseOptions(argumentCount, arguments, options);
    if (!optionsValid)
    {
        std::cerr << "usage: xWalkRobotHatSoakTest --seed N --iterations N "
                     "--logical-duration N --fault-rate 0..1 --report PATH\n";
        return 2;
    }
    XWalkLogicalModelConfiguration configuration;
    configuration.accelerationPerTick = 3.0;
    configuration.decelerationPerTick = 8.0;
    configuration.batteryReductionPerTick = 0.0;
    configuration.i2cFailureInterval = 17U;
    configuration.cameraDelayTicks = 2U;
    XWalkLogicalModelState model;
    XWalkLogicalModelStatus modelStatus = initializeLogicalModel(model, configuration);
    if (modelStatus != XWalkLogicalModelStatus::Ok)
    {
        return 3;
    }
    const ProcessSnapshot baseline = processSnapshot();
    std::array<uint64, FAULT_TYPE_COUNT> faults{};
    uint64 randomState = options.seed == 0U ? 1U : options.seed;
    uint64 safeStops{};
    uint64 maximumEvents{};
    boolean passed = true;
    const uint64 ticksPerIteration = std::max<uint64>(1U, options.logicalDuration / options.iterations);
    for (uint64 iteration = 0U; iteration < options.iterations; ++iteration)
    {
        if (!model.armed)
        {
            modelStatus = initializeLogicalModel(model, configuration);
            if (modelStatus != XWalkLogicalModelStatus::Ok)
            {
                passed = false;
                break;
            }
        }
        const float64 left = (randomUnit(randomState) * 200.0) - 100.0;
        const float64 right = (randomUnit(randomState) * 200.0) - 100.0;
        const XWalkLogicalModelStatus motorStatus = commandLogicalMotors(model, left, right);
        if (motorStatus != XWalkLogicalModelStatus::Ok)
        {
            passed = false;
            break;
        }
        const boolean fault = randomUnit(randomState) < options.faultRate;
        if (fault)
        {
            const size faultType = static_cast<size>(nextRandom(randomState) % FAULT_TYPE_COUNT);
            ++faults[faultType];
            enterLogicalSafeState(model);
            ++safeStops;
            if (model.armed || (model.commandedLeftSpeed != 0.0) || (model.commandedRightSpeed != 0.0) ||
                (model.simulatedLeftSpeed != 0.0) || (model.simulatedRightSpeed != 0.0))
            {
                passed = false;
                break;
            }
        }
        else
        {
            const XWalkLogicalModelStatus advanceStatus = advanceLogicalModel(model, ticksPerIteration);
            if (advanceStatus != XWalkLogicalModelStatus::Ok)
            {
                passed = false;
                break;
            }
        }
        maximumEvents = std::max<uint64>(maximumEvents, static_cast<uint64>(model.eventCount));
        if ((model.eventCount > XWALK_LOGICAL_MODEL_MAXIMUM_EVENTS) ||
            (model.logicalTime > options.logicalDuration + ticksPerIteration))
        {
            passed = false;
            break;
        }
    }
    enterLogicalSafeState(model);
    const ProcessSnapshot final = processSnapshot();
    constexpr uint64 MEMORY_TOLERANCE_BYTES{uint64{8U} * 1'024U * 1'024U};
    passed = passed && !model.armed && (model.commandedLeftSpeed == 0.0) && (model.commandedRightSpeed == 0.0) &&
             (final.residentBytes <= baseline.residentBytes + MEMORY_TOLERANCE_BYTES) &&
             (final.descriptors <= baseline.descriptors) && (final.threads <= baseline.threads);
    const boolean reportWritten = writeReport(options, baseline, final, faults, safeStops, maximumEvents, passed);
    if (!reportWritten)
    {
        return 4;
    }
    std::cout << "soak iterations=" << options.iterations << " faults=" << safeStops
              << " result=" << (passed ? "passed" : "failed") << '\n';
    return passed ? 0 : 1;
}
