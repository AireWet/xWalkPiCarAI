/******************************************************************************
 * @file        xHal_Rpi5CarGpioLinux.h
 * @brief       Declares the Linux GPIO character-device backend.
 *
 * @details
 * Owns a GPIO chip descriptor and one claimed line, serializes digital I/O, and dispatches debounced edge
 * events from a worker thread. One backend instance is dedicated to one `XWalkGpio` object.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio Linux Backend
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_GPIO_LINUX_H
#define XHAL_RPI5CAR_GPIO_LINUX_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarGpioDevice.h"

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
     * @class XWalkGpioLinux
     * @brief Provides Linux GPIO character-device operations for one GPIO object.
     *
     * @details
     * Opens and owns one GPIO chip descriptor. Configuration claims one line descriptor, and interrupt
     * registration replaces it with an event descriptor observed by a joinable worker thread.
     */
    class XWalkGpioLinux
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning device-operation implementation that must outlive this backend. */
            XWalkGpioDevice& deviceInterfaceValue;

            /** @brief Mutex serializing descriptor replacement and digital I/O operations. */
            mutexhandle mutex;

            /** @brief Owned Linux GPIO chip descriptor, or `-1` after closure. */
            int32 chipDescriptor{-1};

            /** @brief Owned line or event descriptor, or `-1` when no line is claimed. */
            int32 lineDescriptor{-1};

            /** @brief GPIO line offset currently associated with this backend. */
            uint8 pinValue{};

            /** @brief `true` after the first successful line configuration. */
            boolean pinAssigned{false};

            /** @brief Pull configuration reused when an interrupt event line is requested. */
            XWalkGpioPull pullValue{XWalkGpioPull::None};

            /** @brief Signals the event worker to stop within the bounded poll interval. */
            atomicboolean stopRequested{false};

            /** @brief Joinable worker that waits for Linux GPIO edge events. */
            threadhandle eventThread;

            /** @brief Non-owning application context forwarded to `interruptHandler`. */
            contextpointer interruptContext{nullptr};

            /** @brief Non-owning application handler invoked from `eventThread`; null when inactive. */
            gpiointerrupthandler interruptHandler{nullptr};

            /** @brief Minimum accepted edge interval in nanoseconds. */
            uint64 debounceNanoseconds{};

            /** @brief Kernel timestamp of the most recently accepted edge in nanoseconds. */
            uint64 lastEventNanoseconds{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Opens a Linux GPIO chip device.
             *
             * @param[in] devicePath
             * Non-null path to the GPIO character device.
             *
             * @param[in] expectedName
             * Optional exact kernel chip name verified when non-empty.
             *
             * @param[in] expectedLabel
             * Optional exact kernel chip label verified when non-empty.
             *
             * @param[in] minimumLineCount
             * Minimum number of lines required from the selected controller.
             *
             * @return
             * Owned non-negative Linux file descriptor.
             */
            int32
            openDevice(cstring devicePath, stringview expectedName, stringview expectedLabel, uint32 minimumLineCount);

            /**
             * @brief Converts a mode and pull setting to Linux GPIO handle flags.
             *
             * @param[in] mode
             * Requested input or output mode.
             *
             * @param[in] pull
             * Requested internal bias.
             *
             * @return
             * Linux GPIO handle flag bit mask.
             */
            static uint32 createHandleFlags(XWalkGpioMode mode, XWalkGpioPull pull) noexcept;

            /**
             * @brief Converts an edge selection to Linux GPIO event flags.
             *
             * @param[in] edge
             * Requested rising, falling, or combined edge selection.
             *
             * @return
             * Linux GPIO event flag bit mask.
             */
            static uint32 createEventFlags(XWalkGpioEdge edge) noexcept;

            /** @brief Closes the owned line descriptor when one is open. */
            void releaseLine() noexcept;

            /**
             * @brief Validates that the requested pin is assigned to this backend.
             *
             * @param[in] pin
             * GPIO line offset expected to match `pinValue`.
             */
            void validateAssignedPin(uint8 pin) const;

            /**
             * @brief Waits for edge events and invokes the registered application handler.
             *
             * @warning
             * The application handler executes on this worker thread and must not throw.
             */
            void interruptLoop();

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a Linux GPIO backend and opens its chip device.
             *
             * @param[in] devicePath
             * Non-null path to the GPIO character device.
             *
             * @param[in] expectedName
             * Optional exact kernel chip name verified before line claims.
             *
             * @param[in] expectedLabel
             * Optional exact kernel chip label verified before line claims.
             *
             * @param[in] minimumLineCount
             * Minimum controller line count required by the application graph.
             */
            explicit XWalkGpioLinux(cstring devicePath = XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE,
                                    stringview expectedName = {},
                                    stringview expectedLabel = {},
                                    uint32 minimumLineCount = 0U);

            /**
             * @brief Constructs the backend with an injected device-operation interface.
             * @param[in,out] deviceInterface Device boundary that must outlive this backend.
             * @param[in] devicePath Non-null, non-empty logical device path.
             * @param[in] expectedName Optional exact logical chip name.
             * @param[in] expectedLabel Optional exact logical chip label.
             * @param[in] minimumLineCount Minimum required controller line count.
             */
            XWalkGpioLinux(XWalkGpioDevice& deviceInterface,
                           cstring devicePath = XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE,
                           stringview expectedName = {},
                           stringview expectedLabel = {},
                           uint32 minimumLineCount = 0U);

            /** @brief Stops event dispatch and closes all owned Linux descriptors. */
            ~XWalkGpioLinux();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction because callbacks refer to this object. */
            XWalkGpioLinux(XWalkGpioLinux&&) = delete;
            /** @brief Disables copying of owned descriptors and worker state. */
            XWalkGpioLinux(const XWalkGpioLinux&) = delete;
            /** @brief Disables move assignment of owned descriptors and worker state. */
            XWalkGpioLinux& operator=(XWalkGpioLinux&&) = delete;
            /** @brief Disables copy assignment of owned descriptors and worker state. */
            XWalkGpioLinux& operator=(const XWalkGpioLinux&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Claims and configures one Linux GPIO line.
             *
             * @param[in] pin
             * GPIO line offset in the range 0 through 255.
             *
             * @param[in] mode
             * Requested input or output direction.
             *
             * @param[in] pull
             * Requested internal bias.
             *
             * @param[in] initialValue
             * Initial physical output level; ignored for input mode.
             */
            void configurePin(uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);

            /**
             * @brief Samples the configured physical GPIO line.
             *
             * @param[in] pin
             * GPIO line offset expected to match this backend's claimed line.
             *
             * @return
             * `true` when the physical line is high; otherwise `false`.
             */
            boolean readPin(uint8 pin);

            /**
             * @brief Drives the configured physical GPIO line.
             *
             * @param[in] pin
             * GPIO line offset expected to match this backend's claimed line.
             *
             * @param[in] value
             * Physical output level to drive.
             */
            void writePin(uint8 pin, boolean value);

            /**
             * @brief Registers a debounced Linux GPIO edge handler.
             *
             * @param[in] pin
             * GPIO line offset associated with this backend.
             *
             * @param[in] edge
             * Signal transition to observe.
             *
             * @param[in] debounceMs
             * Minimum accepted edge interval in milliseconds.
             *
             * @param[in,out] handlerContext
             * Non-owning application context forwarded to `handler`.
             *
             * @param[in] handler
             * Non-null application handler invoked from the event worker.
             */
            void registerInterrupt(uint8 pin,
                                   XWalkGpioEdge edge,
                                   uint32 debounceMs,
                                   contextpointer handlerContext,
                                   gpiointerrupthandler handler);

            /**
             * @brief Cancels event dispatch for the specified GPIO line.
             *
             * @param[in] pin
             * GPIO line offset associated with this backend.
             */
            void cancelInterrupt(uint8 pin);
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_GPIO_LINUX_H */
