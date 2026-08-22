/******************************************************************************
 * @file        xWalkCbbSignal.h
 * @brief       Defines transport-independent CBB scheduler signals.
 *
 * @details
 * Provides fixed-capacity native signal, routing, and payload records shared by
 * Controller, Agent, and HAL without a Protobuf or server dependency.
 *
 * @project     xWalk Firmware
 * @module      xWalk Common Library
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XWALK_CBB_SIGNAL_H
#define XWALK_CBB_SIGNAL_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

#define XWALK_CLIENT_ADDRESS_SIZE 64U
#define XWALK_CBB_PAYLOAD_SIZE 512U
#define XWALK_HAL_MAILBOX_ID 0x1000U
#define XWALK_CTRL_MAILBOX_ID 0x2000U
#define XWALK_AGENT_MAILBOX_ID 0x3000U

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::ctrl
{

    /** @brief Identifies a native scheduler destination. */
    typedef enum xWalkModuleId
    {
        XWALK_MODULE_NONE = 0,
        XWALK_MODULE_CTRL = 1,
        XWALK_MODULE_AGENT = 2,
        XWALK_MODULE_HAL = 3
    } xWalkModuleId;

    /** @brief Preserves the public stable signal-number registry. */
    typedef enum XWalkSignalNumber
    {
        CXX_XWALK_SIGNAL_UNSPECIFIED = 0,
        CXX_XWALK_I2C_REQ = 0x1081,
        CXX_XWALK_I2C_CFM = 0x1082,
        CXX_XWALK_I2C_REJ = 0x1083,
        CXX_XWALK_CMD_UNKNOWN_REQ = 0x2000,
        CXX_XWALK_CMD_HELP_REQ = 0x2001,
        CXX_XWALK_CMD_SPI_REQ = 0x2002,
        CXX_XWALK_CMD_DOCTOR_REQ = 0x2003,
        CXX_XWALK_CMD_SERVO_ZEROING_REQ = 0x2004,
        CXX_XWALK_CMD_COMPUTER_VISION_REQ = 0x2005,
        CXX_XWALK_CMD_RECORD_VIDEO_REQ = 0x2006,
        CXX_XWALK_CMD_SOUND_BACKGROUND_MUSIC_REQ = 0x2007,
        CXX_XWALK_CMD_TEXT_VISION_TALK_REQ = 0x2008,
        CXX_XWALK_CMD_ONLINE_LLM_TEST_REQ = 0x2009,
        CXX_XWALK_CMD_MOVE_REQ = 0x200A,
        CXX_XWALK_CMD_KEYBOARD_CONTROL_REQ = 0x200B,
        CXX_XWALK_CMD_AVOID_OBSTACLES_REQ = 0x200C,
        CXX_XWALK_CMD_CLIFF_DETECTION_REQ = 0x200D,
        CXX_XWALK_CMD_STARE_AT_YOU_REQ = 0x200E,
        CXX_XWALK_CMD_BULL_FIGHT_REQ = 0x200F,
        CXX_XWALK_CMD_TREASURE_HUNT_REQ = 0x2010,
        CXX_XWALK_CMD_VIDEO_CAR_REQ = 0x2011,
        CXX_XWALK_CMD_APP_CONTROL_REQ = 0x2012,
        CXX_XWALK_CMD_TURN_REQ = 0x2013,
        CXX_XWALK_CMD_CAMERA_REQ = 0x2014,
        CXX_XWALK_CMD_SENSOR_REQ = 0x2015,
        CXX_XWALK_CMD_LINE_TRACK_REQ = 0x2016,
        CXX_XWALK_CMD_SELF_DRIVE_REQ = 0x2017,
        CXX_XWALK_CMD_SOUND_REQ = 0x2018,
        CXX_XWALK_CMD_VOICE_CHAT_REQ = 0x2019,
        CXX_XWALK_CMD_VOICE_ACTIVE_CAR_REQ = 0x201A,
        CXX_XWALK_CMD_GPT_CAR_REQ = 0x201B,
        CXX_XWALK_CMD_VOICE_CONTROLLED_CAR_REQ = 0x201C,
        CXX_XWALK_CMD_VOICE_PROMPT_CAR_REQ = 0x201D,
        CXX_XWALK_CMD_STORYTELLING_ROBOT_REQ = 0x201E,
        CXX_XWALK_CMD_CALIBRATE_REQ = 0x201F,
        CXX_XWALK_CMD_VIDEO_STREAM_REQ = 0x2020,
        CXX_XWALK_APP_CFG_REQ = 0x2081,
        CXX_XWALK_APP_ARGS_REQ = 0x2082,
        CXX_XWALK_CTRL_CMD_REQ = 0x2083,
        CXX_XWALK_NO_ARG_REQ = 0x2084,
        CXX_XWALK_LIFECYCLE_REQ = 0x2085,
        CXX_XWALK_MOVE_REQ = 0x2086,
        CXX_XWALK_TURN_REQ = 0x2087,
        CXX_XWALK_CAMERA_REQ = 0x2088,
        CXX_XWALK_SENSOR_REQ = 0x2089,
        CXX_XWALK_SELF_DRIVE_REQ = 0x208A,
        CXX_XWALK_SPI_REQ = 0x208B,
        CXX_XWALK_GPT_CAR_REQ = 0x208C,
        CXX_XWALK_CALIBRATION_REQ = 0x208D,
        CXX_XWALK_SOUND_REQ = 0x208E,
        CXX_XWALK_SERVO_CAL_CFG_REQ = 0x208F,
        CXX_XWALK_CMD_UNKNOWN_CFM = 0x2100,
        CXX_XWALK_CMD_HELP_CFM = 0x2101,
        CXX_XWALK_CMD_SPI_CFM = 0x2102,
        CXX_XWALK_CMD_DOCTOR_CFM = 0x2103,
        CXX_XWALK_CMD_SERVO_ZEROING_CFM = 0x2104,
        CXX_XWALK_CMD_COMPUTER_VISION_CFM = 0x2105,
        CXX_XWALK_CMD_RECORD_VIDEO_CFM = 0x2106,
        CXX_XWALK_CMD_SOUND_BACKGROUND_MUSIC_CFM = 0x2107,
        CXX_XWALK_CMD_TEXT_VISION_TALK_CFM = 0x2108,
        CXX_XWALK_CMD_ONLINE_LLM_TEST_CFM = 0x2109,
        CXX_XWALK_CMD_MOVE_CFM = 0x210A,
        CXX_XWALK_CMD_KEYBOARD_CONTROL_CFM = 0x210B,
        CXX_XWALK_CMD_AVOID_OBSTACLES_CFM = 0x210C,
        CXX_XWALK_CMD_CLIFF_DETECTION_CFM = 0x210D,
        CXX_XWALK_CMD_STARE_AT_YOU_CFM = 0x210E,
        CXX_XWALK_CMD_BULL_FIGHT_CFM = 0x210F,
        CXX_XWALK_CMD_TREASURE_HUNT_CFM = 0x2110,
        CXX_XWALK_CMD_VIDEO_CAR_CFM = 0x2111,
        CXX_XWALK_CMD_APP_CONTROL_CFM = 0x2112,
        CXX_XWALK_CMD_TURN_CFM = 0x2113,
        CXX_XWALK_CMD_CAMERA_CFM = 0x2114,
        CXX_XWALK_CMD_SENSOR_CFM = 0x2115,
        CXX_XWALK_CMD_LINE_TRACK_CFM = 0x2116,
        CXX_XWALK_CMD_SELF_DRIVE_CFM = 0x2117,
        CXX_XWALK_CMD_SOUND_CFM = 0x2118,
        CXX_XWALK_CMD_VOICE_CHAT_CFM = 0x2119,
        CXX_XWALK_CMD_VOICE_ACTIVE_CAR_CFM = 0x211A,
        CXX_XWALK_CMD_GPT_CAR_CFM = 0x211B,
        CXX_XWALK_CMD_VOICE_CONTROLLED_CAR_CFM = 0x211C,
        CXX_XWALK_CMD_VOICE_PROMPT_CAR_CFM = 0x211D,
        CXX_XWALK_CMD_STORYTELLING_ROBOT_CFM = 0x211E,
        CXX_XWALK_CMD_CALIBRATE_CFM = 0x211F,
        CXX_XWALK_CMD_VIDEO_STREAM_CFM = 0x2120,
        CXX_XWALK_CTRL_CMD_CFM = 0x2183,
        CXX_XWALK_NO_ARG_CFM = 0x2184,
        CXX_XWALK_LIFECYCLE_CFM = 0x2185,
        CXX_XWALK_MOVE_CFM = 0x2186,
        CXX_XWALK_TURN_CFM = 0x2187,
        CXX_XWALK_CAMERA_CFM = 0x2188,
        CXX_XWALK_SENSOR_CFM = 0x2189,
        CXX_XWALK_SELF_DRIVE_CFM = 0x218A,
        CXX_XWALK_SPI_CFM = 0x218B,
        CXX_XWALK_GPT_CAR_CFM = 0x218C,
        CXX_XWALK_CALIBRATION_CFM = 0x218D,
        CXX_XWALK_SOUND_CFM = 0x218E,
        CXX_XWALK_CMD_UNKNOWN_REJ = 0x2200,
        CXX_XWALK_CMD_HELP_REJ = 0x2201,
        CXX_XWALK_CMD_SPI_REJ = 0x2202,
        CXX_XWALK_CMD_DOCTOR_REJ = 0x2203,
        CXX_XWALK_CMD_SERVO_ZEROING_REJ = 0x2204,
        CXX_XWALK_CMD_COMPUTER_VISION_REJ = 0x2205,
        CXX_XWALK_CMD_RECORD_VIDEO_REJ = 0x2206,
        CXX_XWALK_CMD_SOUND_BACKGROUND_MUSIC_REJ = 0x2207,
        CXX_XWALK_CMD_TEXT_VISION_TALK_REJ = 0x2208,
        CXX_XWALK_CMD_ONLINE_LLM_TEST_REJ = 0x2209,
        CXX_XWALK_CMD_MOVE_REJ = 0x220A,
        CXX_XWALK_CMD_KEYBOARD_CONTROL_REJ = 0x220B,
        CXX_XWALK_CMD_AVOID_OBSTACLES_REJ = 0x220C,
        CXX_XWALK_CMD_CLIFF_DETECTION_REJ = 0x220D,
        CXX_XWALK_CMD_STARE_AT_YOU_REJ = 0x220E,
        CXX_XWALK_CMD_BULL_FIGHT_REJ = 0x220F,
        CXX_XWALK_CMD_TREASURE_HUNT_REJ = 0x2210,
        CXX_XWALK_CMD_VIDEO_CAR_REJ = 0x2211,
        CXX_XWALK_CMD_APP_CONTROL_REJ = 0x2212,
        CXX_XWALK_CMD_TURN_REJ = 0x2213,
        CXX_XWALK_CMD_CAMERA_REJ = 0x2214,
        CXX_XWALK_CMD_SENSOR_REJ = 0x2215,
        CXX_XWALK_CMD_LINE_TRACK_REJ = 0x2216,
        CXX_XWALK_CMD_SELF_DRIVE_REJ = 0x2217,
        CXX_XWALK_CMD_SOUND_REJ = 0x2218,
        CXX_XWALK_CMD_VOICE_CHAT_REJ = 0x2219,
        CXX_XWALK_CMD_VOICE_ACTIVE_CAR_REJ = 0x221A,
        CXX_XWALK_CMD_GPT_CAR_REJ = 0x221B,
        CXX_XWALK_CMD_VOICE_CONTROLLED_CAR_REJ = 0x221C,
        CXX_XWALK_CMD_VOICE_PROMPT_CAR_REJ = 0x221D,
        CXX_XWALK_CMD_STORYTELLING_ROBOT_REJ = 0x221E,
        CXX_XWALK_CMD_CALIBRATE_REJ = 0x221F,
        CXX_XWALK_CMD_VIDEO_STREAM_REJ = 0x2220,
        CXX_XWALK_CTRL_CMD_REJ = 0x2283,
        CXX_XWALK_NO_ARG_REJ = 0x2284,
        CXX_XWALK_LIFECYCLE_REJ = 0x2285,
        CXX_XWALK_MOVE_REJ = 0x2286,
        CXX_XWALK_TURN_REJ = 0x2287,
        CXX_XWALK_CAMERA_REJ = 0x2288,
        CXX_XWALK_SENSOR_REJ = 0x2289,
        CXX_XWALK_SELF_DRIVE_REJ = 0x228A,
        CXX_XWALK_SPI_REJ = 0x228B,
        CXX_XWALK_GPT_CAR_REJ = 0x228C,
        CXX_XWALK_CALIBRATION_REJ = 0x228D,
        CXX_XWALK_SOUND_REJ = 0x228E
    } XWalkSignalNumber;

    /** @brief Correlates one native request without dynamic storage. */
    typedef struct xClientAddress
    {
            ::ctrl::uint32 mailBoxId;
            char clientAddress[XWALK_CLIENT_ADDRESS_SIZE];
            ::ctrl::uint32 xWalkLocalIndex;
            ::ctrl::uint32 moduleType;
    } xClientAddress;

    /** @brief Describes caller-owned binary payload input. */
    typedef struct xWalkEncodedPayload
    {
            const ::ctrl::uint8* data;
            ::ctrl::size size;
    } xWalkEncodedPayload;

    /** @brief Describes caller-owned binary payload output storage. */
    typedef struct xWalkPayloadBuffer
    {
            ::ctrl::uint8* data;
            ::ctrl::size capacity;
            ::ctrl::size size;
    } xWalkPayloadBuffer;

    /** @brief Carries one pointer-free native CBB scheduler signal. */
    typedef struct XWalkSignal
    {
            ::ctrl::uint32 sigNo;
            xClientAddress clientInfo;
            xWalkModuleId source;
            xWalkModuleId destination;
            ::ctrl::size payloadSize;
            ::ctrl::uint8 payload[XWALK_CBB_PAYLOAD_SIZE];
    } XWalkSignal;

    /** @brief Defines one native module handler invoked as `(context, signal)`. */
    typedef ::ctrl::int32 (*xWalkSignalHandler_LPP)(::ctrl::contextpointer context, const XWalkSignal* signal);

    /** @brief Carries bounded native rejection details. */
    typedef struct xWalkRejectPayload
    {
            ::ctrl::uint32 errorSignal;
            ::ctrl::uint32 reason;
            char detail[96U];
    } xWalkRejectPayload;

} /* namespace xwalk::ctrl */

#endif /* XWALK_CBB_SIGNAL_H */
