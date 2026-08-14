/******************************************************************************
 * @file        xAgent_Rpi5CarPicarxHardwareTest.cpp
 * @brief       Provides the physical PiCar-X reset and stop smoke test.
 *
 * @details
 * Detects the Robot HAT revision, composes its Linux backends, resets the MCU,
 * and invokes the safe close state through the selected motor-driver mode.
 *
 * @project     xWalk Firmware
 * @module      xWalkPicarx Hardware Test
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarPicarxConfiguration.h"

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarBoardControl.h"
#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarDevice.h"
#include "xHal_Rpi5CarGpioLinux.h"
#include "xHal_Rpi5CarI2cLinux.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains hardware-test callbacks private to this translation unit. */
namespace {

/**
 * @brief Accepts the unused speaker-prime request required by board control.
 * @param[in] context Optional context; unused.
 * @param[in] durationMs Requested duration in milliseconds; unused because
 * speaker power is never enabled.
 */
void primeSpeaker(agent::contextpointer context, agent::uint32 durationMs) {
  static_cast<void>(context);
  static_cast<void>(durationMs);
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Resets the Robot HAT MCU, composes the PiCar-X agent, and commands its
 * safe closed state.
 * @return Zero after every physical operation succeeds; an exception reports
 * hardware failure.
 * @warning This function accesses physical I2C, GPIO, motors, and servos. Lift
 * the wheels and clear motion.
 */
agent::int32 main() {
  xwalk::hal::XWalkConfigStore config(XWALK_PICARX_CONFIG_FILE);
  const agent::string i2cDevice =
      config.get("hardware_i2c_device", XHAL_RPI5CAR_I2C_DEFAULT_DEVICE);
  const agent::string gpioDevice =
      config.get("hardware_gpio_device", XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE);
  const agent::string deviceTreeRoot =
      config.get("hardware_device_tree_root", XHAL_RPI5CAR_DEVICE_TREE_ROOT);
  const agent::string requestedBoard = config.get("hardware_board", "auto");
  const agent::string gpioChipName = config.get("hardware_gpio_chip_name", "");
  const agent::string gpioChipLabel =
      config.get("hardware_gpio_chip_label", "");
  constexpr agent::uint32 minimumGpioLineCount{28U};

  xwalk::hal::XWalkDevice device(deviceTreeRoot);
  xwalk::hal::XWalkDeviceInformation deviceInformation = device.information();
  if (requestedBoard == "robot_hat_v4") {
    if (deviceInformation.detected) {
      XWALK_RPIAGENT_ERROR(
          XWALK_RUNTIME,
          "Configured Robot HAT v4 conflicts with detected Robot HAT v5");
    }
    deviceInformation = {};
  } else if (((requestedBoard == "auto") ||
              (requestedBoard == "robot_hat_v5")) &&
             !deviceInformation.detected) {
    XWALK_RPIAGENT_ERROR(XWALK_RUNTIME, "Hardware test requires verified HAT "
                                        "detection or explicit v4 selection");
  } else if ((requestedBoard != "auto") && (requestedBoard != "robot_hat_v5")) {
    XWALK_RPIAGENT_ERROR(
        XWALK_INVAL,
        "hardware_board must be auto, robot_hat_v4, or robot_hat_v5");
  }

  xwalk::hal::XWalkI2cLinux i2cBackend(i2cDevice.c_str());
  xwalk::hal::XWalkI2c i2c(
      &i2cBackend, XHAL_I2C_PROBE_CALLBACK(xwalk::hal::XWalkI2cLinux),
      XHAL_I2C_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
      XHAL_I2C_READ_CALLBACK(xwalk::hal::XWalkI2cLinux),
      XHAL_I2C_READ_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux),
      XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(xwalk::hal::XWalkI2cLinux));

  {
    xwalk::hal::XWalkGpioLinux resetBackend(
        gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
    xwalk::hal::XWalkGpioLinux speakerBackend(
        gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
    const xwalk::hal::XWalkGpioCallbacks callbacks =
        XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux);
    xwalk::hal::XWalkGpio resetGpio(&resetBackend, callbacks, "MCURST");
    xwalk::hal::XWalkGpio speakerGpio(&speakerBackend, callbacks,
                                      deviceInformation.speakerEnablePin);
    xwalk::hal::XWalkAdc batteryAdc(i2c, "A4");
    xwalk::hal::XWalkBoardControl boardControl(
        resetGpio, speakerGpio, batteryAdc, nullptr, &primeSpeaker);
    boardControl.resetMcu();
    xwalk::hal::common::sleepMilliseconds(200U);
  }

  xwalk::hal::XWalkPwmTimerState timerState;
  xwalk::hal::XWalkPwm panPwm(i2c, "P0", {}, timerState);
  xwalk::hal::XWalkPwm tiltPwm(i2c, "P1", {}, timerState);
  xwalk::hal::XWalkPwm directionPwm(i2c, "P2", {}, timerState);
  xwalk::hal::XWalkServo panServo(panPwm);
  xwalk::hal::XWalkServo tiltServo(tiltPwm);
  xwalk::hal::XWalkServo directionServo(directionPwm);

  xwalk::hal::XWalkGpioLinux triggerBackend(
      gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
  xwalk::hal::XWalkGpioLinux echoBackend(gpioDevice.c_str(), gpioChipName,
                                         gpioChipLabel, minimumGpioLineCount);
  const xwalk::hal::XWalkGpioCallbacks callbacks =
      XHAL_GPIO_CALLBACKS(xwalk::hal::XWalkGpioLinux);
  xwalk::hal::XWalkGpio trigger(&triggerBackend, callbacks, "D2");
  xwalk::hal::XWalkGpio echo(&echoBackend, callbacks, "D3");

  xwalk::hal::XWalkAdc adc0(i2c, "A0");
  xwalk::hal::XWalkAdc adc1(i2c, "A1");
  xwalk::hal::XWalkAdc adc2(i2c, "A2");
  xwalk::hal::XWalkGrayscaleModule grayscale(adc0, adc1, adc2);
  xwalk::hal::XWalkUltrasonic ultrasonic(trigger, echo);
  const auto closePicarx =
      [&](xwalk::hal::XWalkMotors &motors) -> agent::int32 {
    xwalk::agent::XWalkPicarx picarx(motors, directionServo, panServo,
                                     tiltServo, grayscale, ultrasonic, config);
    static_cast<void>(picarx.initialize());
    picarx.close();
    return 0;
  };

  if (deviceInformation.motorMode == XHAL_RPI5CAR_DEVICE_V5_MOTOR_MODE) {
    xwalk::hal::XWalkPwm leftForwardPwm(i2c, "P12", {}, timerState);
    xwalk::hal::XWalkPwm leftReversePwm(i2c, "P13", {}, timerState);
    xwalk::hal::XWalkPwm rightForwardPwm(i2c, "P14", {}, timerState);
    xwalk::hal::XWalkPwm rightReversePwm(i2c, "P15", {}, timerState);
    xwalk::hal::XWalkMotor leftMotor(leftForwardPwm, leftReversePwm);
    xwalk::hal::XWalkMotor rightMotor(rightForwardPwm, rightReversePwm);
    xwalk::hal::XWalkMotors motors(leftMotor, rightMotor);
    return closePicarx(motors);
  }

  xwalk::hal::XWalkPwm leftPwm(i2c, "P13", {}, timerState);
  xwalk::hal::XWalkPwm rightPwm(i2c, "P12", {}, timerState);
  xwalk::hal::XWalkGpioLinux leftDirectionBackend(
      gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
  xwalk::hal::XWalkGpioLinux rightDirectionBackend(
      gpioDevice.c_str(), gpioChipName, gpioChipLabel, minimumGpioLineCount);
  xwalk::hal::XWalkGpio leftDirection(&leftDirectionBackend, callbacks, "D4");
  xwalk::hal::XWalkGpio rightDirection(&rightDirectionBackend, callbacks, "D5");
  xwalk::hal::XWalkMotor leftMotor(leftPwm, leftDirection);
  xwalk::hal::XWalkMotor rightMotor(rightPwm, rightDirection);
  xwalk::hal::XWalkMotors motors(leftMotor, rightMotor);
  return closePicarx(motors);
}
