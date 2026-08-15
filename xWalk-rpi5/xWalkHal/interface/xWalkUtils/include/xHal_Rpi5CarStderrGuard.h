/******************************************************************************
 * @file        xHal_Rpi5CarStderrGuard.h
 * @brief       Declares a scope-bound standard-error suppression guard.
 *
 * @details
 * Ports the Python `ignore_stderr` context manager through injected redirect
 * and restore operations without embedding platform descriptor calls.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils
 *
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_STDERR_GUARD_H
#define XHAL_RPI5CAR_STDERR_GUARD_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarUtilsTypes.h"

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkStderrGuard
     * @brief Restores standard error automatically at the end of one scope.
     *
     * @details
     * Stores a non-owning callback context, a restore callback, and an opaque
     * backend token. Construction begins suppression and destruction restores it.
     */
    class XWalkStderrGuard final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Nullable non-owning context used by both redirect callbacks.
             *
             * @note
             * A non-null object must outlive the guard. Null requires stateless callbacks.
             */
            contextpointer backendContextPointer;

            /** @brief Non-null synchronous restore callback copied during construction. */
            utilityrestorecallback restoreCallback;

            /** @brief Backend-defined token identifying the original standard-error state. */
            int32 restoreToken;

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates both operations before beginning suppression.
             *
             * @param[in] redirect
             * Callback that must be non-null.
             *
             * @param[in] restore
             * Callback that must be non-null.
             *
             * @throws std::invalid_argument
             * If either callback is null.
             */
            static void validateCallbacks(utilityredirectcallback redirect, utilityrestorecallback restore);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Begins standard-error suppression through an injected backend.
             *
             * @param[in,out] backendContext
             * Nullable non-owning context used by both callbacks.
             *
             * @param[in] redirect
             * Non-null callback that begins suppression and returns a restore token.
             *
             * @param[in] restore
             * Non-null callback used during destruction.
             *
             * @throws std::invalid_argument
             * If either callback is null.
             */
            XWalkStderrGuard(contextpointer backendContext,
                             utilityredirectcallback redirect,
                             utilityrestorecallback restore);

            /**
             * @brief Restores standard error without releasing backend ownership.
             *
             * @note
             * The restore callback must not throw because the destructor cannot report failures safely.
             */
            ~XWalkStderrGuard();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying of the active suppression token. */
            XWalkStderrGuard(const XWalkStderrGuard&) = delete;
            /** @brief Disables copy assignment of the active suppression token. */
            XWalkStderrGuard& operator=(const XWalkStderrGuard&) = delete;
            /** @brief Disables moving because callback context identity is retained. */
            XWalkStderrGuard(XWalkStderrGuard&&) = delete;
            /** @brief Disables move assignment because callback context identity is retained. */
            XWalkStderrGuard& operator=(XWalkStderrGuard&&) = delete;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_STDERR_GUARD_H */
