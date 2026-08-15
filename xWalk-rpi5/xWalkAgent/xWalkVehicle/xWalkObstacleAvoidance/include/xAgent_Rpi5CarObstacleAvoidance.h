/******************************************************************************
 * @file        xAgent_Rpi5CarObstacleAvoidance.h
 * @brief       Declares bounded ultrasonic obstacle avoidance.
 *
 * @details
 * Ports the distance bands and vehicle actions from upstream
 * `example/4.avoiding_obstacles.py` while adding cancellation and safe handling
 * for failed ultrasonic samples.
 *
 * @project     xWalk Firmware
 * @module      xWalkObstacleAvoidance
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_OBSTACLE_AVOIDANCE_H
#define XAGENT_RPI5CAR_OBSTACLE_AVOIDANCE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarObstacleAvoidanceTypes.h"
#include "xAgent_Rpi5CarPicarx.h"

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
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkObstacleAvoidance
     * @brief Applies one bounded obstacle-avoidance decision to a PiCar-X vehicle.
     *
     * @details
     * Observes a caller-owned PiCar-X coordinator and scheduling callbacks. The
     * caller owns ultrasonic acquisition and repeated foreground scheduling.
     */
    class XWalkObstacleAvoidance final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning PiCar-X pointer that remains non-null for this lifetime. */
            XWalkPicarx* picarxObject{nullptr};
            /** @brief Nullable non-owning context forwarded synchronously to callbacks. */
            agent::contextpointer callbackContext{nullptr};
            /** @brief Non-null synchronous timing callback. */
            obstacleavoidancedelaycallback delayCallback{nullptr};
            /** @brief Non-null synchronous cancellation callback. */
            obstacleavoidancecontinuecallback continueCallback{nullptr};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Waits in cancellable slices no longer than 20 milliseconds. */
            agent::boolean wait(agent::uint32 durationMs) const;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Binds caller-owned vehicle and synchronous scheduling operations.
             * @param[in] picarx PiCar-X coordinator that must outlive this Agent.
             * @param[in,out] context Optional callback context that must outlive this Agent.
             * @param[in] delayOperation Non-null synchronous delay operation.
             * @param[in] continueOperation Non-null synchronous cancellation query.
             * @throws std::invalid_argument If either callback is null.
             */
            XWalkObstacleAvoidance(XWalkPicarx& picarx,
                                   agent::contextpointer context,
                                   obstacleavoidancedelaycallback delayOperation,
                                   obstacleavoidancecontinuecallback continueOperation);

            /** @brief Performs a non-throwing emergency motor stop without releasing dependencies. */
            ~XWalkObstacleAvoidance() noexcept;

            XWalkObstacleAvoidance(const XWalkObstacleAvoidance&) = delete;
            XWalkObstacleAvoidance(XWalkObstacleAvoidance&&) = delete;
            XWalkObstacleAvoidance& operator=(const XWalkObstacleAvoidance&) = delete;
            XWalkObstacleAvoidance& operator=(XWalkObstacleAvoidance&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Applies one source-compatible decision for a measured distance.
             * @param[in] distanceCm Ultrasonic distance in centimeters; non-positive values are invalid.
             * @return The applied movement band, sensor failure, or cancellation result.
             * @warning Successful decisions may move the physical vehicle at 50-percent requested speed.
             */
            XWalkObstacleAvoidanceResult step(agent::float64 distanceCm);

            /** @brief Stops both drive motors without changing steering. */
            void stop();
    };

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_OBSTACLE_AVOIDANCE_H */
