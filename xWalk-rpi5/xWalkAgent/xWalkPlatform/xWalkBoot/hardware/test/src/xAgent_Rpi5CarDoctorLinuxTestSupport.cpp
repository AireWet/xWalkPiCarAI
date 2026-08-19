/******************************************************************************
 * @file        xAgent_Rpi5CarDoctorLinuxTestSupport.cpp
 * @brief       Implements host-safe Doctor test assertions.
 *
 * @details
 * Checks typed Doctor outcomes and evidence text without accessing hardware.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarDoctorLinuxTestSupport.h"

#include <iostream>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent::test::doctor
 * @brief Contains host-safe Linux Doctor test support.
 */
namespace xwalk::agent::test::doctor
{

    /******************************************************************************
     * Function definitions
     ******************************************************************************/

    /**
     * @brief Writes one diagnostic and returns a failing process status.
     * @param[in] message Null-terminated diagnostic that remains valid for the call.
     * @return One.
     */
    int fail(const char* message)
    {
        std::cerr << message << '\n';
        return 1;
    }

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
                          const char* failureMessage)
    {
        const agent::boolean statusMatched = result.status == expectedStatus;
        const agent::boolean detailMatched = result.detail.find(detailFragment) != agent::string::npos;
        return statusMatched && detailMatched ? 0 : fail(failureMessage);
    }

} /* namespace xwalk::agent::test::doctor */
