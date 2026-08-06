/******************************************************************************
 * @file        xHal_Rpi5CarUtilsLinux.cpp
 * @brief       Defines Raspberry Pi Linux utility platform operations.
 *
 * @details
 * Implements terminal output, mixer control, child-process execution,
 * executable discovery, IPv4 and username lookup, and stderr redirection.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Linux Backend
 *
 * @author      Joxy John
 * @date        2026-08-01
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

#include "xHal_Rpi5CarUtilsLinux.h"

#include "xHal_Rpi5CarLinuxHeaders.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Returns the ANSI Select Graphic Rendition code for a utility color.
 *
 * @param[in] color
 * Supported terminal color.
 *
 * @return
 * Non-owning static ANSI code text.
 *
 * @throws std::out_of_range
 * If `color` is not supported.
 */
cstring XWalkUtilsLinux::colorCode(XWalkUtilityColor color)
{
    switch (color)
    {
        case XWalkUtilityColor::Gray:
            return "1;30";
        case XWalkUtilityColor::Red:
            return "0;31";
        case XWalkUtilityColor::Green:
            return "0;32";
        case XWalkUtilityColor::Yellow:
            return "0;33";
        case XWalkUtilityColor::Blue:
            return "0;34";
        case XWalkUtilityColor::Purple:
            return "0;35";
        case XWalkUtilityColor::DarkGreen:
            return "0;36";
        case XWalkUtilityColor::White:
            return "0;37";
        default:
            XHAL_THROW_OUT_OF_RANGE("Linux utility color is not supported");
    }
}

/**
 * @brief Writes a complete byte sequence to one Linux descriptor.
 *
 * @param[in] descriptor
 * Open writable Linux descriptor.
 *
 * @param[in] bytes
 * Byte view written completely before return.
 *
 * @throws std::runtime_error
 * If the descriptor write cannot complete.
 */
void XWalkUtilsLinux::writeAll(int32 descriptor, stringview bytes)
{
    size writtenByteCount{};
    const hal::boolean processingLoopRequested{true};
    while (processingLoopRequested)
    {
        const hal::boolean bytesRemaining =
            static_cast<hal::boolean>(
                writtenByteCount < bytes.size());
        if (bytesRemaining == false)
        {
            break;
        }
        const size remainingByteCount = bytes.size() - writtenByteCount;
        const auto writeResult = ::write(descriptor, bytes.data() + writtenByteCount,
            remainingByteCount);
        if (writeResult > 0)
        {
            writtenByteCount += static_cast<size>(writeResult);
        }
        else if ((writeResult < 0) && (errno == EINTR))
        {
            /* Retry an interrupted descriptor write. */
        }
        else
        {
            XHAL_THROW_RUNTIME_ERROR("Linux utility descriptor write failed");
        }
    }
}

/**
 * @brief Checks whether a path identifies an accessible executable file.
 *
 * @param[in] path
 * Non-empty Linux filesystem path.
 *
 * @return
 * `true` when the path is a regular executable file; otherwise `false`.
 */
boolean XWalkUtilsLinux::isExecutableFile(const string& path)
{
    struct stat pathStatus{};
    return (::stat(path.c_str(), &pathStatus) == 0) &&
        S_ISREG(pathStatus.st_mode) && (::access(path.c_str(), X_OK) == 0);
}

/******************************************************************************
 * Public member function definitions
 ******************************************************************************/

/**
 * @brief Writes ANSI-colored text to standard output.
 *
 * @param[in] color
 * Supported terminal color.
 *
 * @param[in] message
 * Message bytes written between the ANSI prefix and reset sequence.
 *
 * @param[in] ending
 * Bytes appended after the reset sequence.
 *
 * @param[in] flush
 * Flush request retained for callback compatibility; descriptor writes are
 * synchronous and have no user-space stream buffer.
 *
 * @throws std::out_of_range
 * If `color` is not supported.
 *
 * @throws std::runtime_error
 * If standard output cannot accept the complete record.
 */
