/******************************************************************************
 * @file        xHal_Rpi5CarBoardControl.h
 * @brief       Declares Robot HAT board-level control operations.
 *
 * @details
 * Coordinates caller-owned GPIO and ADC objects for MCU reset, battery-voltage
 * acquisition, generic output control, and safe speaker power sequencing.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
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

#ifndef XHAL_RPI5CAR_BOARD_CONTROL_H
#define XHAL_RPI5CAR_BOARD_CONTROL_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControlTypes.h"
#include "xHal_Rpi5CarGpio.h"

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
     * @class XWalkBoardControl
     * @brief Coordinates board-level Robot HAT control signals and measurements.
     *
     * @details
     * Stores non-owning pointers to application-created MCU-reset GPIO,
     * speaker-enable GPIO, and battery ADC objects. The controller performs no
     * allocation and never owns or releases those hardware dependencies.
     *
     * @note
     * One controlling execution context must serialize calls to this object and
     * direct calls to its injected dependencies.
     */
    class XWalkBoardControl
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /**
             * @brief Non-owning pointer to the MCU-reset GPIO dependency.
             *
             * @note
             * Never null; the GPIO must outlive this controller.
             */
            XWalkGpio* mcuResetGpioPointer{nullptr};

            /**
             * @brief Non-owning pointer to the board-selected speaker-enable GPIO.
             *
             * @note
             * Never null; the GPIO must outlive this controller.
             */
            XWalkGpio* speakerEnableGpioPointer{nullptr};

            /**
             * @brief Non-owning pointer to the ADC bound to battery channel A4.
             *
             * @note
             * Never null; the ADC must outlive this controller.
             */
            XWalkAdc* batteryAdcPointer{nullptr};

            /**
             * @brief Nullable non-owning speaker-prime callback context.
             *
             * @note
             * Null is permitted only when `speakerPrimeCallbackValue` supports it.
             */
            contextpointer speakerPrimeContextPointer{nullptr};

            /** @brief Non-null callback that primes output after speaker power is enabled. */
            boardspeakerprimecallback speakerPrimeCallbackValue{nullptr};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Validates all injected hardware roles and callback binding.
             *
             * @param[in] mcuResetGpio
             * GPIO required to use the Robot HAT MCU-reset line.
             *
             * @param[in] speakerEnableGpio
             * GPIO required to use a supported board speaker-enable line.
             *
             * @param[in] batteryAdc
             * ADC required to use battery channel A4.
             *
             * @param[in] speakerPrimeCallback
             * Callback required to prime speaker output.
             *
             * @throws std::invalid_argument
             * If a hardware object is bound to the wrong role or the callback is null.
             */
            static void validateDependencies(const XWalkGpio& mcuResetGpio,
                                             const XWalkGpio& speakerEnableGpio,
                                             const XWalkAdc& batteryAdc,
                                             boardspeakerprimecallback speakerPrimeCallback);

            /**
             * @brief Primes the powered speaker for the required bounded duration.
             *
             * @pre
             * The speaker-enable line is active and the callback context is valid.
             */
            void primeSpeakerOutput();

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a controller from application-owned dependencies.
             *
             * @param[in] mcuResetGpio
             * MCU-reset GPIO that must outlive this controller.
             *
             * @param[in] speakerEnableGpio
             * Board-selected speaker GPIO that must outlive this controller.
             *
             * @param[in] batteryAdc
             * ADC channel A4 object that must outlive this controller.
             *
             * @param[in,out] speakerPrimeContext
             * Nullable non-owning callback context. A non-null object must outlive
             * this controller, and null requires explicit callback support.
             *
             * @param[in] speakerPrimeCallback
             * Non-null callback used after speaker power is enabled.
             *
             * @throws std::invalid_argument
             * If a hardware object is bound to the wrong role or the callback is null.
             */
            XWalkBoardControl(XWalkGpio& mcuResetGpio,
                              XWalkGpio& speakerEnableGpio,
                              XWalkAdc& batteryAdc,
                              contextpointer speakerPrimeContext,
                              boardspeakerprimecallback speakerPrimeCallback);

            /** @brief Destroys the controller without changing or releasing hardware. */
            ~XWalkBoardControl();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction because dependencies are identity-bound. */
            XWalkBoardControl(XWalkBoardControl&&) = delete;
            /** @brief Disables copying of non-owning hardware bindings. */
            XWalkBoardControl(const XWalkBoardControl&) = delete;
            /** @brief Disables move assignment because dependencies are identity-bound. */
            XWalkBoardControl& operator=(XWalkBoardControl&&) = delete;
            /** @brief Disables copy assignment of non-owning hardware bindings. */
            XWalkBoardControl& operator=(const XWalkBoardControl&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Drives a caller-owned GPIO to one logical level.
             *
             * @param[in,out] gpio
             * Non-owning GPIO reference configured as an output by this operation.
             *
             * @param[in] value
             * Logical output level to drive.
             */
            void setPin(XWalkGpio& gpio, boolean value);

            /**
             * @brief Pulses the Robot HAT MCU reset signal.
             *
             * @details
             * Drives reset low for 10 milliseconds, then high for 10 milliseconds.
             *
             * @post
             * The reset line is logically high after successful completion.
             *
             * @warning
             * A GPIO failure after reset is asserted can leave the MCU held in reset.
             */
            void resetMcu();

            /**
             * @brief Acquires the estimated Robot HAT battery voltage.
             *
             * @return
             * ADC voltage multiplied by the board divider ratio, in volts.
             */
            float64 batteryVoltage();

            /**
             * @brief Enables speaker power and primes the audio output.
             *
             * @post
             * Speaker power remains enabled after priming succeeds.
             */
            void enableSpeaker();

            /**
             * @brief Disables physical speaker power.
             *
             * @post
             * The speaker-enable GPIO is logically low after successful completion.
             */
            void disableSpeaker();
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_BOARD_CONTROL_H */
