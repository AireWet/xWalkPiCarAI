/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorAssessment.h
 * @brief       Declares bounded Doctor evidence and assessment decisions.
 *
 * @details
 * Separates Robot HAT verification and Doctor safety decisions from Linux
 * device access so every status transition remains host-testable.
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

#ifndef XAGENT_RPI5CAR_DOCTOR_ASSESSMENT_H
#define XAGENT_RPI5CAR_DOCTOR_ASSESSMENT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /******************************************************************************
     * Enumeration declarations
     ******************************************************************************/

    /**
     * @enum XWalkDoctorResultStatus
     * @brief Classifies one Doctor assessment without relying on report text.
     */
    enum class XWalkDoctorResultStatus
    {
        /**
         * @brief The available evidence satisfies the assessed invariant.
         */
        Pass,
        /**
         * @brief The available evidence is incomplete without a detected conflict.
         */
        Warn,
        /**
         * @brief The available evidence conflicts or violates a required invariant.
         */
        Fail
    };

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @struct XWalkDoctorGpioEvidence
     * @brief Records bounded GPIO inspection and reset outcomes.
     */
    struct XWalkDoctorGpioEvidence
    {
            /**
             * @brief Whether the selected controller met its configured minimum line count.
             */
            agent::boolean lineCountMatched{false};
            /**
             * @brief Whether the selected controller name and label exactly matched configuration.
             */
            agent::boolean identityMatched{false};
            /**
             * @brief Whether Doctor successfully requested only the configured reset output.
             */
            agent::boolean resetRequested{false};
            /**
             * @brief Whether Doctor completed low, high, and settle phases of the reset pulse.
             */
            agent::boolean resetCompleted{false};
    };

    /**
     * @struct XWalkDoctorI2cEvidence
     * @brief Records passive Robot HAT MCU and ADC inspection outcomes.
     */
    struct XWalkDoctorI2cEvidence
    {
            /**
             * @brief Whether the configured I2C character device opened successfully.
             */
            agent::boolean deviceOpened{false};
            /**
             * @brief Whether a Robot HAT MCU answered a firmware transaction at a supported address.
             */
            agent::boolean mcuResponded{false};
            /**
             * @brief Whether the three-byte firmware version was read successfully.
             */
            agent::boolean firmwareRead{false};
            /**
             * @brief Whether ADC channel A4 returned a complete battery sample.
             */
            agent::boolean batterySampleRead{false};
            /**
             * @brief Supported seven-bit MCU address that answered, or zero when none answered.
             */
            agent::uint8 mcuAddress{0U};
    };

    /**
     * @struct XWalkDoctorRobotHatEvidence
     * @brief Collects identity and operational evidence used for board verification.
     */
    struct XWalkDoctorRobotHatEvidence
    {
            /**
             * @brief Whether the supported Robot HAT v5 Device Tree UUID was detected.
             */
            agent::boolean v5UuidDetected{false};
            /**
             * @brief Whether the configured GPIO controller name and label matched exactly.
             */
            agent::boolean gpioIdentityMatched{false};
            /**
             * @brief Whether the bounded MCU reset completed and restored the line high.
             */
            agent::boolean resetCompleted{false};
            /**
             * @brief Whether the Robot HAT MCU answered at a supported I2C address.
             */
            agent::boolean mcuResponded{false};
            /**
             * @brief Whether the MCU firmware version was read successfully.
             */
            agent::boolean firmwareRead{false};
            /**
             * @brief Whether the battery ADC returned a complete sample.
             */
            agent::boolean batterySampleRead{false};
            /**
             * @brief Supported seven-bit MCU address that answered, or zero when unavailable.
             */
            agent::uint8 mcuAddress{0U};
    };

    /**
     * @struct XWalkDoctorOperationState
     * @brief Tracks every operation category bounded by Doctor safety policy.
     */
    struct XWalkDoctorOperationState
    {
            /**
             * @brief Whether the configured MCU reset output was requested.
             */
            agent::boolean resetRequested{false};
            /**
             * @brief Whether the configured reset pulse completed and restored high.
             */
            agent::boolean resetCompleted{false};
            /**
             * @brief Whether any motor or servo operation was activated.
             */
            agent::boolean actuatorActivated{false};
            /**
             * @brief Whether speaker power or audio playback was activated.
             */
            agent::boolean speakerActivated{false};
            /**
             * @brief Whether camera or microphone capture was activated.
             */
            agent::boolean mediaCaptureActivated{false};
            /**
             * @brief Whether an SPI payload transfer was activated.
             */
            agent::boolean spiTransferActivated{false};
            /**
             * @brief Whether a local or network model endpoint was called.
             */
            agent::boolean modelEndpointActivated{false};
    };

    /**
     * @struct XWalkDoctorAssessmentResult
     * @brief Owns one status and its precise report evidence.
     */
    struct XWalkDoctorAssessmentResult
    {
            /**
             * @brief Machine-testable assessment status.
             */
            XWalkDoctorResultStatus status{XWalkDoctorResultStatus::Warn};
            /**
             * @brief Human-readable evidence associated with the status.
             */
            agent::string detail{};
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkDoctorAssessment
     * @brief Converts collected Doctor evidence into deterministic report results.
     */
    class XWalkDoctorAssessment final
    {
        public:
            /**
             * @brief Assesses configured Robot HAT identity and operational evidence.
             *
             * @param[in] profile Explicit `robot_hat_v4`, `robot_hat_v5`, or fail-safe `auto` profile.
             * @param[in] evidence Read-only evidence collected without actuator or media activation.
             *
             * @return
             * PASS for positively verified profiles, FAIL for conflicts or unsafe automatic selection,
             * and WARN when an explicit v4 profile lacks complete operational evidence.
             */
            static XWalkDoctorAssessmentResult assessRobotHat(agent::stringview profile,
                                                              const XWalkDoctorRobotHatEvidence& evidence);

            /**
             * @brief Assesses the bounded Doctor operation invariant.
             *
             * @param[in] state Explicit record of every operation category Doctor can activate.
             *
             * @return
             * PASS only for a completed reset with no prohibited activation; otherwise FAIL.
             */
            static XWalkDoctorAssessmentResult assessSafety(const XWalkDoctorOperationState& state);
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_DOCTOR_ASSESSMENT_H */