void XWalkUtilsLinux::writeOutput(XWalkUtilityColor color, stringview message,
    stringview ending, boolean flush) const
{
    static_cast<void>(flush);
    string record{"\033["};
    record.append(colorCode(color));
    record.push_back('m');
    record.append(message.data(), message.size());
    record.append("\033[0m");
    record.append(ending.data(), ending.size());
    writeAll(STDOUT_FILENO, record);
}

/**
 * @brief Applies the PCM playback volume through `amixer`.
 *
 * @param[in] volumePercent
 * Volume in the inclusive range zero through one hundred percent.
 *
 * @throws std::runtime_error
 * If `amixer` does not complete successfully.
 */
void XWalkUtilsLinux::setSystemVolume(uint8 volumePercent) const
{
    const string command = "amixer -M sset 'PCM' " + std::to_string(volumePercent) + "%";
    const XWalkCommandResult commandResult = runCommandProcess(command);
    if (commandResult.status != 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux utility could not set PCM volume");
    }
}

/**
 * @brief Executes trusted command text through the system shell.
 *
 * @param[in] command
 * Trusted command text forwarded without modification.
 *
 * @param[in] user
 * Optional existing Linux user applied in the child process.
 *
 * @param[in] group
 * Optional existing Linux group applied in the child process.
 *
 * @return
 * Normalized exit status and owned combined standard output and error.
 *
 * @throws std::invalid_argument
 * If a requested user or group cannot be resolved.
 *
 * @throws std::runtime_error
 * If pipe, process, read, or wait operations fail.
 *
 * @warning
 * Shell metacharacters are interpreted. Validate every untrusted input before
 * building or forwarding command text.
 */
XWalkCommandResult XWalkUtilsLinux::runCommandProcess(stringview command,
    stringview user, stringview group) const
{
    boolean changeUser{false};
    boolean changeGroup{false};
    uint32 userId{};
    uint32 groupId{};

    const string ownedUser{user};
    const hal::boolean ownedUserAvailable =
        static_cast<hal::boolean>(
            !ownedUser.empty());
    if (ownedUserAvailable)
    {
        const passwd* const userInformation = ::getpwnam(ownedUser.c_str());
        if (userInformation == nullptr)
        {
            XHAL_THROW_INVALID_ARGUMENT("Linux utility command user was not found");
        }
        userId = static_cast<uint32>(userInformation->pw_uid);
        changeUser = true;
    }

    const string ownedGroup{group};
    const hal::boolean ownedGroupAvailable =
        static_cast<hal::boolean>(
            !ownedGroup.empty());
    if (ownedGroupAvailable)
    {
        const struct group* const groupInformation = ::getgrnam(ownedGroup.c_str());
        if (groupInformation == nullptr)
        {
            XHAL_THROW_INVALID_ARGUMENT("Linux utility command group was not found");
        }
        groupId = static_cast<uint32>(groupInformation->gr_gid);
        changeGroup = true;
    }

    fixedarray<int32, 2U> outputPipe{};
    const hal::boolean outputPipeDifferent =
        static_cast<hal::boolean>(
            ::pipe2(outputPipe.data(), O_CLOEXEC) != 0);
    if (outputPipeDifferent)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux utility command pipe creation failed");
    }

    const string ownedCommand{command};
    const auto childProcess = ::fork();
    if (childProcess < 0)
    {
        static_cast<void>(::close(outputPipe[0U]));
        static_cast<void>(::close(outputPipe[1U]));
        XHAL_THROW_RUNTIME_ERROR("Linux utility command process creation failed");
    }

    if (childProcess == 0)
    {
        static_cast<void>(::close(outputPipe[0U]));
        const hal::boolean outputPipeInvalid =
            static_cast<hal::boolean>(
                (::dup2(outputPipe[1U], STDOUT_FILENO) < 0) ||
            (::dup2(outputPipe[1U], STDERR_FILENO) < 0));
        if (outputPipeInvalid)
        {
            ::_exit(127);
        }
        static_cast<void>(::close(outputPipe[1U]));

        const hal::boolean groupChangeFailed =
            static_cast<hal::boolean>(
                changeGroup && (::setgid(static_cast<gid_t>(groupId)) != 0));
        if (groupChangeFailed)
        {
            ::_exit(127);
        }
        const hal::boolean userChangeFailed =
            static_cast<hal::boolean>(
                changeUser && (::setuid(static_cast<uid_t>(userId)) != 0));
        if (userChangeFailed)
        {
            ::_exit(127);
        }

        ::execl("/bin/sh", "sh", "-c", ownedCommand.c_str(),
            static_cast<charpointer>(nullptr));
        ::_exit(127);
    }

    static_cast<void>(::close(outputPipe[1U]));
    string output;
    fixedarray<char, 4'096U> buffer{};
    while (true)
    {
        const auto readResult = ::read(outputPipe[0U], buffer.data(), buffer.size());
        if (readResult > 0)
        {
            output.append(buffer.data(), static_cast<size>(readResult));
        }
        else if (readResult == 0)
        {
            break;
        }
        else if (errno == EINTR)
        {
            /* Retry an interrupted pipe read. */
        }
        else
        {
            static_cast<void>(::close(outputPipe[0U]));
            static_cast<void>(::waitpid(childProcess, nullptr, 0));
            XHAL_THROW_RUNTIME_ERROR("Linux utility command output read failed");
        }
    }
    static_cast<void>(::close(outputPipe[0U]));

    int childStatus{};
    auto waitResult = ::waitpid(childProcess, &childStatus, 0);
    while ((waitResult < 0) && (errno == EINTR))
    {
        waitResult = ::waitpid(childProcess, &childStatus, 0);
    }
    if (waitResult < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux utility command wait failed");
    }

    int32 status{127};
    const hal::boolean childExitedNormally =
        static_cast<hal::boolean>(
            WIFEXITED(childStatus));
    if (childExitedNormally)
    {
        status = static_cast<int32>(WEXITSTATUS(childStatus));
    }
    else
    {
        const hal::boolean childTerminatedBySignal =
            static_cast<hal::boolean>(
                WIFSIGNALED(childStatus));
        if (childTerminatedBySignal)
        {
            status = static_cast<int32>(128 + WTERMSIG(childStatus));
        }
        else
        {
            /* Retain the conventional command-launch failure status. */
        }
    }
    return {status, output};
}

