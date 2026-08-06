/******************************************************************************
 * @file        xControllerSensorHandler.cpp
 * @brief       Implements the SensorHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based handler group.
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

#include "xControllerParsing.h"

#include "xHal_Rpi5CarExceptions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

/**
 * @brief Executes the sensor command.
 * @param[in] request Validated sensor-report selection.
 * @return Zero after sensor output completes.
 */
::ctrl::int32 XWalkController::XWALK_handlerSensor(const XWalkSensorRequest& request)
{
    if (request.type == XWalkSensorType::Distance)
    {
        const ::ctrl::float64 distanceCm = picarxObject->distance();
        output((distanceCm == 0.0) ? ::ctrl::string("None") : XWALK_formatOneDecimal(distanceCm));
        return 0;
    }
    delay(300U);
    ::ctrl::fixedarray<hal::linetrackervalues, 5U> samples{};
    ::ctrl::size sampleCount{};
    const hal::linetrackervalues poisoned{2'571, 3'085, 3'599};
    for (::ctrl::uint32 attempt = 0U; attempt < 5U; ++attempt)
    {
        const hal::linetrackervalues data = picarxObject->grayscaleData();
        if ((data != poisoned) && (data[0U] < 2'000))
        {
            samples[sampleCount] = data;
            ++sampleCount;
        }
        delay(100U);
    }

    hal::linetrackervalues data{};
    if (sampleCount > 0U)
    {
        hal::linetrackervalues reference{};
        for (::ctrl::uint32 channel = 0U; channel < 3U; ++channel)
        {
            ::ctrl::int32 sum{};
            for (::ctrl::size sample = 0U; sample < sampleCount; ++sample)
            {
                sum += samples[sample][channel];
            }
            reference[channel] = sum / static_cast<::ctrl::int32>(sampleCount);
        }
        picarxObject->setGrayscaleReference(reference);
        output(::ctrl::string("Auto ref: ") + XWALK_formatValues(reference));
        data = samples[sampleCount - 1U];
    }
    else
    {
        output("WARNING: ADC may be corrupted");
    }
    output(::ctrl::string("Grayscale: ") + XWALK_formatValues(data));
    output(::ctrl::string("Line status: ") + XWALK_formatStatus(picarxObject->lineStatus(data)));
    output(::ctrl::string("Cliff detected: ") + (picarxObject->cliffStatus(data) ? "true" : "false"));
    return 0;
}

} /* namespace xwalk::ctrl */
