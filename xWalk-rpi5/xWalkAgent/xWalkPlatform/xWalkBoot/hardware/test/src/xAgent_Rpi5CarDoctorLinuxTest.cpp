/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinuxTest.cpp
 * @brief       Verifies host-safe Linux Doctor decisions and executable discovery.
 *
 * @details
 * Exercises Robot HAT and bounded-operation assessments plus executable lookup
 * without opening any hardware device.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor Test
 * @author      Joxy John
 * @date        2026-08-18
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarDoctorLinux.h"
#include "xAgent_Rpi5CarDoctorLinuxTestSupport.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

#include <cstdlib>
#include <limits.h>

/**
 * @brief Verifies absolute lookup, PATH lookup, and ignored empty PATH entries.
 * @return Zero on success; otherwise non-zero.
 */
int main()
{
    using xwalk::agent::XWalkDoctorAssessment;
    using xwalk::agent::XWalkDoctorOperationState;
    using xwalk::agent::XWalkDoctorResultStatus;
    using xwalk::agent::XWalkDoctorRobotHatEvidence;
    using xwalk::agent::test::doctor::requireAssessment;

    const XWalkDoctorRobotHatEvidence verifiedV4{false, true, true, true, true, true, 0x14U};
    const auto verifiedV4Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v4", verifiedV4);
    const agent::int32 verifiedV4Result = requireAssessment(verifiedV4Assessment,
                                                            XWalkDoctorResultStatus::Pass,
                                                            "MCU response at 0x14",
                                                            "complete Robot HAT v4 evidence did not pass");
    if (verifiedV4Result != 0)
    {
        return 1;
    }
    XWalkDoctorRobotHatEvidence incompleteV4 = verifiedV4;
    incompleteV4.gpioIdentityMatched = false;
    auto incompleteV4Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v4", incompleteV4);
    agent::int32 incompleteV4Result = requireAssessment(incompleteV4Assessment,
                                                        XWalkDoctorResultStatus::Warn,
                                                        "evidence is incomplete",
                                                        "mismatched v4 GPIO identity was not incomplete");
    if (incompleteV4Result != 0)
    {
        return 1;
    }
    incompleteV4 = verifiedV4;
    incompleteV4.mcuResponded = false;
    incompleteV4Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v4", incompleteV4);
    incompleteV4Result = requireAssessment(incompleteV4Assessment,
                                           XWalkDoctorResultStatus::Warn,
                                           "evidence is incomplete",
                                           "missing v4 MCU response was not incomplete");
    if (incompleteV4Result != 0)
    {
        return 1;
    }
    incompleteV4 = verifiedV4;
    incompleteV4.firmwareRead = false;
    incompleteV4Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v4", incompleteV4);
    incompleteV4Result = requireAssessment(incompleteV4Assessment,
                                           XWalkDoctorResultStatus::Warn,
                                           "evidence is incomplete",
                                           "v4 firmware-read failure was not incomplete");
    if (incompleteV4Result != 0)
    {
        return 1;
    }
    incompleteV4 = verifiedV4;
    incompleteV4.batterySampleRead = false;
    incompleteV4Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v4", incompleteV4);
    incompleteV4Result = requireAssessment(incompleteV4Assessment,
                                           XWalkDoctorResultStatus::Warn,
                                           "evidence is incomplete",
                                           "v4 battery-read failure was not incomplete");
    if (incompleteV4Result != 0)
    {
        return 1;
    }
    XWalkDoctorRobotHatEvidence conflictingV4 = verifiedV4;
    conflictingV4.v5UuidDetected = true;
    const auto conflictingV4Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v4", conflictingV4);
    const agent::int32 conflictingV4Result =
        requireAssessment(conflictingV4Assessment,
                          XWalkDoctorResultStatus::Fail,
                          "conflicts with detected Robot HAT v5 UUID",
                          "conflicting Robot HAT v5 UUID did not fail v4 verification");
    if (conflictingV4Result != 0)
    {
        return 1;
    }
    const XWalkDoctorRobotHatEvidence verifiedV5{true, false, false, false, false, false, 0U};
    const auto verifiedV5Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v5", verifiedV5);
    const agent::int32 verifiedV5Result = requireAssessment(verifiedV5Assessment,
                                                            XWalkDoctorResultStatus::Pass,
                                                            "9daeea78-0000-076e-0032-582369ac3e02",
                                                            "supported Robot HAT v5 UUID did not pass");
    if (verifiedV5Result != 0)
    {
        return 1;
    }
    const XWalkDoctorRobotHatEvidence missingV5{};
    const auto missingV5Assessment = XWalkDoctorAssessment::assessRobotHat("robot_hat_v5", missingV5);
    const agent::int32 missingV5Result = requireAssessment(missingV5Assessment,
                                                           XWalkDoctorResultStatus::Fail,
                                                           "requires Device Tree UUID",
                                                           "Robot HAT v5 without its UUID did not fail");
    if (missingV5Result != 0)
    {
        return 1;
    }
    const auto automaticAssessment = XWalkDoctorAssessment::assessRobotHat("auto", missingV5);
    const agent::int32 automaticResult = requireAssessment(automaticAssessment,
                                                           XWalkDoctorResultStatus::Fail,
                                                           "cannot select Robot HAT v4",
                                                           "automatic selection inferred Robot HAT v4");
    if (automaticResult != 0)
    {
        return 1;
    }

    const XWalkDoctorOperationState boundedState{true, true, false, false, false, false, false};
    const auto boundedAssessment = XWalkDoctorAssessment::assessSafety(boundedState);
    const agent::int32 boundedResult = requireAssessment(boundedAssessment,
                                                         XWalkDoctorResultStatus::Pass,
                                                         "MCU reset completed",
                                                         "successful bounded reset did not pass Safety");
    if (boundedResult != 0)
    {
        return 1;
    }
    XWalkDoctorOperationState failedResetState = boundedState;
    failedResetState.resetCompleted = false;
    const auto failedResetAssessment = XWalkDoctorAssessment::assessSafety(failedResetState);
    const agent::int32 failedResetResult = requireAssessment(failedResetAssessment,
                                                             XWalkDoctorResultStatus::Fail,
                                                             "reset did not finish",
                                                             "failed reset did not fail Safety");
    if (failedResetResult != 0)
    {
        return 1;
    }
    XWalkDoctorOperationState violatedState = boundedState;
    violatedState.spiTransferActivated = true;
    const auto violatedAssessment = XWalkDoctorAssessment::assessSafety(violatedState);
    const agent::int32 violatedResult = requireAssessment(violatedAssessment,
                                                          XWalkDoctorResultStatus::Fail,
                                                          "invariant violated",
                                                          "prohibited Doctor operation did not fail Safety");
    if (violatedResult != 0)
    {
        return 1;
    }

    char directoryTemplate[] = "/tmp/xwalk-doctor-path.XXXXXX";
    char* const directory = ::mkdtemp(directoryTemplate);
    if (directory == nullptr)
    {
        return xwalk::agent::test::doctor::fail("mkdtemp failed");
    }
    const agent::string executableName("xwalk-doctor-path-fixture");
    const agent::string executablePath = agent::string(directory) + "/" + executableName;
    const agent::int32 descriptor = ::open(executablePath.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC, 0700);
    if (descriptor < 0)
    {
        static_cast<void>(::rmdir(directory));
        return xwalk::agent::test::doctor::fail("fixture creation failed");
    }
    constexpr char fixture[] = "#!/bin/sh\nexit 0\n";
    const agent::boolean fixtureWritten =
        ::write(descriptor, fixture, sizeof(fixture) - 1U) == static_cast<ssize_t>(sizeof(fixture) - 1U);
    static_cast<void>(::close(descriptor));

    char originalDirectory[PATH_MAX]{};
    const agent::boolean directoryRecorded = ::getcwd(originalDirectory, sizeof(originalDirectory)) != nullptr;
    const agent::cstring originalPathValue = std::getenv("PATH");
    const agent::string originalPath =
        originalPathValue == nullptr ? agent::string() : agent::string(originalPathValue);
    const agent::boolean absoluteFound = xwalk::agent::XWalkDoctorLinux::executableAvailable(executablePath);
    const agent::boolean changedDirectory = ::chdir(directory) == 0;
    const agent::boolean emptyPathSet = ::setenv("PATH", ":", 1) == 0;
    const agent::boolean emptyEntryIgnored = !xwalk::agent::XWalkDoctorLinux::executableAvailable(executableName);
    const agent::boolean localPathSet = ::setenv("PATH", directory, 1) == 0;
    const agent::boolean localPathFound = xwalk::agent::XWalkDoctorLinux::executableAvailable(executableName);

    const agent::boolean directoryRestored =
        static_cast<agent::boolean>((!directoryRecorded) || (::chdir(originalDirectory) == 0));
    if (originalPathValue == nullptr)
    {
        static_cast<void>(::unsetenv("PATH"));
    }
    else
    {
        static_cast<void>(::setenv("PATH", originalPath.c_str(), 1));
    }
    static_cast<void>(::unlink(executablePath.c_str()));
    static_cast<void>(::rmdir(directory));

    if (!fixtureWritten || !directoryRecorded || !changedDirectory || !emptyPathSet || !localPathSet ||
        !directoryRestored)
    {
        return xwalk::agent::test::doctor::fail("test environment setup failed");
    }
    if (!absoluteFound)
    {
        return xwalk::agent::test::doctor::fail("absolute executable was not found");
    }
    if (!emptyEntryIgnored)
    {
        return xwalk::agent::test::doctor::fail("an empty PATH entry was treated as the working directory");
    }
    if (!localPathFound)
    {
        return xwalk::agent::test::doctor::fail("user-local PATH executable was not found");
    }
    return 0;
}
