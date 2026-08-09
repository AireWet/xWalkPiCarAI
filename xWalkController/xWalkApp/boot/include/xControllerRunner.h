/******************************************************************************
 * @file        xControllerRunner.h
 * @brief       Declares boot-service Controller command composition.
 *
 * @details
 * Exposes the application callback that composes one Controller around the
 * services retained by an active Agent boot graph.
 *
 * @project     xWalk Firmware
 * @module      xWalkController Application
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

#ifndef XCONTROLLER_RUNNER_H
#define XCONTROLLER_RUNNER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Forward declarations
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains Agent boot services consumed by the Controller application.
 */
namespace xwalk::agent
{
struct XWalkBootServices;
} /* namespace xwalk::agent */

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::ctrl
 * @brief Contains Controller application composition for the xWalk firmware.
 */
namespace xwalk::ctrl
{

/******************************************************************************
 * Function declarations
 ******************************************************************************/

/**
 * @brief Executes one CLI command through services retained by xWalkBoot.
 * @param[in,out] context Non-null Controller boot context valid throughout the call.
 * @param[in,out] services Command-specific non-owning services retained by xWalkBoot.
 * @return Command-specific status, or three when the required base service is absent.
 */
::ctrl::int32 XWALK_runController(::ctrl::contextpointer context,
    xwalk::agent::XWalkBootServices& services);

} /* namespace xwalk::ctrl */

#endif /* XCONTROLLER_RUNNER_H */
