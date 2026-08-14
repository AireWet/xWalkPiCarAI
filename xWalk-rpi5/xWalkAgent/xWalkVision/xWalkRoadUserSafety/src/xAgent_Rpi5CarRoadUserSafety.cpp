/******************************************************************************
 * @file        xAgent_Rpi5CarRoadUserSafety.cpp
 * @brief       Implements model-neutral road-user safety evaluation.
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

#include "xAgent_Rpi5CarRoadUserSafety.h"

#include "xHal_Rpi5CarTrace.h"
#include <cmath>

namespace xwalk::agent {

/**
 * @brief Binds one complete caller-owned road-user safety provider.
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
XWalkRoadUserSafety::XWalkRoadUserSafety(
    agent::contextpointer context,
    const XWalkRoadUserSafetyCallbacks &providerCallbacks)
    : callbackContext(context), callbacks(providerCallbacks) {
  if ((callbacks.detect == nullptr) || (callbacks.classify == nullptr) ||
      (callbacks.alert == nullptr) || (callbacks.stopMotion == nullptr) ||
      (callbacks.reportFault == nullptr)) {
    XWALK_RPIAGENT_ERROR(XWALK_INVAL,
                         "Road-user safety requires complete callbacks");
  }
}

/**
 * @brief Reports whether every detector field is finite and within its
 * contract.
 *
 * @param[in] detection
 * Detector output to validate without invoking a classifier.
 *
 * @return
 * `true` only for supported classes and bounded finite numeric fields.
 */
agent::boolean XWalkRoadUserSafety::validDetection(
    const XWalkRoadUserDetection &detection) noexcept {
  const agent::boolean classValid =
      (detection.classIdentifier >=
       static_cast<agent::int32>(XWalkRoadUserClass::Person)) &&
      (detection.classIdentifier <=
       static_cast<agent::int32>(XWalkRoadUserClass::Motorbike));
  const agent::boolean numericValuesFinite =
      std::isfinite(detection.confidence) &&
      std::isfinite(detection.distanceMeters) &&
      std::isfinite(detection.closingSpeedMetersPerSecond) &&
      std::isfinite(detection.lateralOffsetNormalized) &&
      std::isfinite(detection.boundingAreaNormalized);
  return classValid && numericValuesFinite && (detection.confidence >= 0.0) &&
         (detection.confidence <= 1.0) && (detection.distanceMeters >= 0.0) &&
         (detection.lateralOffsetNormalized >= -1.0) &&
         (detection.lateralOffsetNormalized <= 1.0) &&
         (detection.boundingAreaNormalized >= 0.0) &&
         (detection.boundingAreaNormalized <= 1.0);
}

/**
 * @brief Converts one validated detection into model-neutral classifier
 * features.
 *
 * @param[in] detection
 * Detection previously accepted by `validDetection()`.
 *
 * @return
 * Feature record preserving class, confidence, distance, motion, and geometry.
 */
XWalkRoadRiskFeatures
XWalkRoadUserSafety::features(const XWalkRoadUserDetection &detection) {
  return {static_cast<XWalkRoadUserClass>(detection.classIdentifier),
          detection.confidence,
          detection.distanceMeters,
          detection.closingSpeedMetersPerSecond,
          std::fabs(detection.lateralOffsetNormalized),
          detection.boundingAreaNormalized};
}

/**
 * @brief Applies the non-throwing stop-and-fault response.
 *
 * @param[in] status
 * Non-Ok failure status reported to the caller-owned fault sink.
 *
 * @return
 * FailSafeStop result with zero accepted detections.
 *
 * @post
 * Both safety callbacks have been invoked once.
 */
XWalkRoadUserSafetyResult
XWalkRoadUserSafety::failSafe(XWalkRoadSafetyStatus status) noexcept {
  callbacks.stopMotion(callbackContext);
  callbacks.reportFault(callbackContext, status);
  return {XWalkRoadRisk::FailSafeStop, status, 0U};
}

/**
 * @brief Evaluates one bounded detector sample and applies its safety response.
 *
 * @return
 * Aggregate validated risk and status. Dangerous and fail-safe results request
 * motor stop before returning.
 *
 * @post
 * Invalid outputs and non-Ok provider statuses request stop-and-disarm.
 * Provider callbacks are non-throwing C-style status boundaries.
 */
XWalkRoadUserSafetyResult XWalkRoadUserSafety::evaluate() noexcept {
  const XWalkRoadUserDetectionBatch batch = callbacks.detect(callbackContext);
  if (batch.status != XWalkRoadSafetyStatus::Ok) {
    return failSafe(batch.status);
  }
  XWalkRoadRisk aggregateRisk{XWalkRoadRisk::Safe};
  agent::uint32 acceptedCount{};
  for (const XWalkRoadUserDetection &detection : batch.detections) {
    if (!validDetection(detection)) {
      return failSafe(XWalkRoadSafetyStatus::InvalidOutput);
    }
    const XWalkRoadRiskClassification classification =
        callbacks.classify(callbackContext, features(detection));
    if (classification.status != XWalkRoadSafetyStatus::Ok) {
      return failSafe(classification.status);
    }
    const agent::uint8 riskValue =
        static_cast<agent::uint8>(classification.risk);
    if (riskValue > static_cast<agent::uint8>(XWalkRoadRisk::Dangerous)) {
      return failSafe(XWalkRoadSafetyStatus::ClassifierFailure);
    }
    if (riskValue > static_cast<agent::uint8>(aggregateRisk)) {
      aggregateRisk = classification.risk;
    }
    ++acceptedCount;
  }
  if (aggregateRisk == XWalkRoadRisk::Dangerous) {
    callbacks.stopMotion(callbackContext);
  }
  if (aggregateRisk != XWalkRoadRisk::Safe) {
    callbacks.alert(callbackContext, aggregateRisk, acceptedCount);
  }
  return {aggregateRisk, XWalkRoadSafetyStatus::Ok, acceptedCount};
}

} /* namespace xwalk::agent */
