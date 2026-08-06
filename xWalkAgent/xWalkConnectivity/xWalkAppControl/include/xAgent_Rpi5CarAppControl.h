/******************************************************************************
 * @file        xAgent_Rpi5CarAppControl.h
 * @brief       Declares bounded SunFounder mobile-app vehicle coordination.
 * @project     xWalk Firmware
 * @module      xWalkAppControl
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_APP_CONTROL_H
#define XAGENT_RPI5CAR_APP_CONTROL_H

#include "xAgent_Rpi5CarAppControlTypes.h"
#include "xAgent_Rpi5CarPicarx.h"

namespace xwalk::agent
{

/** @brief Ports `12.app_control.py` through injected transport and vision. */
class XWalkAppControl final
{
private:
    XWalkPicarx* picarxObject{nullptr};
    XWalkAppControlCallbacks callbacks{};
    XWalkAppControlConfiguration configurationValue{};
    agent::float64 speedPercentValue{};
    agent::boolean lastColorEnabled{};
    agent::boolean lastFaceEnabled{};
    agent::boolean lastObjectEnabled{};
    agent::uint8 lastLineState{};
    agent::boolean startedValue{};

protected:
    static void validate(const XWalkAppControlCallbacks& providerCallbacks,
        const XWalkAppControlConfiguration& configuration);
    agent::boolean wait(agent::uint32 durationMs) const;
    agent::boolean applyVoice(const agent::string& command);
    agent::boolean applyLineTracking();
    agent::boolean applyObstacleAvoidance();
    void applyJoystick(const XWalkAppControlInput& input);

public:
    XWalkAppControl(XWalkPicarx& picarx,
        const XWalkAppControlCallbacks& providerCallbacks,
        const XWalkAppControlConfiguration& configuration = {});
    ~XWalkAppControl() noexcept;

    XWalkAppControl(const XWalkAppControl&) = delete;
    XWalkAppControl(XWalkAppControl&&) = delete;
    XWalkAppControl& operator=(const XWalkAppControl&) = delete;
    XWalkAppControl& operator=(XWalkAppControl&&) = delete;

    agent::boolean start();
    XWalkAppControlResult step();
    void finish();
    agent::boolean started() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_APP_CONTROL_H */
