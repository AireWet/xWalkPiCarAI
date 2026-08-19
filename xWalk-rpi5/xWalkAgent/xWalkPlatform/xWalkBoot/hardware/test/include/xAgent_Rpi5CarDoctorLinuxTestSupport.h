/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinuxTestSupport.h
 * @brief       Declares host-safe Doctor test assertions.
 *
 * @details
 * Provides reusable assessment checks without opening GPIO, I2C, SPI, camera,
 * audio, or model resources.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi Doctor Test
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

#ifndef XAGENT_RPI5CAR_DOCTOR_LINUX_TEST_SUPPORT_H
#define XAGENT_RPI5CAR_DOCTOR_LINUX_TEST_SUPPORT_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarDoctorAssessment.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test::doctor
 * @brief Contains host-safe Linux Doctor test support.
 */
namespace xwalk::agent::test::doctor
{

    /******************************************************************************
     * Function declarations
     ******************************************************************************/

    /**
     * @brief Writes one diagnostic and returns a failing process status.
     * @param[in] message Null-terminated diagnostic that remains valid for the call.
     * @return One.
     */
    int fail(const char* message);

    /**
     * @brief Verifies one assessment status and required detail fragment.
     * @param[in] result Assessment result under test.
     * @param[in] expectedStatus Required status.
     * @param[in] detailFragment Required non-empty detail fragment.
     * @param[in] failureMessage Null-terminated diagnostic that remains valid for the call.
     * @return Zero when both expectations match; otherwise one.
     */
    int requireAssessment(const XWalkDoctorAssessmentResult& result,
                          XWalkDoctorResultStatus expectedStatus,
                          agent::stringview detailFragment,
                          const char* failureMessage);

} /* namespace xwalk::agent::test::doctor */

#endif /* XAGENT_RPI5CAR_DOCTOR_LINUX_TEST_SUPPORT_H */
