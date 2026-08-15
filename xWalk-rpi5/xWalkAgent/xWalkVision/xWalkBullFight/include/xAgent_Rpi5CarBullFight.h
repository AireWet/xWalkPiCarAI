/******************************************************************************
 * @file        xAgent_Rpi5CarBullFight.h
 * @brief       Declares bounded red-target pursuit.
 * @project     xWalk Firmware
 * @module      xWalkBullFight
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_BULL_FIGHT_H
#define XAGENT_RPI5CAR_BULL_FIGHT_H

#include "xAgent_Rpi5CarBullFightTypes.h"
#include "xAgent_Rpi5CarPicarx.h"

namespace xwalk::agent
{

    /** @brief Ports `10.bull_fight.py` through caller-owned vehicle and vision services. */
    class XWalkBullFight final
    {
        private:
            XWalkPicarx* picarxObject{nullptr};
            agent::contextpointer callbackContext{nullptr};
            XWalkComputerVisionCallbacks callbacks{};
            XWalkBullFightConfiguration configurationValue{};
            agent::float64 panAngleDegreesValue{};
            agent::float64 tiltAngleDegreesValue{};
            agent::float64 directionAngleDegreesValue{};
            agent::boolean startedValue{};

        protected:
            static void validate(const XWalkComputerVisionCallbacks& providerCallbacks,
                                 const XWalkBullFightConfiguration& configuration);
            agent::float64 constrainCameraAngle(agent::float64 angleDegrees) const noexcept;
            agent::boolean wait(agent::uint32 durationMs) const;

        public:
            XWalkBullFight(XWalkPicarx& picarx,
                           agent::contextpointer context,
                           const XWalkComputerVisionCallbacks& providerCallbacks,
                           const XWalkBullFightConfiguration& configuration = {});
            ~XWalkBullFight() noexcept;

            XWalkBullFight(const XWalkBullFight&) = delete;
            XWalkBullFight(XWalkBullFight&&) = delete;
            XWalkBullFight& operator=(const XWalkBullFight&) = delete;
            XWalkBullFight& operator=(XWalkBullFight&&) = delete;

            /** @brief Starts the camera, selects red detection, and resets retained angles. */
            agent::boolean start();
            /** @brief Applies one detected-target pursuit or zero-power search step. */
            XWalkBullFightResult step();
            /** @brief Stops motors and camera, resets state, and waits 100 milliseconds. */
            void finish();
            /** @brief Reports whether camera acquisition is active. */
            agent::boolean started() const noexcept;
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_BULL_FIGHT_H */
