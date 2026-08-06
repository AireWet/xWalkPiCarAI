/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionOpenCv.h
 * @brief       Declares the OpenCV computer-vision provider.
 *
 * @details
 * Owns one Linux camera stream and implements color, frontal-face, QR, and
 * timestamped JPEG operations for the computer-vision Agent callback boundary.
 *
 * @project     xWalk Firmware
 * @module      xWalkComputerVision OpenCV Backend
 *
 * @author      Joxy John
 * @date        2026-08-04
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XAGENT_RPI5CAR_COMPUTER_VISION_OPENCV_H
#define XAGENT_RPI5CAR_COMPUTER_VISION_OPENCV_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarComputerVision.h"

#include <opencv2/objdetect.hpp>
#include <opencv2/videoio.hpp>

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
 * Type definitions
 ******************************************************************************/

/** @brief Sequence of OpenCV contour points used during color detection. */
using computervisionpointvector = std::vector<cv::Point>;
/** @brief Sequence of OpenCV color contours. */
using computervisioncontourvector = std::vector<computervisionpointvector>;
/** @brief Sequence of OpenCV face rectangles. */
using computervisionrectanglevector = std::vector<cv::Rect>;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/**
 * @struct XWalkComputerVisionOpenCvConfiguration
 * @brief Stores the Linux camera and local output configuration.
 */
struct XWalkComputerVisionOpenCvConfiguration
{
    /** @brief Absolute V4L2 device path. */
    agent::string cameraDevice{"/dev/video0"};
    /** @brief Writable directory receiving timestamped JPEG photographs. */
    agent::string photoDirectory{"/tmp/xwalk-pictures"};
    /** @brief Readable frontal-face Haar cascade XML path. */
    agent::string faceCascadePath{
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml"};
    /** @brief Requested capture width from 16 through 7680 pixels. */
    agent::uint32 widthPixels{640U};
    /** @brief Requested capture height from 16 through 4320 pixels. */
    agent::uint32 heightPixels{480U};
};

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkComputerVisionOpenCv
 * @brief Implements the vision callback table with one OpenCV camera stream.
 */
class XWalkComputerVisionOpenCv final
{
private:
    /** @brief Validated provider settings copied at construction. */
    XWalkComputerVisionOpenCvConfiguration configurationValue{};
    /** @brief Owned OpenCV camera stream. */
    cv::VideoCapture camera{};
    /** @brief Owned frontal-face cascade loaded before camera start. */
    cv::CascadeClassifier faceCascade{};
    /** @brief Owned OpenCV QR decoder. */
    cv::QRCodeDetector qrDetector{};
    /** @brief Active selected-color mode. */
    XWalkComputerVisionColor colorValue{XWalkComputerVisionColor::Close};
    /** @brief True when face observations are requested. */
    agent::boolean faceEnabledValue{};
    /** @brief True when QR observations are requested. */
    agent::boolean qrEnabledValue{};

protected:
    /** @brief Converts callback context to its required provider. */
    static XWalkComputerVisionOpenCv& provider(agent::contextpointer context);
    /** @brief Opens the configured camera stream. */
    static agent::boolean startProvider(agent::contextpointer context);
    /** @brief Releases the camera stream without throwing. */
    static void stopProvider(agent::contextpointer context) noexcept;
    /** @brief Captures one timestamped JPEG photograph. */
    static agent::string capturePhoto(agent::contextpointer context);
    /** @brief Selects one color-detection mode. */
    static void selectColor(agent::contextpointer context,
        XWalkComputerVisionColor color);
    /** @brief Enables or disables frontal-face detection. */
    static void switchFace(agent::contextpointer context, agent::boolean enabled);
    /** @brief Enables or disables QR decoding. */
    static void switchQr(agent::contextpointer context, agent::boolean enabled);
    /** @brief Acquires one frame and applies every enabled detector. */
    static XWalkComputerVisionObservation observeFrame(
        agent::contextpointer context);
    /** @brief Reads one non-empty frame from the active camera. */
    cv::Mat readFrame();
    /** @brief Detects the selected HSV color in one BGR frame. */
    XWalkComputerVisionDetection detectColor(const cv::Mat& frame) const;
    /** @brief Detects frontal faces in one BGR frame. */
    XWalkComputerVisionDetection detectFace(const cv::Mat& frame);

public:
    /**
     * @brief Constructs one idle OpenCV provider.
     * @param[in] configuration Camera, cascade, size, and output settings.
     * @throws std::invalid_argument If a path is empty or not absolute.
     * @throws std::out_of_range If an image dimension is invalid.
     * @throws std::runtime_error If the face cascade cannot be loaded.
     */
    explicit XWalkComputerVisionOpenCv(
        const XWalkComputerVisionOpenCvConfiguration& configuration = {});

    /** @brief Releases the camera stream. */
    ~XWalkComputerVisionOpenCv() noexcept;

    XWalkComputerVisionOpenCv(const XWalkComputerVisionOpenCv&) = delete;
    XWalkComputerVisionOpenCv(XWalkComputerVisionOpenCv&&) = delete;
    XWalkComputerVisionOpenCv& operator=(const XWalkComputerVisionOpenCv&) = delete;
    XWalkComputerVisionOpenCv& operator=(XWalkComputerVisionOpenCv&&) = delete;

    /** @brief Returns provider callbacks with scheduling entries left for composition. */
    XWalkComputerVisionCallbacks callbacks() const noexcept;
};

} /* namespace xwalk::agent */

#endif /* XAGENT_RPI5CAR_COMPUTER_VISION_OPENCV_H */
