/******************************************************************************
 * @file        xHal_Rpi5CarGpio.h
 * @brief       Declares the hardware-independent xWalk GPIO pin interface.
 *
 * @details
 * Defines GPIO modes, pulls, interrupt edges, callback bindings, named Robot HAT pin mapping, and
 * digital pin operations without coupling consumers to a platform-specific GPIO implementation.
 *
 * @project     xWalk Firmware
 * @module      xWalkGpio
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

#ifndef XHAL_RPI5CAR_GPIO_H
#define XHAL_RPI5CAR_GPIO_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommon.h"

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
     * Enumeration declarations
     ******************************************************************************/

    /** @brief Identifies whether a GPIO line receives or drives a digital signal. */
    enum class XWalkGpioMode : uint8
    {
        Output = 0U, /**< Drives the configured digital output level. */
        Input = 1U   /**< Samples the external digital input level. */
    };

    /** @brief Selects the internal bias applied while a GPIO line is claimed. */
    enum class XWalkGpioPull : uint8
    {
        None = 0U, /**< Disables the internal bias. */
        Up = 1U,   /**< Enables the internal pull-up bias. */
        Down = 2U  /**< Enables the internal pull-down bias. */
    };

    /** @brief Selects the signal transition that invokes a GPIO interrupt handler. */
    enum class XWalkGpioEdge : uint8
    {
        Falling = 0U, /**< High-to-low signal transition. */
        Rising = 1U,  /**< Low-to-high signal transition. */
        Both = 2U     /**< Either rising or falling signal transition. */
    };

    /******************************************************************************
     * Type definitions
     ******************************************************************************/

    /**
     * @brief Application handler invoked after a debounced GPIO edge.
     *
     * @param[in,out] context
     * Non-owning application context supplied when the interrupt is configured.
     *
     * @warning
     * A hardware backend may invoke this handler from a worker thread or interrupt-oriented context.
     */
    using gpiointerrupthandler = void (*)(contextpointer context);

    /**
     * @brief Callback used to claim and configure one GPIO line.
     *
     * @param[in,out] context
     * Non-owning backend context supplied during GPIO construction.
     *
     * @param[in] pin
     * Linux GPIO line offset in the range 0 through 255.
     *
     * @param[in] mode
     * Requested digital input or output mode.
     *
     * @param[in] pull
     * Requested internal pull configuration.
     *
     * @param[in] initialValue
     * Initial logical output level; ignored by an input configuration.
     */
    using gpioconfigurecallback =
        void (*)(contextpointer context, uint8 pin, XWalkGpioMode mode, XWalkGpioPull pull, boolean initialValue);

    /**
     * @brief Callback used to sample one GPIO line.
     *
     * @param[in,out] context
     * Non-owning backend context supplied during GPIO construction.
     *
     * @param[in] pin
     * Linux GPIO line offset in the range 0 through 255.
     *
     * @return
     * `true` for a physical high level; otherwise `false`.
     */
    using gpioreadcallback = boolean (*)(contextpointer context, uint8 pin);

    /**
     * @brief Callback used to drive one GPIO line.
     *
     * @param[in,out] context
     * Non-owning backend context supplied during GPIO construction.
     *
     * @param[in] pin
     * Linux GPIO line offset in the range 0 through 255.
     *
     * @param[in] value
     * Physical output level to drive.
     */
    using gpiowritecallback = void (*)(contextpointer context, uint8 pin, boolean value);

    /**
     * @brief Callback used to register a debounced GPIO edge handler.
     *
     * @param[in,out] context
     * Non-owning backend context supplied during GPIO construction.
     *
     * @param[in] pin
     * Linux GPIO line offset in the range 0 through 255.
     *
     * @param[in] edge
     * Signal transition that triggers the handler.
     *
     * @param[in] debounceMs
     * Minimum interval between accepted events in milliseconds.
     *
     * @param[in,out] handlerContext
     * Non-owning application context forwarded to `handler`.
     *
     * @param[in] handler
     * Non-null function invoked for accepted events.
     */
    using gpiointerruptcallback = void (*)(contextpointer context,
                                           uint8 pin,
                                           XWalkGpioEdge edge,
                                           uint32 debounceMs,
                                           contextpointer handlerContext,
                                           gpiointerrupthandler handler);

    /**
     * @brief Callback used to cancel the active GPIO interrupt handler.
     *
     * @param[in,out] context
     * Non-owning backend context supplied during GPIO construction.
     *
     * @param[in] pin
     * Linux GPIO line offset whose handler is cancelled.
     */
    using gpiocancelinterruptcallback = void (*)(contextpointer context, uint8 pin);

    /******************************************************************************
     * Structure declarations
     ******************************************************************************/

    /**
     * @brief Groups the non-owning backend callbacks required by one GPIO object.
     *
     * @details
     * Every callback must remain callable until the GPIO object is destroyed.
     */
    struct XWalkGpioCallbacks
    {
            gpioconfigurecallback configure{nullptr};             /**< Claims and configures the line. */
            gpioreadcallback read{nullptr};                       /**< Samples the physical line level. */
            gpiowritecallback write{nullptr};                     /**< Drives the physical line level. */
            gpiointerruptcallback interrupt{nullptr};             /**< Registers an edge handler. */
            gpiocancelinterruptcallback cancelInterrupt{nullptr}; /**< Cancels the edge handler. */
    };

    /******************************************************************************
     * Class declarations
     ******************************************************************************/

    /**
     * @class XWalkGpio
     * @brief Controls one named or numeric Robot HAT GPIO line.
     *
     * @details
     * Stores a non-owning backend context and callback bindings, automatically changes direction for reads and
     * writes as the Python implementation does, applies logical polarity, manages an optional edge handler,
     * and emits filtered lifecycle and operation traces plus unfiltered validation diagnostics.
     */
    class XWalkGpio
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning opaque pointer forwarded to all backend callbacks.
             *
             * @note
             * Null is permitted only when every callback supports a null context.
             */
            contextpointer contextValue{nullptr};

            /** @brief Validated callback bindings whose targets must outlive this object. */
            XWalkGpioCallbacks callbacksValue{};

            /** @brief Linux GPIO line offset selected from the Robot HAT pin map. */
            uint8 pinValue{};

            /** @brief Current direction requested from the backend. */
            XWalkGpioMode modeValue{XWalkGpioMode::Output};

            /** @brief Current internal bias requested from the backend. */
            XWalkGpioPull pullValue{XWalkGpioPull::None};

            /** @brief Logical output level most recently requested by the caller. */
            boolean outputValue{false};

            /** @brief `true` when logical and physical levels have the same polarity. */
            boolean activeHighValue{true};

            /** @brief `true` while this object has a registered interrupt handler. */
            boolean interruptActive{false};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Resolves a Robot HAT board name to a Linux GPIO line offset.
             *
             * @param[in] pinName
             * Case-sensitive name from the Robot HAT pin dictionary.
             *
             * @return
             * Mapped GPIO line offset.
             *
             * @throws std::invalid_argument
             * If the name is not present in the supported pin dictionary.
             */
            static uint8 resolvePin(stringview pinName);

            /**
             * @brief Validates a numeric GPIO line against the Robot HAT pin dictionary.
             *
             * @param[in] pin
             * Candidate Linux GPIO line offset.
             *
             * @return
             * Validated line offset.
             *
             * @throws std::out_of_range
             * If the value is not used by a supported Robot HAT pin name.
             */
            static uint8 validatePin(uint32 pin);

            /**
             * @brief Validates that every required backend callback is non-null.
             *
             * @param[in] callbacks
             * Callback set to validate.
             *
             * @return
             * Validated callback set.
             *
             * @throws std::invalid_argument
             * If any callback is null.
             */
            static XWalkGpioCallbacks validateCallbacks(const XWalkGpioCallbacks& callbacks);

            /**
             * @brief Converts a logical software level to its physical pin level.
             *
             * @param[in] logicalValue
             * Software-visible digital level.
             *
             * @return
             * Physical level after applying the configured polarity.
             */
            boolean physicalValue(boolean logicalValue) const noexcept;

        public:
            /**************************************************************************
             * Public static functions
             **************************************************************************/

            /**
             * @brief Tries to resolve a Robot HAT board name to a Linux GPIO line offset.
             *
             * @param[in] pinName
             * Case-sensitive name from the Robot HAT pin dictionary.
             *
             * @param[out] pin
             * Mapped GPIO line offset when the name is supported; unchanged otherwise.
             *
             * @return
             * `true` when `pinName` was resolved; otherwise `false`.
             */
            static boolean tryResolvePin(stringview pinName, uint8& pin) noexcept;

            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs and configures a numeric Robot HAT GPIO line.
             *
             * @param[in] context
             * Non-owning backend context that must satisfy all callbacks.
             *
             * @param[in] callbacks
             * Non-null backend operations that must remain valid for this object's lifetime.
             *
             * @param[in] pin
             * Numeric GPIO line used by at least one Robot HAT pin name.
             *
             * @param[in] mode
             * Initial input or output direction.
             *
             * @param[in] pull
             * Initial internal bias configuration.
             *
             * @param[in] activeHigh
             * `true` for normal logical polarity or `false` to invert logical reads and writes.
             *
             * @throws std::invalid_argument
             * If any callback is null.
             *
             * @throws std::out_of_range
             * If `pin` is not present in the Robot HAT pin dictionary.
             */
            XWalkGpio(contextpointer context,
                      const XWalkGpioCallbacks& callbacks,
                      uint32 pin,
                      XWalkGpioMode mode = XWalkGpioMode::Output,
                      XWalkGpioPull pull = XWalkGpioPull::None,
                      boolean activeHigh = true);

            /**
             * @brief Constructs and configures a named Robot HAT GPIO line.
             *
             * @param[in] context
             * Non-owning backend context that must satisfy all callbacks.
             *
             * @param[in] callbacks
             * Non-null backend operations that must remain valid for this object's lifetime.
             *
             * @param[in] pinName
             * Case-sensitive name from the Robot HAT pin dictionary.
             *
             * @param[in] mode
             * Initial input or output direction.
             *
             * @param[in] pull
             * Initial internal bias configuration.
             *
             * @param[in] activeHigh
             * `true` for normal logical polarity or `false` to invert logical reads and writes.
             *
             * @throws std::invalid_argument
             * If any callback is null or `pinName` is not supported.
             */
            XWalkGpio(contextpointer context,
                      const XWalkGpioCallbacks& callbacks,
                      stringview pinName,
                      XWalkGpioMode mode = XWalkGpioMode::Output,
                      XWalkGpioPull pull = XWalkGpioPull::None,
                      boolean activeHigh = true);

            /** @brief Cancels this object's interrupt registration without owning its backend. */
            ~XWalkGpio();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction because callbacks may retain this object's identity. */
            XWalkGpio(XWalkGpio&&) = delete;
            /** @brief Disables copying of the GPIO callback binding. */
            XWalkGpio(const XWalkGpio&) = delete;
            /** @brief Disables move assignment of the GPIO callback binding. */
            XWalkGpio& operator=(XWalkGpio&&) = delete;
            /** @brief Disables copy assignment of the GPIO callback binding. */
            XWalkGpio& operator=(const XWalkGpio&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Reconfigures the line direction and internal bias.
             *
             * @param[in] mode
             * Requested input or output direction.
             *
             * @param[in] pull
             * Requested internal bias configuration.
             */
            void setup(XWalkGpioMode mode, XWalkGpioPull pull = XWalkGpioPull::None);

            /**
             * @brief Reads the logical GPIO level.
             *
             * @return
             * Logical level after applying the configured polarity.
             *
             * @post
             * The pin is configured as an input.
             */
            boolean read();

            /**
             * @brief Drives the logical GPIO level.
             *
             * @param[in] value
             * Logical level to drive after polarity conversion.
             *
             * @return
             * The requested logical level.
             *
             * @post
             * The pin is configured as an output.
             */
            boolean write(boolean value);

            /**
             * @brief Drives the logical active level.
             *
             * @return
             * Always `true` after a successful write.
             */
            boolean on();

            /**
             * @brief Drives the logical inactive level.
             *
             * @return
             * Always `false` after a successful write.
             */
            boolean off();

            /**
             * @brief Drives the logical high level.
             *
             * @return
             * Always `true` after a successful write.
             */
            boolean high();

            /**
             * @brief Drives the logical low level.
             *
             * @return
             * Always `false` after a successful write.
             */
            boolean low();

            /**
             * @brief Registers a debounced GPIO edge handler.
             *
             * @param[in,out] handlerContext
             * Non-owning application context forwarded to `handler`.
             *
             * @param[in] handler
             * Non-null application handler.
             *
             * @param[in] edge
             * Signal transition that triggers the handler.
             *
             * @param[in] debounceMs
             * Minimum interval between accepted events in milliseconds.
             *
             * @param[in] pull
             * Internal bias applied while the interrupt input is claimed.
             *
             * @throws std::invalid_argument
             * If `handler` is null.
             *
             * @warning
             * The handler context must outlive the registration, and the handler must not throw.
             */
            void irq(contextpointer handlerContext,
                     gpiointerrupthandler handler,
                     XWalkGpioEdge edge,
                     uint32 debounceMs = XHAL_RPI5CAR_GPIO_DEFAULT_DEBOUNCE_MS,
                     XWalkGpioPull pull = XWalkGpioPull::None);

            /**
             * @brief Cancels the active interrupt handler when one is registered.
             *
             * @post
             * No interrupt registration remains associated with this object.
             */
            void close();

            /**
             * @brief Provides the Python-compatible alias for `close()`.
             *
             * @post
             * No interrupt registration remains associated with this object.
             */
            void deinit();

            /**
             * @brief Returns the Linux GPIO line offset.
             *
             * @return
             * Validated line offset from the Robot HAT pin dictionary.
             */
            uint8 pin() const noexcept;

            /**
             * @brief Returns the current input or output mode.
             *
             * @return
             * Most recently configured mode.
             */
            XWalkGpioMode mode() const noexcept;

            /**
             * @brief Returns the current internal pull configuration.
             *
             * @return
             * Most recently configured pull setting.
             */
            XWalkGpioPull pull() const noexcept;

            /**
             * @brief Returns an owned Linux-style name for the GPIO line.
             *
             * @return
             * Name in the form `GPIO<number>`.
             */
            string name() const;
    };

} /* namespace xwalk::hal */

