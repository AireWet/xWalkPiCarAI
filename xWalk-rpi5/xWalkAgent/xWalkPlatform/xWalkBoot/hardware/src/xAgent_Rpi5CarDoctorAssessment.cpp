/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorAssessment.cpp
 * @brief       Implements bounded Doctor evidence assessments.
 *
 * @details
 * Produces deterministic Robot HAT verification and safety results without
 * opening Linux devices, enabling complete host-side decision coverage.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor
 *
 * @author      Joxy John
 * @date        2026-08-19
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

#include "xAgent_Rpi5CarDoctorAssessment.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Public member function definitions
     ******************************************************************************/

    /**
     * @brief Assesses configured Robot HAT identity and operational evidence.
     * @param[in] profile Explicit `robot_hat_v4`, `robot_hat_v5`, or fail-safe `auto` profile.
     * @param[in] evidence Read-only evidence collected without actuator or media activation.
     * @return PASS for verified profiles, FAIL for conflicts, or WARN for incomplete explicit-v4 evidence.
     */
    XWalkDoctorAssessmentResult XWalkDoctorAssessment::assessRobotHat(agent::stringview profile,
                                                                      const XWalkDoctorRobotHatEvidence& evidence)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .072, "Doctor evaluating Robot HAT evidence");
        if (profile == "robot_hat_v5")
        {
            if (evidence.v5UuidDetected)
            {
                return {XWalkDoctorResultStatus::Pass,
                        "Robot HAT v5 profile verified by Device Tree UUID "
                        "9daeea78-0000-076e-0032-582369ac3e02"};
            }
            return {XWalkDoctorResultStatus::Fail,
                    "Robot HAT v5 profile requires Device Tree UUID "
                    "9daeea78-0000-076e-0032-582369ac3e02"};
        }
        if (profile == "auto")
        {
            if (evidence.v5UuidDetected)
            {
                return {XWalkDoctorResultStatus::Pass,
                        "auto selected Robot HAT v5 from Device Tree UUID "
                        "9daeea78-0000-076e-0032-582369ac3e02"};
            }
            return {XWalkDoctorResultStatus::Fail,
                    "auto cannot select Robot HAT v4 from an absent v5 UUID; provision an explicit profile"};
        }
        if (profile != "robot_hat_v4")
        {
            return {XWalkDoctorResultStatus::Fail, "unsupported hardware_board value"};
        }
        if (evidence.v5UuidDetected)
        {
            return {XWalkDoctorResultStatus::Fail,
                    "provisioned v4 profile conflicts with detected Robot HAT v5 UUID "
                    "9daeea78-0000-076e-0032-582369ac3e02"};
        }
        const agent::boolean operationalEvidenceComplete = static_cast<agent::boolean>(
            evidence.gpioIdentityMatched && evidence.resetCompleted && evidence.mcuResponded && evidence.firmwareRead &&
            evidence.batterySampleRead && ((evidence.mcuAddress == 0x14U) || (evidence.mcuAddress == 0x15U)));
        if (operationalEvidenceComplete)
        {
            const agent::string address = evidence.mcuAddress == 0x14U ? "0x14" : "0x15";
            return {XWalkDoctorResultStatus::Pass,
                    "provisioned v4 profile, exact GPIO identity, MCU response at " + address +
                        ", firmware, and battery checks agree"};
        }
        return {XWalkDoctorResultStatus::Warn,
                "provisioned v4 profile has no v5 UUID conflict, but exact GPIO identity, MCU reset, supported MCU "
                "response, firmware, and battery evidence is incomplete"};
    }

    /**
     * @brief Assesses the bounded Doctor operation invariant.
     * @param[in] state Explicit record of every operation category Doctor can activate.
     * @return PASS only for a completed reset with no prohibited activation; otherwise FAIL.
     */
    XWalkDoctorAssessmentResult XWalkDoctorAssessment::assessSafety(const XWalkDoctorOperationState& state)
    {
        const agent::boolean prohibitedOperationActivated = static_cast<agent::boolean>(
            state.actuatorActivated || state.speakerActivated || state.mediaCaptureActivated ||
            state.spiTransferActivated || state.modelEndpointActivated);
        const agent::boolean boundedResetCompleted =
            static_cast<agent::boolean>(state.resetRequested && state.resetCompleted);
        if (!prohibitedOperationActivated && boundedResetCompleted)
        {
            return {XWalkDoctorResultStatus::Pass,
                    "MCU reset completed; no actuator, media, SPI-transfer, or model-endpoint operation was activated"};
        }
        if (prohibitedOperationActivated)
        {
            return {XWalkDoctorResultStatus::Fail,
                    "bounded-operation invariant violated by an actuator, speaker, media, SPI-transfer, or "
                    "model-endpoint activation"};
        }
        return {XWalkDoctorResultStatus::Fail,
                "bounded-operation invariant was not completed because the MCU reset did not finish"};
    }

} /* namespace xwalk::agent */