/**
 * @brief Checks whether one executable path or name is accessible.
 *
 * @param[in] executable
 * Absolute path, relative path containing a slash, or bare executable name.
 *
 * @return
 * `true` when an executable file is accessible; otherwise `false`.
 */
boolean XWalkUtilsLinux::executableExists(stringview executable) const
{
    const hal::boolean executableEmpty =
        static_cast<hal::boolean>(
            executable.empty());
    if (executableEmpty)
    {
        return false;
    }

    const string ownedExecutable{executable};
    const hal::boolean ownedExecutableDifferent =
        static_cast<hal::boolean>(
            ownedExecutable.find('/') != string::npos);
    if (ownedExecutableDifferent)
    {
        return isExecutableFile(ownedExecutable);
    }

    const cstring pathEnvironment = std::getenv("PATH");
    if (pathEnvironment == nullptr)
    {
        return false;
    }

    const string searchPath{pathEnvironment};
    size pathStart{};
    const hal::boolean pathSearchRequested{true};
    while (pathSearchRequested)
    {
        const hal::boolean pathEntryAvailable =
            static_cast<hal::boolean>(
                pathStart <= searchPath.size());
        if (pathEntryAvailable == false)
        {
            break;
        }
        const size separator = searchPath.find(':', pathStart);
        const size pathLength = (separator == string::npos) ?
            searchPath.size() - pathStart : separator - pathStart;
        string directory = searchPath.substr(pathStart, pathLength);
        const hal::boolean directoryEmpty =
            static_cast<hal::boolean>(
                directory.empty());
        if (directoryEmpty)
        {
            directory = ".";
        }
        string candidate = directory;
        candidate += '/';
        candidate += ownedExecutable;
        const hal::boolean executableFileMatched =
            static_cast<hal::boolean>(
                isExecutableFile(candidate));
        if (executableFileMatched)
        {
            return true;
        }
        if (separator == string::npos)
        {
            break;
        }
        pathStart = separator + 1U;
    }
    return false;
}

