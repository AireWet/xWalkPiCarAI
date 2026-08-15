/******************************************************************************
 * @file        xControllerDeploymentConfig.h
 * @brief       Declares device-free deployment configuration diagnostics.
 * @project     xWalk Firmware
 * @module      xWalkController Application
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 ******************************************************************************/

#ifndef XCONTROLLER_DEPLOYMENT_CONFIG_H
#define XCONTROLLER_DEPLOYMENT_CONFIG_H

#include "xWalkControllerConfigTypes.h"

namespace xwalk::ctrl
{

    /** @brief Owns one non-actuating deployment configuration validation result. */
    struct XWalkDeploymentConfigReport
    {
            ::ctrl::boolean valid{};      /**< Whether all known deployment invariants passed. */
            ::ctrl::stringvector lines{}; /**< Sanitized human-readable validation results. */
    };

    /**
     * @brief Validates layered deployment configuration without opening hardware devices.
     * @param[in] configurationFilePath Readable absolute configuration manifest.
     * @return Sanitized validation report containing no environment values or secrets.
     */
    XWalkDeploymentConfigReport XWALK_validateDeploymentConfig(::ctrl::stringview configurationFilePath);

    /**
     * @brief Returns sanitized known effective configuration values.
     * @param[in] configurationFilePath Readable absolute configuration manifest.
     * @return Known effective values in stable schema order, with secret-shaped values redacted.
     * @throws std::runtime_error If layered configuration loading fails.
     */
    ::ctrl::stringvector XWALK_effectiveDeploymentConfig(::ctrl::stringview configurationFilePath);

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_DEPLOYMENT_CONFIG_H */
