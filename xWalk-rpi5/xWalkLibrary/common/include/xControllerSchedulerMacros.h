/******************************************************************************
 * @file        xControllerSchedulerMacros.h
 * @brief       Defines single-evaluation common signal convenience wrappers.
 *
 * @project     xWalk Firmware
 * @module      xWalk Common Library
 *
 * @author      Joxy John
 * @date        2026-08-22
 * @version     1.0.0
 ******************************************************************************/

#ifndef XCONTROLLER_SCHEDULER_MACROS_H
#define XCONTROLLER_SCHEDULER_MACROS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xWalkCbbApi.h"

/******************************************************************************
 * Macro definitions
 ******************************************************************************/

#define cxx_xWalkCtrlSend_LPP(PAYLOAD, SIG_NO)                                                                         \
    (::xwalk::ctrl::cxx_xWalkSend_LPP(XWALK_CTRL_MAILBOX_ID, (PAYLOAD), (SIG_NO), nullptr))
#define cxx_xWalkAgentSend_LPP(PAYLOAD, SIG_NO)                                                                        \
    (::xwalk::ctrl::cxx_xWalkSend_LPP(XWALK_AGENT_MAILBOX_ID, (PAYLOAD), (SIG_NO), nullptr))
#define cxx_xWalkHalSend_LPP(PAYLOAD, SIG_NO)                                                                          \
    (::xwalk::ctrl::cxx_xWalkSend_LPP(XWALK_HAL_MAILBOX_ID, (PAYLOAD), (SIG_NO), nullptr))

#define cxx_xWalkCtrlGetSignal_LPP(SIGNAL)                                                                             \
    (::xwalk::ctrl::cxx_xWalkGetSignalNumber_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL)))
#define cxx_xWalkAgentGetSignal_LPP(SIGNAL)                                                                            \
    (::xwalk::ctrl::cxx_xWalkGetSignalNumber_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL)))
#define cxx_xWalkHalGetSignal_LPP(SIGNAL) (::xwalk::ctrl::cxx_xWalkGetSignalNumber_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL)))

#define cxx_xWalkCtrlSetSignal_LPP(SIGNAL, SIG_NO)                                                                     \
    (::xwalk::ctrl::cxx_xWalkSetSignalNumber_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL), (SIG_NO)))
#define cxx_xWalkAgentSetSignal_LPP(SIGNAL, SIG_NO)                                                                    \
    (::xwalk::ctrl::cxx_xWalkSetSignalNumber_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL), (SIG_NO)))
#define cxx_xWalkHalSetSignal_LPP(SIGNAL, SIG_NO)                                                                      \
    (::xwalk::ctrl::cxx_xWalkSetSignalNumber_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL), (SIG_NO)))

#define cxx_xWalkCtrlGetClientInfo_LPP(SIGNAL, CLIENT_INFO)                                                            \
    (::xwalk::ctrl::cxx_xWalkGetClientInfo_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL), (CLIENT_INFO)))
#define cxx_xWalkAgentGetClientInfo_LPP(SIGNAL, CLIENT_INFO)                                                           \
    (::xwalk::ctrl::cxx_xWalkGetClientInfo_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL), (CLIENT_INFO)))
#define cxx_xWalkHalGetClientInfo_LPP(SIGNAL, CLIENT_INFO)                                                             \
    (::xwalk::ctrl::cxx_xWalkGetClientInfo_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL), (CLIENT_INFO)))

#define cxx_xWalkCtrlSetClientInfo_LPP(SIGNAL, CLIENT_INFO)                                                            \
    (::xwalk::ctrl::cxx_xWalkSetClientInfo_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL), (CLIENT_INFO)))
#define cxx_xWalkAgentSetClientInfo_LPP(SIGNAL, CLIENT_INFO)                                                           \
    (::xwalk::ctrl::cxx_xWalkSetClientInfo_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL), (CLIENT_INFO)))
#define cxx_xWalkHalSetClientInfo_LPP(SIGNAL, CLIENT_INFO)                                                             \
    (::xwalk::ctrl::cxx_xWalkSetClientInfo_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL), (CLIENT_INFO)))

#define cxx_xWalkCtrlGetPayload_LPP(SIGNAL, PAYLOAD)                                                                   \
    (::xwalk::ctrl::cxx_xWalkGetPayload_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL), (PAYLOAD)))
#define cxx_xWalkAgentGetPayload_LPP(SIGNAL, PAYLOAD)                                                                  \
    (::xwalk::ctrl::cxx_xWalkGetPayload_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL), (PAYLOAD)))
#define cxx_xWalkHalGetPayload_LPP(SIGNAL, PAYLOAD)                                                                    \
    (::xwalk::ctrl::cxx_xWalkGetPayload_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL), (PAYLOAD)))

