/******************************************************************************
 * @file        xAgent_Rpi5CarMoveExampleTypes.h
 * @brief       Declares callback types for the bounded movement example.
 * @project     xWalk Firmware
 * @module      xWalkMoveExample
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_MOVE_EXAMPLE_TYPES_H
#define XAGENT_RPI5CAR_MOVE_EXAMPLE_TYPES_H

#include "xHal_Rpi5CarTypes.h"

namespace xwalk::agent
{

    using moveexampledelaycallback = void (*)(agent::contextpointer context, agent::uint32 durationMs);
    using moveexamplecontinuecallback = agent::boolean (*)(agent::contextpointer context);

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_MOVE_EXAMPLE_TYPES_H */