/******************************************************************************
 * Function-like macros
 ******************************************************************************/

/**
 * @brief Creates the complete GPIO callback set for a backend type.
 *
 * @warning
 * The context passed to every callback must point to a live `BACKEND_TYPE` object.
 */
#define XHAL_GPIO_CALLBACKS(BACKEND_TYPE)                                                                              \
    xwalk::hal::XWalkGpioCallbacks                                                                                     \
    {                                                                                                                  \
        +[](xwalk::hal::contextpointer xwalkCallbackContext,                                                           \
            xwalk::hal::uint8 xwalkCallbackPin,                                                                        \
            xwalk::hal::XWalkGpioMode xwalkCallbackMode,                                                               \
            xwalk::hal::XWalkGpioPull xwalkCallbackPull,                                                               \
            xwalk::hal::boolean xwalkCallbackValue)                                                                    \
        {                                                                                                              \
            static_cast<BACKEND_TYPE*>(xwalkCallbackContext)                                                           \
                ->configurePin(xwalkCallbackPin, xwalkCallbackMode, xwalkCallbackPull, xwalkCallbackValue);            \
        },                                                                                                             \
            +[](xwalk::hal::contextpointer xwalkCallbackContext,                                                       \
                xwalk::hal::uint8 xwalkCallbackPin) -> xwalk::hal::boolean                                             \
        {                                                                                                              \
            return static_cast<BACKEND_TYPE*>(xwalkCallbackContext)->readPin(xwalkCallbackPin);                        \
        },                                                                                                             \
            +[](xwalk::hal::contextpointer xwalkCallbackContext,                                                       \
                xwalk::hal::uint8 xwalkCallbackPin,                                                                    \
                xwalk::hal::boolean xwalkCallbackValue)                                                                \
        {                                                                                                              \
            static_cast<BACKEND_TYPE*>(xwalkCallbackContext)->writePin(xwalkCallbackPin, xwalkCallbackValue);          \
        },                                                                                                             \
            +[](xwalk::hal::contextpointer xwalkCallbackContext,                                                       \
                xwalk::hal::uint8 xwalkCallbackPin,                                                                    \
                xwalk::hal::XWalkGpioEdge xwalkCallbackEdge,                                                           \
                xwalk::hal::uint32 xwalkCallbackDebounceMs,                                                            \
                xwalk::hal::contextpointer xwalkCallbackHandlerContext,                                                \
                xwalk::hal::gpiointerrupthandler xwalkCallbackHandler)                                                 \
        {                                                                                                              \
            static_cast<BACKEND_TYPE*>(xwalkCallbackContext)                                                           \
                ->registerInterrupt(xwalkCallbackPin,                                                                  \
                                    xwalkCallbackEdge,                                                                 \
                                    xwalkCallbackDebounceMs,                                                           \
                                    xwalkCallbackHandlerContext,                                                       \
                                    xwalkCallbackHandler);                                                             \
        },                                                                                                             \
            +[](xwalk::hal::contextpointer xwalkCallbackContext, xwalk::hal::uint8 xwalkCallbackPin)                   \
        {                                                                                                              \
            static_cast<BACKEND_TYPE*>(xwalkCallbackContext)->cancelInterrupt(xwalkCallbackPin);                       \
        }                                                                                                              \
    }

#endif /* XHAL_RPI5CAR_GPIO_H */