/**
 * @brief Queries the first IPv4 address assigned to an interface.
 *
 * @param[in] interfaceName
 * Exact Linux interface name.
 *
 * @return
 * Owned dotted-decimal IPv4 text, or an empty string when unavailable.
 */
string XWalkUtilsLinux::interfaceIpv4(stringview interfaceName) const
{
    ifaddrs* interfaceList{nullptr};
    const hal::boolean interfaceListDifferent =
        static_cast<hal::boolean>(
            ::getifaddrs(&interfaceList) != 0);
    if (interfaceListDifferent)
    {
        return {};
    }

    string address;
    for (const ifaddrs* entry = interfaceList; entry != nullptr; entry = entry->ifa_next)
    {
        if ((entry->ifa_addr == nullptr) || (entry->ifa_name == nullptr) ||
            (entry->ifa_addr->sa_family != AF_INET) || (interfaceName != entry->ifa_name))
        {
            continue;
        }

        const sockaddr_in* const ipv4Address =
            reinterpret_cast<const sockaddr_in*>(entry->ifa_addr);
        fixedarray<char, INET_ADDRSTRLEN> text{};
        const hal::boolean ipv4AddressSinAddrTextDifferent =
            static_cast<hal::boolean>(
                ::inet_ntop(AF_INET, &ipv4Address->sin_addr, text.data(), text.size()) != nullptr);
        if (ipv4AddressSinAddrTextDifferent)
        {
            address = text.data();
            break;
        }
    }
    ::freeifaddrs(interfaceList);
    return address;
}

/**
 * @brief Returns the effective application username.
 *
 * @return
 * `SUDO_USER`, then `LOGNAME`, then the effective-user database name, or an
 * empty string when none is available.
 */
string XWalkUtilsLinux::effectiveUsername() const
{
    const cstring sudoUser = std::getenv("SUDO_USER");
    if ((sudoUser != nullptr) && (sudoUser[0U] != '\0'))
    {
        return sudoUser;
    }

    const cstring loginName = std::getenv("LOGNAME");
    if ((loginName != nullptr) && (loginName[0U] != '\0'))
    {
        return loginName;
    }

    const passwd* const userInformation = ::getpwuid(::geteuid());
    if ((userInformation != nullptr) && (userInformation->pw_name != nullptr))
    {
        return userInformation->pw_name;
    }
    return {};
}

/**
 * @brief Redirects standard error to `/dev/null`.
 *
 * @return
 * Owned duplicate of the prior standard-error descriptor.
 *
 * @throws std::runtime_error
 * If the null device or descriptor operations fail.
 */
int32 XWalkUtilsLinux::redirectStandardError() const
{
    const int32 nullDescriptor = ::open("/dev/null", O_WRONLY | O_CLOEXEC);
    if (nullDescriptor < 0)
    {
        XHAL_THROW_RUNTIME_ERROR("Linux utility could not open the null device");
    }

    const int32 restoreToken = ::dup(STDERR_FILENO);
    if (restoreToken < 0)
    {
        static_cast<void>(::close(nullDescriptor));
        XHAL_THROW_RUNTIME_ERROR("Linux utility could not duplicate standard error");
    }

    const hal::boolean stderrRedirectFailed =
        static_cast<hal::boolean>(
            ::dup2(nullDescriptor, STDERR_FILENO) < 0);
    if (stderrRedirectFailed)
    {
        static_cast<void>(::close(restoreToken));
        static_cast<void>(::close(nullDescriptor));
        XHAL_THROW_RUNTIME_ERROR("Linux utility could not redirect standard error");
    }
    static_cast<void>(::close(nullDescriptor));
    return restoreToken;
}

/**
 * @brief Restores standard error and closes the restore token.
 *
 * @param[in] restoreToken
 * Descriptor returned by `redirectStandardError()`.
 *
 * @note
 * Descriptor errors cannot be reported from the guard destructor.
 */
void XWalkUtilsLinux::restoreStandardError(int32 restoreToken) const noexcept
{
    if (restoreToken >= 0)
    {
        static_cast<void>(::dup2(restoreToken, STDERR_FILENO));
        static_cast<void>(::close(restoreToken));
    }
}

} /* namespace xwalk::hal */
