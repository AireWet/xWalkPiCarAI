/******************************************************************************
 * @file        xHal_Rpi5CarSpi.h
 * @brief       Declares the hardware-independent xWalk SPI interface.
 *
 * @details
 * Validates bounded full-duplex transfers and forwards them synchronously to
 * one caller-owned SPI backend without opening a platform device.
 *
 * @project     xWalk Firmware
 * @module      xWalkSpi
 *
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_SPI_H
#define XHAL_RPI5CAR_SPI_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarSpiTypes.h"

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
     * @class XWalkSpi
     * @brief Provides bounded backend-neutral full-duplex SPI transactions.
     *
     * @details
     * Stores a non-owning callback context and one required synchronous transfer
     * callback. The backend and context must outlive this object. Operations emit
     * filtered trace records and unfiltered failure diagnostics.
     */
    class XWalkSpi final
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Nullable non-owning backend context. */
            contextpointer contextValue{nullptr};
            /** @brief Required synchronous full-duplex transfer operation. */
            spitransfercallback transferCallback{nullptr};

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Binds the interface to one caller-owned SPI backend.
             * @param[in,out] context Nullable context that outlives this object.
             * @param[in] transferOperation Non-null synchronous transfer callback.
             * @throws std::invalid_argument If `transferOperation` is null.
             */
            XWalkSpi(contextpointer context, spitransfercallback transferOperation);

            /** @brief Releases no caller-owned backend resource. */
            ~XWalkSpi();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables copying of the callback binding. */
            XWalkSpi(const XWalkSpi&) = delete;
            /** @brief Disables copy assignment of the callback binding. */
            XWalkSpi& operator=(const XWalkSpi&) = delete;
            /** @brief Disables move construction to preserve callback context identity. */
            XWalkSpi(XWalkSpi&&) = delete;
            /** @brief Disables move assignment to preserve callback context identity. */
            XWalkSpi& operator=(XWalkSpi&&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Performs one bounded full-duplex transaction.
             * @param[in] transmitData Non-empty payload containing at most 256 bytes.
             * @return Received bytes with exactly the transmitted length.
             * @throws std::invalid_argument If the payload is empty.
             * @throws std::out_of_range If the payload exceeds 256 bytes.
             * @throws std::runtime_error If the backend returns an unexpected length.
             */
            bytevector transfer(const bytevector& transmitData);
    };

} /* namespace xwalk::hal */

/******************************************************************************
 * Function-like macros
 ******************************************************************************/

/**
 * @brief Creates a full-duplex callback for one SPI backend type.
 * @warning The context must point to a live `BACKEND_TYPE` object.
 */
#define XHAL_SPI_TRANSFER_CALLBACK(BACKEND_TYPE)                                                                       \
    +[](xwalk::hal::contextpointer xwalkCallbackContext,                                                               \
        const xwalk::hal::bytevector& xwalkCallbackData) -> xwalk::hal::bytevector                                     \
    {                                                                                                                  \
        return static_cast<BACKEND_TYPE*>(xwalkCallbackContext)->transfer(xwalkCallbackData);                          \
    }

#endif /* XHAL_RPI5CAR_SPI_H */
