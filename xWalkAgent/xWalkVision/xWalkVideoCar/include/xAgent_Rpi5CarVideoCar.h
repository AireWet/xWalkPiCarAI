/******************************************************************************
 * @file        xAgent_Rpi5CarVideoCar.h
 * @brief       Declares interactive camera-assisted vehicle control.
 * @project     xWalk Firmware
 * @module      xWalkVideoCar
 * @author      Joxy John
 * @date        2026-08-05
 * @version     1.0.0
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_VIDEO_CAR_H
#define XAGENT_RPI5CAR_VIDEO_CAR_H

#include "xAgent_Rpi5CarPicarx.h"
#include "xAgent_Rpi5CarVideoCarTypes.h"

namespace xwalk::agent
{

/** @brief Ports `11.video_car.py` through owned vehicle and vision services. */
class XWalkVideoCar final
{
private:
    XWalkPicarx* picarxObject{nullptr};
    agent::contextpointer callbackContext{nullptr};
    XWalkComputerVisionCallbacks callbacks{};
    XWalkVideoCarConfiguration configurationValue{};
    XWalkVideoCarMotion motionValue{XWalkVideoCarMotion::Stop};
    agent::uint32 speedPercentValue{};
    agent::boolean startedValue{};

protected:
    static void validate(const XWalkComputerVisionCallbacks& providerCallbacks,
        const XWalkVideoCarConfiguration& configuration);
    void applyMotion();
    agent::boolean wait(agent::uint32 durationMs) const;
    XWalkVideoCarResult result(XWalkVideoCarEvent event,
        const agent::string& photoPath = {}) const;

public:
    XWalkVideoCar(XWalkPicarx& picarx, agent::contextpointer context,
        const XWalkComputerVisionCallbacks& providerCallbacks,
        const XWalkVideoCarConfiguration& configuration = {});
    ~XWalkVideoCar() noexcept;

    XWalkVideoCar(const XWalkVideoCar&) = delete;
    XWalkVideoCar(XWalkVideoCar&&) = delete;
    XWalkVideoCar& operator=(const XWalkVideoCar&) = delete;
    XWalkVideoCar& operator=(XWalkVideoCar&&) = delete;

    /** @brief Starts camera acquisition and completes its warm-up delay. */
    agent::boolean start();
    /** @brief Applies one source-compatible keyboard command. */
    XWalkVideoCarResult handleKey(const agent::string& key);
    /** @brief Stops vehicle motion and camera acquisition. */
    void finish();
    /** @brief Reports whether camera acquisition is active. */
    agent::boolean started() const noexcept;
    /** @brief Returns a stable display name for a retained motion. */
    static agent::string motionName(XWalkVideoCarMotion motion);
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_VIDEO_CAR_H */
