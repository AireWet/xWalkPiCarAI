/******************************************************************************
 * @file        xHal_Rpi5CarTraceFormatting.cpp
 * @brief       Formats timestamped xWalk trace records.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 * @author      Joxy John
 * @date        2026-08-09
 * @version     2.0.0
 ******************************************************************************/

#include "xHal_Rpi5CarTrace.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

    /**
     * @brief Reduces one compiler source path to a non-sensitive basename.
     *
     * @param[in] sourceFile
     * Absolute or relative compiler-provided source path.
     *
     * @return
     * Final filename component, or `unknown` when no filename is available.
     */
    string XWalkTrace::sanitizedSourceName(stringview sourceFile)
    {
        const size slashPosition = sourceFile.find_last_of("/\\");
        const boolean separatorPresent = slashPosition != stringview::npos;
        const string filename = separatorPresent ? string(sourceFile.substr(slashPosition + 1U)) : string(sourceFile);
        const boolean filenameMissing = filename.empty();
        return filenameMissing ? string("unknown") : filename;
    }

    /**
     * @brief Captures timing data and emits one synchronized terminal and log line.
     *
     * @param[in] component
     * Registered module, `TRACE`, or compatibility component label.
     *
     * @param[in] category
     * Priority or warning, error, assertion, or compatibility category.
     *
     * @param[in] uid
     * Complete tagged identifier, or an empty view for untagged records.
     *
     * @param[in] sourceFile
     * Caller source path reduced to its basename before output.
     *
     * @param[in] sourceLine
     * One-based caller source line, or zero for an internal diagnostic.
     *
     * @param[in] message
     * Fully formatted record text.
     *
     * @throws std::runtime_error
     * If UTC conversion or append-and-flush output fails.
     */
    void XWalkTrace::writeRecordLocked(stringview component,
                                       stringview category,
                                       stringview uid,
                                       stringview sourceFile,
                                       uint32 sourceLine,
                                       stringview message) const
    {
        static mutexhandle outputMutex;
        mutexlock outputLock(outputMutex);
        const std::chrono::system_clock::time_point wallTime = std::chrono::system_clock::now();
        const steadytimestamp eventTime = steadyclock::now();
        const std::time_t wallSeconds = std::chrono::system_clock::to_time_t(wallTime);
        std::tm utcTime{};
        const std::tm* convertedTime = ::gmtime_r(&wallSeconds, &utcTime);
        const boolean timestampConversionFailed = convertedTime == nullptr;
        if (timestampConversionFailed)
        {
            throw runtimeerror("Trace UTC timestamp conversion failed");
        }

        const auto wallMilliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(wallTime.time_since_epoch()) % 1000;
        const auto elapsedMicroseconds =
            std::chrono::duration_cast<std::chrono::microseconds>(eventTime - startTime).count();
        const auto elapsedSeconds = elapsedMicroseconds / 1000000;
        const auto elapsedRemainder = elapsedMicroseconds % 1000000;

        std::ostringstream record;
        record << std::put_time(&utcTime, "%Y-%m-%d %H:%M:%S") << '.' << std::setfill('0') << std::setw(3)
               << wallMilliseconds.count() << 'Z' << " [T+" << elapsedSeconds << '.' << std::setw(6) << elapsedRemainder
               << "s]" << " [" << component << "] [" << category << ']';

        const boolean uidPresent = uid.empty() == false;
        if (uidPresent)
        {
            record << " [" << uid << ']';
        }
        record << " [" << sanitizedSourceName(sourceFile) << ':' << sourceLine << "] " << message;

        const string recordText = record.str();
        std::clog << recordText << '\n';
        std::clog.flush();
        logFile << recordText << '\n';
        logFile.flush();
        const boolean logWriteFailed = !logFile;
        if (logWriteFailed)
        {
            throw runtimeerror("Trace record could not be written to the log file");
        }
    }

} /* namespace xwalk::hal */
