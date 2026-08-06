/******************************************************************************
 * @file        xHal_Rpi5CarCameraLinux.cpp
 * @brief       Implements Linux CSI and USB still-image capture.
 *
 * @project     xWalk Firmware
 * @module      xWalkCamera Linux Backend
 * @author      Joxy John
 * @date        2026-08-02
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarCameraLinux.h"

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarLinuxHeaders.h"

namespace xwalk::hal
{

/** @brief Constructs one Linux camera backend. */
XWalkCameraLinux::XWalkCameraLinux(XWalkCameraConnection connection,
    stringview executable, stringview usbDevice):
    connectionValue(connection), executableName(executable), usbDevicePath(usbDevice)
{
    const hal::boolean executableNameEmpty = executableName.empty();
    if (executableNameEmpty)
    {
        XHAL_THROW_INVALID_ARGUMENT("Camera executable must not be empty");
    }
    const hal::boolean usbDevicePathMissing =
        (connectionValue == XWalkCameraConnection::Usb) && usbDevicePath.empty();
    if (usbDevicePathMissing)
    {
        XHAL_THROW_INVALID_ARGUMENT("USB camera device path must not be empty");
    }
}

/** @brief Releases no process or camera resource while idle. */
XWalkCameraLinux::~XWalkCameraLinux() = default;

/** @brief Returns the capture callback requiring this object as context. */
cameracapturecallback XWalkCameraLinux::callback() const noexcept
{
    return &captureImage;
}

/** @brief Converts callback context into its required backend. */
XWalkCameraLinux& XWalkCameraLinux::backend(contextpointer context)
{
    if (context == nullptr)
    {
        XHAL_THROW_INVALID_ARGUMENT("Camera Linux backend context must not be null");
    }
    return *static_cast<XWalkCameraLinux*>(context);
}

/** @brief Runs the selected capture process and verifies its output. */
boolean XWalkCameraLinux::captureImage(contextpointer context,
    stringview outputPath, const XWalkCameraConfiguration& configuration)
{
    XWalkCameraLinux& cameraBackend = backend(context);
    const boolean completed =
        (cameraBackend.connectionValue == XWalkCameraConnection::Csi)
            ? cameraBackend.captureCsi(outputPath, configuration)
            : cameraBackend.captureUsb(outputPath, configuration);
    return completed && validOutput(outputPath);
}

/** @brief Runs one CSI capture process without a shell. */
boolean XWalkCameraLinux::captureCsi(stringview outputPath,
    const XWalkCameraConfiguration& configuration) const
{
    const string width = common::uint32ToString(configuration.widthPixels);
    const string height = common::uint32ToString(configuration.heightPixels);
    const string timeout = common::uint32ToString(configuration.timeoutMs);
    const string ownedOutput{outputPath};
    const auto childProcess = ::fork();
    if (childProcess < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("CSI camera process creation failed");
    }
    if (childProcess == 0)
    {
        const int32 nullDescriptor = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
        if (nullDescriptor >= 0)
        {
            static_cast<void>(::dup2(nullDescriptor, STDOUT_FILENO));
            static_cast<void>(::dup2(nullDescriptor, STDERR_FILENO));
            static_cast<void>(::close(nullDescriptor));
        }
        ::execlp(executableName.c_str(), executableName.c_str(), "--nopreview",
            "--immediate", "--timeout", timeout.c_str(), "--width", width.c_str(),
            "--height", height.c_str(), "--encoding", "jpg", "--output",
            ownedOutput.c_str(), static_cast<charpointer>(nullptr));
        ::_exit(127);
    }
    return waitForProcess(static_cast<int32>(childProcess), configuration.timeoutMs);
}

/** @brief Runs one USB V4L2 capture process without a shell. */
boolean XWalkCameraLinux::captureUsb(stringview outputPath,
    const XWalkCameraConfiguration& configuration) const
{
    const string videoSize = common::uint32ToString(configuration.widthPixels) +
        "x" + common::uint32ToString(configuration.heightPixels);
    const string ownedOutput{outputPath};
    const auto childProcess = ::fork();
    if (childProcess < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("USB camera process creation failed");
    }
    if (childProcess == 0)
    {
        ::execlp(executableName.c_str(), executableName.c_str(), "-nostdin",
            "-loglevel", "error", "-f", "v4l2", "-video_size", videoSize.c_str(),
            "-i", usbDevicePath.c_str(), "-frames:v", "1", "-y", "-t",
            "1", ownedOutput.c_str(), static_cast<charpointer>(nullptr));
        ::_exit(127);
    }
    return waitForProcess(static_cast<int32>(childProcess), configuration.timeoutMs);
}

/**
 * @brief Waits within one deadline and validates the capture-process exit status.
 * @param[in] processId Positive Linux process identifier returned by `fork()`.
 * @param[in] timeoutMs Maximum wait in milliseconds before the child is terminated.
 * @return `true` only when the child exits successfully before the deadline.
 */
boolean XWalkCameraLinux::waitForProcess(int32 processId, uint32 timeoutMs) noexcept
{
    constexpr uint32 pollIntervalMilliseconds{10U};
    int processStatus{};
    const steadytimestamp deadline = steadyclock::now() + millisecondduration(timeoutMs);
    const hal::boolean waitLoopRequested{true};
    while (waitLoopRequested)
    {
        const hal::boolean deadlinePending = steadyclock::now() < deadline;
        if (deadlinePending == false)
        {
            break;
        }
        const auto waitResult = ::waitpid(processId, &processStatus, WNOHANG);
        if (waitResult == processId)
        {
            return WIFEXITED(processStatus) && (WEXITSTATUS(processStatus) == 0);
        }
        if ((waitResult < 0) && (errno != EINTR))
        {
            return false;
        }
        static_cast<void>(::usleep(pollIntervalMilliseconds * 1'000U));
    }
    static_cast<void>(::kill(processId, SIGKILL));
    auto waitResult = ::waitpid(processId, &processStatus, 0);
    while ((waitResult < 0) && (errno == EINTR))
    {
        waitResult = ::waitpid(processId, &processStatus, 0);
    }
    return false;
}

/** @brief Confirms one non-empty JPEG-sized regular output file. */
boolean XWalkCameraLinux::validOutput(stringview outputPath)
{
    const filesystempath path{outputPath};
    const hal::boolean validFile = filesystemEntryExists(path) && isRegularFile(path);
    if (validFile == false)
    {
        return false;
    }
    const uint64 fileSize = filesystemFileSize(path);
    return (fileSize >= 4U) && (fileSize <= (32U * 1024U * 1024U));
}

} /* namespace xwalk::hal */
