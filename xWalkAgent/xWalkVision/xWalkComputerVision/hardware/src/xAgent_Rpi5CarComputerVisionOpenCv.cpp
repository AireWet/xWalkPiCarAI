/******************************************************************************
 * @file        xAgent_Rpi5CarComputerVisionOpenCv.cpp
 * @brief       Implements Linux camera computer vision with OpenCV.
 *
 * @details
 * Provides bounded local frame acquisition, HSV color segmentation, Haar
 * frontal-face detection, QR decoding, and timestamped JPEG persistence.
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xAgent_Rpi5CarComputerVisionOpenCv.h"

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::agent
 * @brief Contains application coordinators for the xWalk firmware.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Constructor definitions
 ******************************************************************************/

/**
 * @brief Constructs one idle OpenCV provider.
 * @param[in] configuration Camera, cascade, size, and output settings.
 * @throws std::invalid_argument If a path is empty or not absolute.
 * @throws std::out_of_range If an image dimension is invalid.
 * @throws std::runtime_error If the face cascade cannot be loaded.
 */
XWalkComputerVisionOpenCv::XWalkComputerVisionOpenCv(
    const XWalkComputerVisionOpenCvConfiguration& configuration):
    configurationValue(configuration)
{
    const agent::boolean configurationInvalid =
        static_cast<agent::boolean>(
            configurationValue.cameraDevice.empty() ||
        configurationValue.photoDirectory.empty() ||
        configurationValue.faceCascadePath.empty());
    if (configurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Computer-vision paths must not be empty");
    }
    const agent::boolean cameraConfigurationInvalid =
        static_cast<agent::boolean>(
            !agent::filesystempath(configurationValue.cameraDevice).is_absolute() ||
        !agent::filesystempath(configurationValue.photoDirectory).is_absolute() ||
        !agent::filesystempath(configurationValue.faceCascadePath).is_absolute());
    if (cameraConfigurationInvalid)
    {
        XHAL_THROW_INVALID_ARGUMENT("Computer-vision paths must be absolute");
    }
    if ((configurationValue.widthPixels < 16U) ||
        (configurationValue.widthPixels > 7'680U) ||
        (configurationValue.heightPixels < 16U) ||
        (configurationValue.heightPixels > 4'320U))
    {
        XHAL_THROW_OUT_OF_RANGE("Computer-vision dimensions are outside their range");
    }
    const agent::boolean cascadeLoaded =
        faceCascade.load(configurationValue.faceCascadePath);
    if (cascadeLoaded == false)
    {
        XHAL_THROW_RUNTIME_ERROR("Computer-vision face cascade could not be loaded");
    }
}

/******************************************************************************
 * Destructor definitions
 ******************************************************************************/

/** @brief Releases the camera stream. */
XWalkComputerVisionOpenCv::~XWalkComputerVisionOpenCv() noexcept
{
    const agent::boolean cameraOpen =
        static_cast<agent::boolean>(
            camera.isOpened());
    if (cameraOpen)
    {
        camera.release();
    }
}

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Converts callback context to its required provider.
 * @param[in,out] context Non-null provider pointer.
 * @return Referenced provider.
 * @throws std::invalid_argument If `context` is null.
 */
XWalkComputerVisionOpenCv& XWalkComputerVisionOpenCv::provider(
    agent::contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Computer-vision provider context must not be null");
    }
    return *static_cast<XWalkComputerVisionOpenCv*>(context);
}

/**
 * @brief Opens the configured camera stream.
 * @param[in,out] context Non-null provider pointer.
 * @return `true` when the stream opened and accepted its requested size.
 */
agent::boolean XWalkComputerVisionOpenCv::startProvider(agent::contextpointer context)
{
    XWalkComputerVisionOpenCv& backend = provider(context);
    const agent::boolean cameraOpen =
        static_cast<agent::boolean>(
            backend.camera.isOpened());
    if (cameraOpen)
    {
        return true;
    }
    const agent::boolean opened = backend.camera.open(
        backend.configurationValue.cameraDevice, cv::CAP_V4L2);
    if (!opened)
    {
        return false;
    }
    static_cast<void>(backend.camera.set(cv::CAP_PROP_FRAME_WIDTH,
        static_cast<agent::float64>(backend.configurationValue.widthPixels)));
    static_cast<void>(backend.camera.set(cv::CAP_PROP_FRAME_HEIGHT,
        static_cast<agent::float64>(backend.configurationValue.heightPixels)));
    backend.colorValue = XWalkComputerVisionColor::Close;
    backend.faceEnabledValue = false;
    backend.qrEnabledValue = false;
    return true;
}

/**
 * @brief Releases the camera stream without throwing.
 * @param[in,out] context Nullable provider pointer.
 */
void XWalkComputerVisionOpenCv::stopProvider(agent::contextpointer context) noexcept
{
    if (context == nullptr)
    {
        return;
    }
    XWalkComputerVisionOpenCv& backend =
        *static_cast<XWalkComputerVisionOpenCv*>(context);
    const agent::boolean cameraOpen =
        static_cast<agent::boolean>(
            backend.camera.isOpened());
    if (cameraOpen)
    {
        backend.camera.release();
    }
    backend.colorValue = XWalkComputerVisionColor::Close;
    backend.faceEnabledValue = false;
    backend.qrEnabledValue = false;
}

/**
 * @brief Reads one non-empty frame from the active camera.
 * @return Owned BGR frame.
 * @throws std::logic_error If the provider is not active.
 * @throws std::runtime_error If frame acquisition fails.
 */
cv::Mat XWalkComputerVisionOpenCv::readFrame()
{
    const agent::boolean cameraClosed =
        static_cast<agent::boolean>(
            !camera.isOpened());
    if (cameraClosed)
    {
        XHAL_THROW_LOGIC_ERROR("Computer-vision camera is not started");
    }
    cv::Mat frame;
    const agent::boolean frameRead = camera.read(frame);
    const agent::boolean frameUnavailable =
        static_cast<agent::boolean>(
            (frameRead == false) || frame.empty());
    if (frameUnavailable)
    {
        XHAL_THROW_RUNTIME_ERROR("Computer-vision camera frame acquisition failed");
    }
    return frame;
}

/**
 * @brief Captures one timestamped JPEG photograph.
 * @param[in,out] context Non-null provider pointer.
 * @return Complete saved photograph path.
 * @throws std::runtime_error If directory creation or JPEG writing fails.
 */
agent::string XWalkComputerVisionOpenCv::capturePhoto(agent::contextpointer context)
{
    XWalkComputerVisionOpenCv& backend = provider(context);
    const agent::filesystempath directory{backend.configurationValue.photoDirectory};
    const agent::boolean directoryExists = hal::filesystemEntryExists(directory);
    if (directoryExists == false)
    {
        const agent::boolean directoryCreated = hal::createDirectories(directory);
        if (directoryCreated == false)
        {
            XHAL_THROW_RUNTIME_ERROR(
                "Computer-vision photo directory could not be created");
        }
    }
    const time_t currentTime = ::time(nullptr);
    struct tm localTime{};
    const agent::boolean currentTimeLocalTimeMatched =
        static_cast<agent::boolean>(
            ::localtime_r(&currentTime, &localTime) == nullptr);
    if (currentTimeLocalTimeMatched)
    {
        XHAL_THROW_RUNTIME_ERROR("Computer-vision photo timestamp failed");
    }
    agent::fixedarray<char, 32U> timestamp{};
    const agent::boolean timestampMDMatched =
        static_cast<agent::boolean>(
            ::strftime(timestamp.data(), timestamp.size(), "%Y-%m-%d-%H-%M-%S",
            &localTime) == 0U);
    if (timestampMDMatched)
    {
        XHAL_THROW_RUNTIME_ERROR("Computer-vision photo timestamp formatting failed");
    }
    const agent::filesystempath outputPath = directory /
        (agent::string("photo_") + timestamp.data() + ".jpg");
    const agent::boolean photographWritten =
        cv::imwrite(outputPath.string(), backend.readFrame());
    if (photographWritten == false)
    {
        XHAL_THROW_RUNTIME_ERROR("Computer-vision photograph could not be written");
    }
    return outputPath.string();
}

/**
 * @brief Selects one color-detection mode.
 * @param[in,out] context Non-null provider pointer.
 * @param[in] color Source-compatible color mode.
 */
void XWalkComputerVisionOpenCv::selectColor(agent::contextpointer context,
    XWalkComputerVisionColor color)
{
    static_cast<void>(XWalkComputerVision::colorName(color));
    provider(context).colorValue = color;
}

/**
 * @brief Enables or disables frontal-face detection.
 * @param[in,out] context Non-null provider pointer.
 * @param[in] enabled Requested detector state.
 */
void XWalkComputerVisionOpenCv::switchFace(agent::contextpointer context,
    agent::boolean enabled)
{
    provider(context).faceEnabledValue = enabled;
}

/**
 * @brief Enables or disables QR decoding.
 * @param[in,out] context Non-null provider pointer.
 * @param[in] enabled Requested detector state.
 */
void XWalkComputerVisionOpenCv::switchQr(agent::contextpointer context,
    agent::boolean enabled)
{
    provider(context).qrEnabledValue = enabled;
}

/**
 * @brief Detects the selected HSV color in one BGR frame.
 * @param[in] frame Non-empty BGR image.
 * @return Count and largest qualifying region geometry.
 */
XWalkComputerVisionDetection XWalkComputerVisionOpenCv::detectColor(
    const cv::Mat& frame) const
{
    if (colorValue == XWalkComputerVisionColor::Close)
    {
        return {};
    }
    cv::Mat hsv;
    cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
    cv::Scalar minimum;
    cv::Scalar maximum;
    switch (colorValue)
    {
        case XWalkComputerVisionColor::Red:
            minimum = cv::Scalar(0.0, 100.0, 80.0);
            maximum = cv::Scalar(10.0, 255.0, 255.0);
            break;
        case XWalkComputerVisionColor::Orange:
            minimum = cv::Scalar(10.0, 100.0, 80.0);
            maximum = cv::Scalar(22.0, 255.0, 255.0);
            break;
        case XWalkComputerVisionColor::Yellow:
            minimum = cv::Scalar(22.0, 100.0, 80.0);
            maximum = cv::Scalar(35.0, 255.0, 255.0);
            break;
        case XWalkComputerVisionColor::Green:
            minimum = cv::Scalar(35.0, 80.0, 60.0);
            maximum = cv::Scalar(85.0, 255.0, 255.0);
            break;
        case XWalkComputerVisionColor::Blue:
            minimum = cv::Scalar(85.0, 80.0, 60.0);
            maximum = cv::Scalar(130.0, 255.0, 255.0);
            break;
        case XWalkComputerVisionColor::Purple:
            minimum = cv::Scalar(130.0, 60.0, 50.0);
            maximum = cv::Scalar(170.0, 255.0, 255.0);
            break;
        case XWalkComputerVisionColor::Close:
        default:
            return {};
    }
    cv::Mat mask;
    cv::inRange(hsv, minimum, maximum, mask);
    if (colorValue == XWalkComputerVisionColor::Red)
    {
        cv::Mat upperRed;
        cv::inRange(hsv, cv::Scalar(170.0, 100.0, 80.0),
            cv::Scalar(179.0, 255.0, 255.0), upperRed);
        cv::bitwise_or(mask, upperRed, mask);
    }
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN,
        cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
    computervisioncontourvector contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    XWalkComputerVisionDetection result;
    agent::float64 largestArea{};
    for (const computervisionpointvector& contour : contours)
    {
        const agent::float64 area = cv::contourArea(contour);
        if (area < 100.0)
        {
            continue;
        }
        ++result.count;
        if (area > largestArea)
        {
            largestArea = area;
            const cv::Rect bounds = cv::boundingRect(contour);
            result.centerX = static_cast<agent::int32>(bounds.x + (bounds.width / 2));
            result.centerY = static_cast<agent::int32>(bounds.y + (bounds.height / 2));
            result.width = static_cast<agent::uint32>(bounds.width);
            result.height = static_cast<agent::uint32>(bounds.height);
        }
    }
    return result;
}

/**
 * @brief Detects frontal faces in one BGR frame.
 * @param[in] frame Non-empty BGR image.
 * @return Count and largest detected face geometry.
 */
XWalkComputerVisionDetection XWalkComputerVisionOpenCv::detectFace(
    const cv::Mat& frame)
{
    cv::Mat grayscale;
    cv::cvtColor(frame, grayscale, cv::COLOR_BGR2GRAY);
    cv::equalizeHist(grayscale, grayscale);
    computervisionrectanglevector faces;
    faceCascade.detectMultiScale(grayscale, faces, 1.1, 3, 0,
        cv::Size(24, 24));
    XWalkComputerVisionDetection result;
    result.count = static_cast<agent::uint32>(faces.size());
    agent::int32 largestArea{};
    for (const cv::Rect& face : faces)
    {
        const agent::int32 area = static_cast<agent::int32>(face.area());
        if (area > largestArea)
        {
            largestArea = area;
            result.centerX = static_cast<agent::int32>(face.x + (face.width / 2));
            result.centerY = static_cast<agent::int32>(face.y + (face.height / 2));
            result.width = static_cast<agent::uint32>(face.width);
            result.height = static_cast<agent::uint32>(face.height);
        }
    }
    return result;
}

/**
 * @brief Acquires one frame and applies every enabled detector.
 * @param[in,out] context Non-null provider pointer.
 * @return Current color, face, and QR observations.
 */
XWalkComputerVisionObservation XWalkComputerVisionOpenCv::observeFrame(
    agent::contextpointer context)
{
    XWalkComputerVisionOpenCv& backend = provider(context);
    const cv::Mat frame = backend.readFrame();
    XWalkComputerVisionObservation result;
    if (backend.colorValue != XWalkComputerVisionColor::Close)
    {
        result.color = backend.detectColor(frame);
    }
    if (backend.faceEnabledValue)
    {
        result.face = backend.detectFace(frame);
    }
    if (backend.qrEnabledValue)
    {
        result.qrData = backend.qrDetector.detectAndDecode(frame);
    }
    return result;
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Returns provider callbacks with scheduling entries left for composition.
 * @return Provider operations whose delay and continuation entries are null.
 */
XWalkComputerVisionCallbacks XWalkComputerVisionOpenCv::callbacks() const noexcept
{
    return {&startProvider, &stopProvider, &capturePhoto, &selectColor,
        &switchFace, &switchQr, &observeFrame, nullptr, nullptr};
}

} /* namespace xwalk::agent */
