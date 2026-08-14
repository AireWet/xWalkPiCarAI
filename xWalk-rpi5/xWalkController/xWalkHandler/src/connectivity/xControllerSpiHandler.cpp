/******************************************************************************
 * @file        xControllerSpiHandler.cpp
 * @brief       Implements the SpiHandler command responsibility.
 *
 * @details
 * Keeps this controller responsibility isolated within its functionality-based
 *handler group.
 *
 * @project     xWalk Firmware
 * @module      xWalkHandler
 *
 * @author      Joxy John
 * @date        2026-08-06
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

#include "xController.h"

#include "xHal_Rpi5CarTrace.h"

#include "xControllerParsing.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller command interfaces for the xWalk firmware.
 */
namespace xwalk::ctrl {

/******************************************************************************
 * Member function definitions
 ******************************************************************************/

/**
 * @brief Executes one bounded full-duplex SPI transfer.
 * @param[in] request Validated non-empty transfer bytes.
 * @return Zero after tracing received bytes, or three when SPI is unavailable.
 */
::ctrl::int32
XWalkController::XWALK_handlerSpi(const XWalkSpiRequest &request) {
  if (spiTransferObject == nullptr) {
    XWALK_CTRL_ERROR(XWALK_EXCEPTION, "SPI backend unavailable");
    return 3;
  }
  const ::ctrl::bytevector receiveData =
      spiTransferObject->transfer(request.transmitData);
  const ::ctrl::string formattedReceiveData =
      XWALK_FORMAT_HEX_BYTES(receiveData);
  XWALK_CTRL_TRACE_UID1(CTRL .021, "%s", formattedReceiveData.c_str());
  return 0;
}

} /* namespace xwalk::ctrl */
