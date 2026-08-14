/******************************************************************************
 * @file        xControllerSensorHandler.cpp
 * @brief       Implements the SensorHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

#include "xControllerParsing.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl {

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

/**
 * @brief Executes the sensor command.
 * @param[in] request Validated sensor-report selection.
 * @return Zero after sensor tracing completes.
 */
::ctrl::int32
XWalkController::XWALK_handlerSensor(const XWalkSensorRequest &request) {
  if (request.type == XWalkSensorType::Distance) {
    const ::ctrl::float64 distanceCm = picarxObject->distance();
    XWALK_CTRL_TRACE_UID1(CTRL .037, "%s",
                          distanceCm == 0.0
                              ? "None"
                              : XWALK_FORMAT_ONE_DECIMAL(distanceCm).c_str());
    return 0;
  }
  delay(300U);
  ::ctrl::fixedarray<hal::linetrackervalues, 5U> samples{};
  ::ctrl::size sampleCount{};
  const hal::linetrackervalues poisoned{2'571, 3'085, 3'599};
  for (::ctrl::uint32 attempt = 0U; attempt < 5U; ++attempt) {
    const hal::linetrackervalues data = picarxObject->grayscaleData();
    if ((data != poisoned) && (data[0U] < 2'000)) {
      samples[sampleCount] = data;
      ++sampleCount;
    }
    delay(100U);
  }

  hal::linetrackervalues data{};
  if (sampleCount > 0U) {
    hal::linetrackervalues reference{};
    for (::ctrl::uint32 channel = 0U; channel < 3U; ++channel) {
      ::ctrl::int32 sum{};
      for (::ctrl::size sample = 0U; sample < sampleCount; ++sample) {
        sum += samples[sample][channel];
      }
      reference[channel] = sum / static_cast<::ctrl::int32>(sampleCount);
    }
    picarxObject->setGrayscaleReference(reference);
    XWALK_CTRL_TRACE_UID1(CTRL .038, "Auto ref: %s",
                          XWALK_FORMAT_VALUES(reference).c_str());
    data = samples[sampleCount - 1U];
  } else {
    XWALK_CTRL_WARNING(XWALK_RANGE, "WARNING: ADC may be corrupted");
  }
  XWALK_CTRL_TRACE_UID1(CTRL .039, "Grayscale: %s",
                        XWALK_FORMAT_VALUES(data).c_str());
  XWALK_CTRL_TRACE_UID1(
      CTRL .040, "Line status: %s",
      XWALK_FORMAT_STATUS(picarxObject->lineStatus(data)).c_str());
  XWALK_CTRL_TRACE_UID1(CTRL .041, "Cliff detected: %s",
                        picarxObject->cliffStatus(data) ? "true" : "false");
  return 0;
}

} /* namespace xwalk::ctrl */
