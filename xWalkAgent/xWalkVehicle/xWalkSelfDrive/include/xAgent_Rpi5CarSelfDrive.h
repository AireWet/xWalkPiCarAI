/******************************************************************************
 * @file        xAgent_Rpi5CarSelfDrive.h
 * @brief       Declares the PiCar-X preset-action coordinator.
 *
 * @details
 * Coordinates caller-owned PiCar-X and music objects through synchronous
 * gestures, sound actions, and an optional background action queue.
 *
 * @project     xWalk Firmware
 * @module      xWalkSelfDrive
 *
 * @author      Joxy John
 * @date        2026-07-31
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_SELF_DRIVE_H
#define XAGENT_RPI5CAR_SELF_DRIVE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarSelfDriveTypes.h"

#include "xHal_Rpi5CarMusic.h"

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
 * @class XWalkSelfDrive
 * @brief Runs the named actions defined by the upstream preset-actions module.
 *
 * @details
 * Stores non-owning pointers to one PiCar-X coordinator and one music
 * controller. The optional worker owns only its thread and queued action names.
 * Call direct action and queue-control methods from one controlling context.
 */
class XWalkSelfDrive
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning PiCar-X pointer that is never null after construction. */
        XWalkPicarx* picarxObject{nullptr};
        /** @brief Non-owning music pointer that is never null after construction. */
        hal::XWalkMusic* musicObject{nullptr};
        /** @brief Sound directory copied from deployment configuration or a host-test override. */
        agent::string soundDirectoryValue{"../xWalkAudioResources/sounds"};
        /** @brief Nullable non-owning context passed to the delay callback. */
        agent::contextpointer callbackContext{nullptr};
        /** @brief Non-null synchronous delay operation copied from the caller. */
        selfdrivedelaycallback delayCallback{nullptr};
        /** @brief Non-null cancellation query copied from the caller. */
        selfdrivecontinuecallback continueCallback{nullptr};
        /** @brief Protects status, prior status, and the queued action names. */
        mutable agent::mutexhandle stateMutex{};
        /** @brief Wakes callers waiting for the action queue to return to standby. */
        agent::conditionvariable stateChanged{};
        /** @brief First-in, first-out sequence of validated action names. */
        agent::stringvector actionQueue{};
        /** @brief Joinable background action worker owned by this coordinator. */
        agent::threadhandle worker{};
        /** @brief Atomically requests worker execution or shutdown. */
        agent::atomicboolean runningValue{false};
        /** @brief Current worker state guarded by `stateMutex`. */
        XWalkSelfDriveStatus statusValue{XWalkSelfDriveStatus::Standby};
        /** @brief Last state observed by the worker, guarded by `stateMutex`. */
        XWalkSelfDriveStatus lastStatusValue{XWalkSelfDriveStatus::Standby};
        /** @brief Indicates whether the worker has observed any state. */
        agent::boolean hasLastStatus{};
        /** @brief Records an explicit callback failure without exception handling. */
        agent::atomicboolean operationFailedValue{false};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /** @brief Rejects a null delay callback before storing dependencies. */
        static void validateDelayCallback(selfdrivedelaycallback delayOperation);
        /** @brief Returns whether text names one supported movement, stop, or sound action. */
        static agent::boolean isActionSupported(agent::stringview action) noexcept;
        /** @brief Invokes the application-owned delay operation and records its status. */
        agent::boolean delay(agent::uint32 durationMs);
        /** @brief Runs the background status and action-queue loop. */
        void actionLoop() noexcept;
        /** @brief Executes the thinking pose without restoring the centered pose. */
        void keepThink();
        /** @brief Drives forward briefly and then stops. */
        void forward();
        /** @brief Drives backward briefly and then stops. */
        void backward();
        /** @brief Alternates the steering servo to imitate waving hands. */
        void waveHands();
        /** @brief Alternates steering and camera pan to imitate resistance. */
        void resist();
        /** @brief Alternates low-speed drive directions to imitate cute shaking. */
        void actCute();
        /** @brief Alternates small steering angles to imitate rubbing hands. */
        void rubHands();
        /** @brief Runs the thinking pose and returns to the centered pose. */
        void think();
        /** @brief Oscillates drive, steering, and camera pan together. */
        void twistBody();
        /** @brief Runs the mirrored steering and camera celebration sequence. */
        void celebrate();
        /** @brief Runs the downward camera-tilt sequence and resets the car. */
        void depressed();
        /** @brief Runs the decreasing camera-pan head-shake sequence. */
        void shakeHead();
        /** @brief Runs the repeated camera-tilt nod sequence. */
        void nod();
        /** @brief Starts the upstream horn sound asynchronously. */
        agent::boolean honking();
        /** @brief Starts the upstream engine sound asynchronously. */
        agent::boolean startEngine();

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a preset-action coordinator around caller-owned objects.
         *
         * @param[in] picarx
         * PiCar-X coordinator that must outlive this object and any worker.
         *
         * @param[in] music
         * Music controller that must outlive this object and any worker.
         *
         * @param[in,out] context
         * Optional callback context that must outlive this object and any worker.
         *
         * @param[in] callback
         * Non-null delay operation. Worker use must be non-throwing.
         *
         * @param[in] continueOperation
         * Optional cancellation query. Worker use must be non-throwing when supplied.
         *
         * @param[in] soundDirectory
         * Sound-resource directory. Deployment supplies an absolute path; tests may override it.
         *
         * @throws std::invalid_argument
         * If `callback` is null.
         */
        XWalkSelfDrive(XWalkPicarx& picarx, hal::XWalkMusic& music,
            agent::contextpointer context, selfdrivedelaycallback callback,
            selfdrivecontinuecallback continueOperation = nullptr,
            agent::stringview soundDirectory = "../xWalkAudioResources/sounds");

        /**
         * @brief Stops and joins the worker without releasing injected objects.
         *
         * @note
         * Callback failures are reported through `waitActionsDone()`.
         */
        ~XWalkSelfDrive();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction because the worker retains object identity. */
        XWalkSelfDrive(XWalkSelfDrive&&) = delete;
        /** @brief Disables copying of worker and dependency state. */
        XWalkSelfDrive(const XWalkSelfDrive&) = delete;
        /** @brief Disables move assignment because the worker retains object identity. */
        XWalkSelfDrive& operator=(XWalkSelfDrive&&) = delete;
        /** @brief Disables copying of worker and dependency state. */
        XWalkSelfDrive& operator=(const XWalkSelfDrive&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Executes one supported preset action synchronously.
         *
         * @param[in] action
         * Exact lowercase preset action name, including the serialized `stop` action.
         *
         * @return
         * `true` when the action was recognized and completed; otherwise `false`.
         */
        agent::boolean doAction(agent::stringview action);

        /**
         * @brief Replaces the optional application cancellation query.
         * @param[in,out] context Optional non-owning context that must outlive later action execution.
         * @param[in] continueOperation Optional non-throwing query; null disables cancellation checks.
         */
        void setCancellation(agent::contextpointer context,
            selfdrivecontinuecallback continueOperation) noexcept;

        /**
         * @brief Adds one supported action to the background first-in, first-out queue.
         *
         * @param[in] action
         * Exact lowercase action name to copy into the queue.
         *
         * @return
         * `true` when the action was queued; otherwise `false` for an unknown name.
         */
        agent::boolean addAction(agent::stringview action);

        /**
         * @brief Starts a fresh background worker and clears previously queued actions.
         *
         * @throws std::logic_error
         * If the worker is already running.
         */
        void start();
        /**
         * @brief Requests worker shutdown and joins it when present.
         *
         * @post
         * `running()` returns `false` and no worker remains joinable.
         */
        void stop();

        /**
         * @brief Selects the worker status used on its next iteration.
         * @param[in] status New worker state.
         */
        void setStatus(XWalkSelfDriveStatus status);

        /**
         * @brief Waits until queued actions reach standby or the worker stops.
         *
         * @return `true` when queued work completed; otherwise `false` after emergency shutdown.
         */
        agent::boolean waitActionsDone();
        /**
         * @brief Returns the current worker status under synchronization.
         *
         * @return
         * Current action-flow state.
         */
        XWalkSelfDriveStatus status() const;
        /**
         * @brief Returns whether the background worker has been requested to run.
         *
         * @return
         * `true` between successful `start()` and completion of `stop()`.
         */
        agent::boolean running() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_SELF_DRIVE_H */
