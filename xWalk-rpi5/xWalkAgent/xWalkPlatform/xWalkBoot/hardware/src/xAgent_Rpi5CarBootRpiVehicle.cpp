/******************************************************************************
 * @file        xAgent_Rpi5CarBootRpiVehicle.cpp
 * @brief       Composes the shared Raspberry Pi vehicle hardware graph.
 *
 * @details
 * Loads deployment-owned device and pin assignments, selects the Robot HAT,
 * resets its MCU, and retains the selected motor graph through one callback.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoot RPi
 * @author      Joxy John
 * @date        2026-08-06
 * @version     1.0.0
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xAgent_Rpi5CarBootRpi.h"
#include "xHal_Rpi5CarTrace.h"

#include "xAgent_Rpi5CarPicarxConfiguration.h"
#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarDevice.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"

namespace xwalk::agent
{

    /**
     * @brief Composes the common Robot HAT and PiCar-X graph.
     * @param[in,out] context Nullable caller-owned application context.
     * @param[in] callback Non-null synchronous application callback.
     * @param[in,out] config Loaded deployment configuration.
     * @return Status returned by the selected mode callback.
     * @warning Resets the configured Robot HAT MCU and may claim actuator resources.
     */
    agent::int32 XWalkBootRpi::runVehicle(agent::contextpointer context,
                                          bootapplicationcallback callback,
                                          hal::XWalkConfigStore& config)
    {
        XWALK_RPIAGENT_TRACE_UID0(RPIAGENT .064, "Boot composing Robot HAT vehicle services");
        const agent::string i2cDevice = config.get("hardware_i2c_device", XHAL_RPI5CAR_I2C_DEFAULT_DEVICE);
        const agent::string gpioDevice = config.get("hardware_gpio_device", XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE);
        const agent::string deviceTreeRoot = config.get("hardware_device_tree_root", XHAL_RPI5CAR_DEVICE_TREE_ROOT);
        const agent::string requestedBoard = config.get("hardware_board", "auto");
        const agent::string gpioChipName = config.get("hardware_gpio_chip_name", "");
        const agent::string gpioChipLabel = config.get("hardware_gpio_chip_label", "");
        const agent::uint32 minimumGpioLineCount = parseUnsigned(config.get("hardware_gpio_minimum_line_count", "28"),
                                                                 "hardware_gpio_minimum_line_count",
                                                                 hal::common::UINT32_MAXIMUM);

        hal::XWalkDevice device(deviceTreeRoot);
        const hal::XWalkDeviceInformation deviceInformation = selectBoard(device.information(), requestedBoard);
        hal::XWalkI2cLinux i2cBackend(i2cDevice.c_str());
        hal::XWalkI2c i2c(&i2cBackend,
                          XHAL_I2C_PROBE_CALLBACK(hal::XWalkI2cLinux),
                          XHAL_I2C_WRITE_REGISTER_CALLBACK(hal::XWalkI2cLinux),
                          XHAL_I2C_READ_CALLBACK(hal::XWalkI2cLinux),
                          XHAL_I2C_READ_REGISTER_CALLBACK(hal::XWalkI2cLinux),
                          XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(hal::XWalkI2cLinux));
        const hal::XWalkGpioCallbacks gpioCallbacks = XHAL_GPIO_CALLBACKS(hal::XWalkGpioLinux);
        hal::XWalkGpioLinux speakerBackend(gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
        hal::XWalkGpio speakerGpio(&speakerBackend, gpioCallbacks, deviceInformation.speakerEnablePin);
        hal::XWalkGpioLinux resetBackend(gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
        hal::XWalkGpio resetGpio(&resetBackend, gpioCallbacks, config.get("hardware_mcu_reset_pin", "MCURST"));
        hal::XWalkAdc batteryAdc(i2c, config.get("hardware_battery_adc_channel", "A4"));
        hal::XWalkBoardControl boardControl(resetGpio, speakerGpio, batteryAdc, nullptr, &primeSpeaker);
        boardControl.resetMcu();
        const agent::uint32 resetSettleMs = parseUnsigned(config.get("hardware_mcu_reset_settle_ms", "200"),
                                                          "hardware_mcu_reset_settle_ms",
                                                          hal::common::UINT32_MAXIMUM);
        hal::common::sleepMilliseconds(resetSettleMs);

        const agent::boolean servoZeroingSelected =
            static_cast<agent::boolean>(selectedMode == XWALK_BOOT_SERVO_ZEROING_REQ);
        if (servoZeroingSelected)
        {
            return runServoZeroing(context, callback, i2c);
        }
        const agent::boolean speakerRequired = static_cast<agent::boolean>(
            (selectedMode == XWALK_BOOT_SELF_DRIVE_REQ) || (selectedMode == XWALK_BOOT_SOUND_REQ) ||
            (selectedMode == XWALK_BOOT_APP_CONTROL_REQ) || (selectedMode == XWALK_BOOT_SOUND_BACKGROUND_MUSIC_REQ) ||
            (selectedMode == XWALK_BOOT_TREASURE_HUNT_REQ));
        if (speakerRequired)
        {
            boardControl.enableSpeaker();
        }

        hal::XWalkPwmTimerState timerState;
        hal::XWalkPwm panPwm(i2c, config.get("hardware_pan_pwm_channel", "P0"), {}, timerState);
        hal::XWalkPwm tiltPwm(i2c, config.get("hardware_tilt_pwm_channel", "P1"), {}, timerState);
        hal::XWalkPwm directionPwm(i2c, config.get("hardware_direction_pwm_channel", "P2"), {}, timerState);
        hal::XWalkServoConfiguration panServoConfiguration;
        panServoConfiguration.minimumAngleDegrees = -90.0;
        panServoConfiguration.centreAngleDegrees = 0.0;
        panServoConfiguration.maximumAngleDegrees = 90.0;
        hal::XWalkServoConfiguration tiltServoConfiguration;
        tiltServoConfiguration.minimumAngleDegrees = -35.0;
        tiltServoConfiguration.centreAngleDegrees = 0.0;
        tiltServoConfiguration.maximumAngleDegrees = 65.0;
        hal::XWalkServoConfiguration directionServoConfiguration;
        directionServoConfiguration.minimumAngleDegrees = -30.0;
        directionServoConfiguration.centreAngleDegrees = 0.0;
        directionServoConfiguration.maximumAngleDegrees = 30.0;
        hal::XWalkServo panServo(panPwm, panServoConfiguration);
        hal::XWalkServo tiltServo(tiltPwm, tiltServoConfiguration);
        hal::XWalkServo directionServo(directionPwm, directionServoConfiguration);
        hal::XWalkGpioLinux triggerBackend(gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
        hal::XWalkGpioLinux echoBackend(gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
        hal::XWalkGpio trigger(&triggerBackend, gpioCallbacks, config.get("hardware_ultrasonic_trigger_pin", "D2"));
        hal::XWalkGpio echo(&echoBackend, gpioCallbacks, config.get("hardware_ultrasonic_echo_pin", "D3"));
        hal::XWalkAdc adc0(i2c, config.get("hardware_grayscale_left_channel", "A0"));
        hal::XWalkAdc adc1(i2c, config.get("hardware_grayscale_middle_channel", "A1"));
        hal::XWalkAdc adc2(i2c, config.get("hardware_grayscale_right_channel", "A2"));
        hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
        hal::XWalkUltrasonic ultrasonic(trigger, echo);
        hal::XWalkMotorsConfiguration motorsConfiguration;
        motorsConfiguration.watchdogTimeoutMilliseconds =
            parseUnsigned(config.get("picarx_motor_watchdog_timeout_ms", "500"),
                          "picarx_motor_watchdog_timeout_ms",
                          hal::XHAL_RPI5CAR_MOTOR_WATCHDOG_MAXIMUM_MILLISECONDS);
        const auto runApplication = [&](hal::XWalkMotors& motors) -> agent::int32
        {
            XWalkPicarx picarx(motors, directionServo, panServo, tiltServo, grayscale, ultrasonic, config);
            static_cast<void>(picarx.initialize());
            return runVehicleMode(context,
                                  callback,
                                  config,
                                  boardControl,
                                  picarx,
                                  gpioDevice,
                                  gpioChipName,
                                  gpioChipLabel,
                                  minimumGpioLineCount,
                                  gpioCallbacks);
        };

        const agent::boolean v5MotorMode =
            static_cast<agent::boolean>(deviceInformation.motorMode == XHAL_RPI5CAR_DEVICE_V5_MOTOR_MODE);
        if (v5MotorMode)
        {
            hal::XWalkPwm leftForwardPwm(
                i2c, config.get("hardware_v5_left_forward_pwm_channel", "P12"), {}, timerState);
            hal::XWalkPwm leftReversePwm(
                i2c, config.get("hardware_v5_left_reverse_pwm_channel", "P13"), {}, timerState);
            hal::XWalkPwm rightForwardPwm(
                i2c, config.get("hardware_v5_right_forward_pwm_channel", "P14"), {}, timerState);
            hal::XWalkPwm rightReversePwm(
                i2c, config.get("hardware_v5_right_reverse_pwm_channel", "P15"), {}, timerState);
            hal::XWalkMotor leftMotor(leftForwardPwm, leftReversePwm);
            hal::XWalkMotor rightMotor(rightForwardPwm, rightReversePwm);
            hal::XWalkMotors motors(leftMotor, rightMotor, motorsConfiguration);
            return runApplication(motors);
        }

        hal::XWalkPwm leftPwm(i2c, config.get("hardware_v4_left_pwm_channel", "P13"), {}, timerState);
        hal::XWalkPwm rightPwm(i2c, config.get("hardware_v4_right_pwm_channel", "P12"), {}, timerState);
        hal::XWalkGpioLinux leftDirectionBackend(gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
        hal::XWalkGpioLinux rightDirectionBackend(
            gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
        hal::XWalkGpio leftDirection(
            &leftDirectionBackend, gpioCallbacks, config.get("hardware_v4_left_direction_pin", "D4"));
        hal::XWalkGpio rightDirection(
            &rightDirectionBackend, gpioCallbacks, config.get("hardware_v4_right_direction_pin", "D5"));
        hal::XWalkMotor leftMotor(leftPwm, leftDirection);
        hal::XWalkMotor rightMotor(rightPwm, rightDirection);
        hal::XWalkMotors motors(leftMotor, rightMotor, motorsConfiguration);
        return runApplication(motors);
    }

} /* namespace xwalk::agent */
