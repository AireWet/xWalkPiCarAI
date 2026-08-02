/******************************************************************************
 * @file        xAgent_Rpi5CarController.h
 * @brief       Declares the PiCar-X command-line interface.
 *
 * @details
 * Parses CLI arguments and coordinates movement, line tracking, preset actions,
 * sensors, sound, and calibration.
 *
 * @project     xWalk Firmware
 * @module      xWalkController
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

#ifndef XAGENT_RPI5CAR_CONTROLLER_H
#define XAGENT_RPI5CAR_CONTROLLER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarControllerTypes.h"
#include "xAgent_Rpi5CarLineTracking.h"
#include "xAgent_Rpi5CarLocalVoiceChatbot.h"
#include "xAgent_Rpi5CarSelfDrive.h"
#include "xAgent_Rpi5CarSpiTransfer.h"
#include "xAgent_Rpi5CarVoiceActiveCar.h"
#include "xAgent_Rpi5CarVoiceControlledCar.h"
#include "xAgent_Rpi5CarVoicePromptCar.h"

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
 * @class XWalkController
 * @brief Executes one bounded PiCar-X command from caller-supplied arguments.
 *
 * @details
 * Stores non-owning pointers to caller-selected Agent coordinators, a non-owning
 * platform context, and a copied callback table. The caller owns every dependency
 * and must serialize command execution when sharing the CLI.
 */