#define cxx_xWalkCtrlSetPayload_LPP(SIGNAL, PAYLOAD)                                                                   \
    (::xwalk::ctrl::cxx_xWalkSetPayload_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL), (PAYLOAD)))
#define cxx_xWalkAgentSetPayload_LPP(SIGNAL, PAYLOAD)                                                                  \
    (::xwalk::ctrl::cxx_xWalkSetPayload_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL), (PAYLOAD)))
#define cxx_xWalkHalSetPayload_LPP(SIGNAL, PAYLOAD)                                                                    \
    (::xwalk::ctrl::cxx_xWalkSetPayload_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL), (PAYLOAD)))

#define CXX_getCtrlSignal_LPP(SIGNAL) cxx_xWalkCtrlGetSignal_LPP((SIGNAL))
#define CXX_getAgentSignal_LPP(SIGNAL) cxx_xWalkAgentGetSignal_LPP((SIGNAL))
#define CXX_getHalSignal_LPP(SIGNAL) cxx_xWalkHalGetSignal_LPP((SIGNAL))

#define CXX_setCtrlSignal_LPP(SIGNAL, SIG_NO) cxx_xWalkCtrlSetSignal_LPP((SIGNAL), (SIG_NO))
#define CXX_setAgentSignal_LPP(SIGNAL, SIG_NO) cxx_xWalkAgentSetSignal_LPP((SIGNAL), (SIG_NO))
#define CXX_setHalSignal_LPP(SIGNAL, SIG_NO) cxx_xWalkHalSetSignal_LPP((SIGNAL), (SIG_NO))

#define CXX_getCtrlClientInfo_LPP(SIGNAL, CLIENT_INFO) cxx_xWalkCtrlGetClientInfo_LPP((SIGNAL), (CLIENT_INFO))
#define CXX_getAgentClientInfo_LPP(SIGNAL, CLIENT_INFO) cxx_xWalkAgentGetClientInfo_LPP((SIGNAL), (CLIENT_INFO))
#define CXX_getHalClientInfo_LPP(SIGNAL, CLIENT_INFO) cxx_xWalkHalGetClientInfo_LPP((SIGNAL), (CLIENT_INFO))

#define CXX_setCtrlClientInfo_LPP(SIGNAL, CLIENT_INFO) cxx_xWalkCtrlSetClientInfo_LPP((SIGNAL), (CLIENT_INFO))
#define CXX_setAgentClientInfo_LPP(SIGNAL, CLIENT_INFO) cxx_xWalkAgentSetClientInfo_LPP((SIGNAL), (CLIENT_INFO))
#define CXX_setHalClientInfo_LPP(SIGNAL, CLIENT_INFO) cxx_xWalkHalSetClientInfo_LPP((SIGNAL), (CLIENT_INFO))

#define CXX_setCtrlPayload_LPP(SIGNAL, PAYLOAD) cxx_xWalkCtrlSetPayload_LPP((SIGNAL), (PAYLOAD))
#define CXX_setAgentPayload_LPP(SIGNAL, PAYLOAD) cxx_xWalkAgentSetPayload_LPP((SIGNAL), (PAYLOAD))
#define CXX_setHalPayload_LPP(SIGNAL, PAYLOAD) cxx_xWalkHalSetPayload_LPP((SIGNAL), (PAYLOAD))

#define CXX_getCtrlPayload_LPP(SIGNAL, PAYLOAD) cxx_xWalkCtrlGetPayload_LPP((SIGNAL), (PAYLOAD))
#define CXX_getAgentPayload_LPP(SIGNAL, PAYLOAD) cxx_xWalkAgentGetPayload_LPP((SIGNAL), (PAYLOAD))
#define CXX_getHalPayload_LPP(SIGNAL, PAYLOAD) cxx_xWalkHalGetPayload_LPP((SIGNAL), (PAYLOAD))

#define CXX_sendCtrlSignal_LPP(SIGNAL, ADDRESS)                                                                        \
    (::xwalk::ctrl::cxx_xWalkSendSignal_LPP(XWALK_CTRL_MAILBOX_ID, (SIGNAL), (ADDRESS)))
#define CXX_sendAgentSignal_LPP(SIGNAL, ADDRESS)                                                                       \
    (::xwalk::ctrl::cxx_xWalkSendSignal_LPP(XWALK_AGENT_MAILBOX_ID, (SIGNAL), (ADDRESS)))
#define CXX_sendHalSignal_LPP(SIGNAL, ADDRESS)                                                                         \
    (::xwalk::ctrl::cxx_xWalkSendSignal_LPP(XWALK_HAL_MAILBOX_ID, (SIGNAL), (ADDRESS)))

#endif /* XCONTROLLER_SCHEDULER_MACROS_H */
