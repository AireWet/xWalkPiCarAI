/******************************************************************************
 * @file        xHal_Rpi5CarGpioLifecycle.cpp
 * @brief       Implements GPIO validation, pin mapping, and lifecycle behavior.
 *
 * @details
 * Validates callback bindings and Robot HAT pins, performs initial
 *configuration, and cancels handlers.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarGpio.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /******************************************************************************
     * Protected member function definitions
     ******************************************************************************/

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
    uint8 XWalkGpio::resolvePin(stringview pinName)
    {
        if (pinName == "D0")
        {
            return 17U;
        }
        if ((pinName == "D1") || (pinName == "D7"))
        {
            return 4U;
        }
        if (pinName == "D2")
        {
            return 27U;
        }
        if (pinName == "D3")
        {
            return 22U;
        }
        if (pinName == "D4")
        {
            return 23U;
        }
        if (pinName == "D5")
        {
            return 24U;
        }
        if ((pinName == "D6") || (pinName == "SW") || (pinName == "USER"))
        {
            return 25U;
        }
        if ((pinName == "D8") || (pinName == "MCURST"))
        {
            return 5U;
        }
        if (pinName == "D9")
        {
            return 6U;
        }
        if ((pinName == "D10") || (pinName == "BOARD_TYPE"))
        {
            return 12U;
        }
        if ((pinName == "D11") || (pinName == "BLEINT"))
        {
            return 13U;
        }
        if (pinName == "D12")
        {
            return 19U;
        }
        if ((pinName == "D13") || (pinName == "RST"))
        {
            return 16U;
        }
        if ((pinName == "D14") || (pinName == "LED"))
        {
            return 26U;
        }
        if ((pinName == "D15") || (pinName == "BLERST"))
        {
            return 20U;
        }
        if (pinName == "D16")
        {
            return 21U;
        }
        if (pinName == "CE")
        {
            return 8U;
        }
        XWALK_HAL_ERROR(XWALK_INVAL, "GPIO pin name is not supported");
    }

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
    uint8 XWalkGpio::validatePin(uint32 pin)
    {
        const boolean supported = (pin == 4U) || (pin == 5U) || (pin == 6U) || (pin == 8U) || (pin == 12U) ||
                                  (pin == 13U) || (pin == 16U) || (pin == 17U) || (pin == 19U) || (pin == 20U) ||
                                  (pin == 21U) || (pin == 22U) || (pin == 23U) || (pin == 24U) || (pin == 25U) ||
                                  (pin == 26U) || (pin == 27U);
        if (supported == false)
        {
            XWALK_HAL_ERROR(XWALK_RANGE, "GPIO pin is not present in the Robot HAT map");
        }
        return static_cast<uint8>(pin);
    }

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
    XWalkGpioCallbacks XWalkGpio::validateCallbacks(const XWalkGpioCallbacks& callbacks)
    {
        if ((callbacks.configure == nullptr) || (callbacks.read == nullptr) || (callbacks.write == nullptr) ||
            (callbacks.interrupt == nullptr) || (callbacks.cancelInterrupt == nullptr))
        {
            XWALK_HAL_ERROR(XWALK_INVAL, "GPIO callbacks must not be null");
        }
        return callbacks;
    }

    /**
     * @brief Converts a logical software level to its physical pin level.
     *
     * @param[in] logicalValue
     * Software-visible digital level.
     *
     * @return
     * Physical level after applying the configured polarity.
     */
    boolean XWalkGpio::physicalValue(boolean logicalValue) const noexcept
    {
        return activeHighValue ? logicalValue : !logicalValue;
    }

    /******************************************************************************
     * Constructor definitions
     ******************************************************************************/

    /**
     * @brief Constructs and configures a numeric Robot HAT GPIO line.
     *
     * @param[in] context
     * Non-owning backend context that must satisfy all callbacks.
     *
     * @param[in] callbacks
     * Non-null backend operations that must remain valid for this object's
     * lifetime.
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
     * `true` for normal logical polarity or `false` to invert logical reads and
     * writes.
     *
     * @throws std::invalid_argument
     * If any callback is null.
     *
     * @throws std::out_of_range
     * If `pin` is not present in the Robot HAT pin dictionary.
     */
    XWalkGpio::XWalkGpio(contextpointer context,
                         const XWalkGpioCallbacks& callbacks,
                         uint32 pin,
                         XWalkGpioMode mode,
                         XWalkGpioPull pull,
                         boolean activeHigh)
        : contextValue(context), callbacksValue(validateCallbacks(callbacks)), pinValue(validatePin(pin)),
          modeValue(mode), pullValue(pull), activeHighValue(activeHigh)
    {
        callbacksValue.configure(contextValue, pinValue, modeValue, pullValue, physicalValue(outputValue));
        XWALK_HAL_TRACE_UID3(RPI .067,
                             "GPIO callback interface constructed for line %u with "
                             "mode %u and active-high %u",
                             static_cast<uint32>(pinValue),
                             static_cast<uint32>(modeValue),
                             static_cast<uint32>(activeHighValue));
    }

    /**
     * @brief Constructs and configures a named Robot HAT GPIO line.
     *
     * @param[in] context
     * Non-owning backend context that must satisfy all callbacks.
     *
     * @param[in] callbacks
     * Non-null backend operations that must remain valid for this object's
     * lifetime.
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
     * `true` for normal logical polarity or `false` to invert logical reads and
     * writes.
     *
     * @throws std::invalid_argument
     * If any callback is null or `pinName` is not supported.
     */
    XWalkGpio::XWalkGpio(contextpointer context,
                         const XWalkGpioCallbacks& callbacks,
                         stringview pinName,
                         XWalkGpioMode mode,
                         XWalkGpioPull pull,
                         boolean activeHigh)
        : XWalkGpio(context, callbacks, static_cast<uint32>(resolvePin(pinName)), mode, pull, activeHigh)
    {
    }

    /******************************************************************************
     * Destructor definitions
     ******************************************************************************/

    /**
     * @brief Destroys the GPIO object after cancelling its interrupt registration.
     *
     * @note
     * The backend context is non-owning and is not released.
     */
    XWalkGpio::~XWalkGpio()
    {
        close();
    }

} /* namespace xwalk::hal */