class XWalkController
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Non-owning PiCar-X pointer that is never null after construction. */
        XWalkPicarx* picarxObject{nullptr};
        /** @brief Nullable non-owning line-tracking pointer supplied for line-track commands. */
        XWalkLineTracking* lineTrackingObject{nullptr};
        /** @brief Nullable non-owning self-drive pointer supplied for self-drive commands. */
        XWalkSelfDrive* selfDriveObject{nullptr};
        /** @brief Nullable non-owning local voice-chatbot pointer. */
        XWalkLocalVoiceChatbot* localVoiceChatbotObject{nullptr};
        /** @brief Nullable non-owning voice-active-car pointer. */
        XWalkVoiceActiveCar* voiceActiveCarObject{nullptr};
        /** @brief Nullable non-owning wake-word voice-controlled-car pointer. */
        XWalkVoiceControlledCar* voiceControlledCarObject{nullptr};
        /** @brief Nullable non-owning spoken movement-demonstration pointer. */
        XWalkVoicePromptCar* voicePromptCarObject{nullptr};
        /** @brief Nullable non-owning SPI transfer Agent pointer. */
        XWalkSpiTransfer* spiTransferObject{nullptr};
        /** @brief Nullable non-owning passive preflight lines supplied for doctor. */
        const hal::stringvector* doctorLinesObject{nullptr};
        /** @brief Nullable non-owning context forwarded synchronously to platform callbacks. */
        hal::contextpointer callbackContext{nullptr};
        /** @brief Complete callback table copied during construction. */
        XWalkControllerCallbacks callbacks{};

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /** @brief Validates that every required callback is non-null. */
        static void validateCallbacks(const XWalkControllerCallbacks& backendCallbacks);
        /** @brief Parses named options beginning at one argument index. */
        static controlleroptions parseOptions(const hal::stringvector& arguments, hal::size startIndex);
        /** @brief Retrieves one option or a caller-supplied default. */
        static hal::string optionValue(const controlleroptions& options, hal::stringview name,
            hal::stringview defaultValue, hal::boolean required);
        /** @brief Rejects options outside a command-specific allow list. */
        static void validateOptions(const controlleroptions& options, const hal::stringvector& allowed);
        /** @brief Parses one complete finite number within inclusive limits. */
        static hal::float64 parseNumber(hal::stringview text, hal::cstring name,
            hal::float64 minimum, hal::float64 maximum);
        /** @brief Converts non-negative seconds to a bounded millisecond delay. */
        static hal::uint32 durationMilliseconds(hal::float64 durationSeconds);
        /** @brief Formats one sensor value with one fractional decimal digit. */
        static hal::string formatOneDecimal(hal::float64 value);
        /** @brief Formats three signed sensor counts in bracketed list form. */
        static hal::string formatValues(const hal::linetrackervalues& values);
        /** @brief Formats three binary statuses in bracketed list form. */
        static hal::string formatStatus(const hal::linetrackerstatus& status);
        /** @brief Parses one contiguous hexadecimal SPI payload. */
        static hal::bytevector parseHexBytes(hal::stringview text);
        /** @brief Formats bytes as uppercase space-separated hexadecimal text. */
        static hal::string formatHexBytes(const hal::bytevector& bytes);
        /** @brief Dispatches one command while a command-scope safety guard is active. */
        hal::int32 executePicarxCommand(const hal::stringvector& arguments);
        /** @brief Reports whether the active command may continue and latches emergency stop otherwise. */
        hal::boolean operationMayContinue();
        /** @brief Performs a cancellable delay using bounded application-owned slices. */
        hal::boolean delayWhileOperationRequested(hal::uint32 durationMs);
        /** @brief Executes the move command. */
        hal::int32 executeMove(const hal::stringvector& arguments);
        /** @brief Executes the turn command. */
        hal::int32 executeTurn(const hal::stringvector& arguments);
        /** @brief Executes the camera command. */
        hal::int32 executeCamera(const hal::stringvector& arguments);
        /** @brief Executes the sensor command. */
        hal::int32 executeSensor(const hal::stringvector& arguments);
        /** @brief Executes foreground line-tracking start or immediate stop. */
        hal::int32 executeLineTracking(const hal::stringvector& arguments);
        /** @brief Executes one named self-drive preset action. */
        hal::int32 executeSelfDrive(const hal::stringvector& arguments);
        /** @brief Executes the sound command. */
        hal::int32 executeSound(const hal::stringvector& arguments);
        /** @brief Executes one bounded full-duplex SPI transfer. */
        hal::int32 executeSpi(const hal::stringvector& arguments);
        /** @brief Prints one passive hardware preflight report. */
        hal::int32 executeDoctor(const hal::stringvector& arguments);
        /** @brief Executes the foreground local voice-chatbot command. */
        hal::int32 executeVoiceChat(const hal::stringvector& arguments);
        /** @brief Executes one voice-active-car start or stop command. */
        hal::int32 executeVoiceActiveCar(const hal::stringvector& arguments);
        /** @brief Executes one wake-word voice-control start or stop command. */
        hal::int32 executeVoiceControlledCar(const hal::stringvector& arguments);
        /** @brief Executes one spoken movement demonstration command. */
        hal::int32 executeVoicePromptCar(const hal::stringvector& arguments);
        /** @brief Executes interactive servo calibration. */
        hal::int32 executeCalibration(const hal::stringvector& arguments);
        /** @brief Performs capped raised-wheel motor and steering verification. */
        hal::boolean executeFirstRunVerification();
        /** @brief Samples, confirms, and persists grayscale line and cliff references. */
        void calibrateGrayscaleReferences();
        /** @brief Calibrates one servo through repeated platform prompts. */
        void calibrateServo(hal::stringview title, hal::stringview prompt,
            hal::float64 minimum, hal::float64 maximum, hal::uint8 servoId);
        /** @brief Writes one complete output line through the injected backend. */
        void output(hal::stringview line) const;
        /** @brief Requests one response through the injected backend. */
        hal::string input(hal::stringview prompt) const;
        /** @brief Delays through the injected backend. */
        void delay(hal::uint32 durationMs) const;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs a CLI around one caller-owned PiCar-X coordinator.
         * @param[in] picarx Coordinator that must outlive this CLI.
         * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
         * @param[in] backendCallbacks Complete non-null synchronous callback table.
         */
        XWalkController(XWalkPicarx& picarx, hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /**
         * @brief Constructs a CLI with foreground line-tracking command support.
         * @param[in] picarx Coordinator that must outlive this CLI.
         * @param[in] lineTracking Line-tracking coordinator that must outlive this CLI.
         * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
         * @param[in] backendCallbacks Complete non-null synchronous callback table.
         */
        XWalkController(XWalkPicarx& picarx, XWalkLineTracking& lineTracking,
            hal::contextpointer context, const XWalkControllerCallbacks& backendCallbacks);

        /**
         * @brief Constructs a CLI with named self-drive action support.
         * @param[in] picarx Coordinator that must outlive this CLI.
         * @param[in] selfDrive Self-drive coordinator that must outlive this CLI.
         * @param[in,out] context Optional platform context that must outlive this CLI when non-null.
         * @param[in] backendCallbacks Complete non-null synchronous callback table.
         */
        XWalkController(XWalkPicarx& picarx, XWalkSelfDrive& selfDrive,
            hal::contextpointer context, const XWalkControllerCallbacks& backendCallbacks);

        /**
         * @brief Constructs a CLI with local voice-chatbot support.
         * @param[in] picarx Coordinator that must outlive this CLI.
         * @param[in] localVoiceChatbot Chatbot coordinator that must outlive this CLI.
         * @param[in,out] context Optional platform callback context.
         * @param[in] backendCallbacks Complete non-null synchronous callback table.
         */
        XWalkController(XWalkPicarx& picarx,
            XWalkLocalVoiceChatbot& localVoiceChatbot,
            hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /** @brief Constructs a CLI with sensor-aware voice-active-car support. */
        XWalkController(XWalkPicarx& picarx, XWalkVoiceActiveCar& voiceActiveCar,
            hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /** @brief Constructs a CLI with wake-word movement-control support. */
        XWalkController(XWalkPicarx& picarx,
            XWalkVoiceControlledCar& voiceControlledCar,
            hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /** @brief Constructs a CLI with the spoken movement demonstration. */
        XWalkController(XWalkPicarx& picarx, XWalkVoicePromptCar& voicePromptCar,
            hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /**
         * @brief Constructs a CLI containing only an SPI transfer Agent.
         * @param[in] spiTransfer SPI Agent that must outlive this CLI.
         * @param[in,out] context Optional platform callback context.
         * @param[in] backendCallbacks Complete non-null synchronous callback table.
         */
        XWalkController(XWalkSpiTransfer& spiTransfer,
            hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /**
         * @brief Constructs a CLI containing only a passive preflight report.
         * @param[in] doctorLines Report lines that must outlive this CLI.
         * @param[in,out] context Optional platform callback context.
         * @param[in] backendCallbacks Complete non-null synchronous callback table.
         */
        XWalkController(const hal::stringvector& doctorLines,
            hal::contextpointer context,
            const XWalkControllerCallbacks& backendCallbacks);

        /** @brief Destroys the CLI without changing or releasing its dependencies. */
        ~XWalkController();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        XWalkController(XWalkController&&) = delete;
        XWalkController(const XWalkController&) = delete;
        XWalkController& operator=(XWalkController&&) = delete;
        XWalkController& operator=(const XWalkController&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Executes one CLI command.
         * @param[in] arguments Command arguments excluding the executable name.
         * @return Zero on success or three when a command-specific backend is unavailable.
         */
        hal::int32 run(const hal::stringvector& arguments);

        /** @brief Returns Linux-style command help with options and examples. */
        static hal::string usage();
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_CONTROLLER_H */
