/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafety.h
 * @brief       Declares the model-neutral road-user safety coordinator.
 *
 * @project     xWalk Firmware
 * @module      xWalkRoadUserSafety
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

#ifndef XAGENT_RPI5CAR_ROAD_USER_SAFETY_H
#define XAGENT_RPI5CAR_ROAD_USER_SAFETY_H

#include "xAgent_Rpi5CarRoadUserSafetyTypes.h"

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

    /**
     * @class XWalkRoadUserSafety
     * @brief Validates detections, classifies risk, and requests fail-safe motion stops.
     *
     * @details
     * The class owns no camera, inference runtime, model, classifier, or motor. All
     * dependencies are synchronous caller-owned callbacks that must outlive it.
     */
    class XWalkRoadUserSafety final
    {
        private:
            /** @brief Non-owning callback context that may be null when callbacks permit it. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Complete validated callback table copied during construction. */
            XWalkRoadUserSafetyCallbacks callbacks{};

            /** @brief Converts one validated detection into classifier features. */
            static XWalkRoadRiskFeatures features(const XWalkRoadUserDetection& detection);
            /** @brief Reports whether every numeric and class field is supported. */
            static agent::boolean validDetection(const XWalkRoadUserDetection& detection) noexcept;
            /** @brief Stops motion, reports a fault, and returns a fail-safe result. */
            XWalkRoadUserSafetyResult failSafe(XWalkRoadSafetyStatus status) noexcept;

        public:
            /**
             * @brief Binds one caller-owned detector, classifier, alert, and safety context.
             *
             * @param[in] context
             * Non-owning context forwarded synchronously to every callback.
             *
             * @param[in] providerCallbacks
             * Complete callback table copied during construction.
             *
             * @throws std::invalid_argument
             * If any required callback is null.
             */
            XWalkRoadUserSafety(agent::contextpointer context, const XWalkRoadUserSafetyCallbacks& providerCallbacks);

            /** @brief Destroys the coordinator without owning provider resources. */
            ~XWalkRoadUserSafety() = default;
            XWalkRoadUserSafety(const XWalkRoadUserSafety&) = delete;
            XWalkRoadUserSafety(XWalkRoadUserSafety&&) = delete;
            XWalkRoadUserSafety& operator=(const XWalkRoadUserSafety&) = delete;
            XWalkRoadUserSafety& operator=(XWalkRoadUserSafety&&) = delete;

            /**
             * @brief Evaluates one bounded detector sample and applies its safety response.
             *
             * @return
             * Safe, warning, dangerous, or fail-safe-stop result. Dangerous and
             * fail-safe results request motor stop before returning.
             *
             * @post
             * Invalid output or a non-Ok provider status requests stop-and-disarm
             * and requires recovery outside this coordinator.
             */
            XWalkRoadUserSafetyResult evaluate() noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_ROAD_USER_SAFETY_H */
