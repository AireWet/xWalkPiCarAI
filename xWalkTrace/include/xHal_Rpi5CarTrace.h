/******************************************************************************
 * @file        xHal_Rpi5CarTrace.h
 * @brief       Declares filtered xWalk trace recording and public trace macros.
 *
 * @details
 * Provides compatibility severity methods and process-wide HAL and Controller
 * macros with UID, priority, source-location, timestamp, and elapsed-time data.
 *
 * @project     xWalk Firmware
 * @module      xWalkTrace
 *
 * @author      Joxy John
 * @date        2026-08-09
 * @version     2.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_TRACE_H
#define XHAL_RPI5CAR_TRACE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarTraceTypes.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <type_traits>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkTrace
 * @brief Owns trace filtering, configuration, formatting, and file output.
 *
 * @details
 * Tagged traces use four independently configured priorities and per-UID XML
 * flags. Warnings, errors, and assertions bypass tagged-trace filtering. One
 * process-wide instance serves the public macros. Explicit instances preserve
 * the original severity methods and injected callback contract.
 */
class XWalkTrace final
{
    private:
        /** @brief Non-owning context passed to the compatibility callback. */
        contextpointer outputContextPointer;

        /** @brief Non-null synchronous compatibility callback. */
        traceoutputcallback outputCallback;

        /** @brief Append-only output stream protected by `traceMutex`. */
        mutable outputfilestream logFile;

        /** @brief Priority enable flags indexed from highest priority zero. */
        fixedarray<boolean, XHAL_RPI5CAR_TRACE_PRIORITY_COUNT> priorityEnabledValues;

        /** @brief Individual UID enable flags loaded once from XML. */
        orderedmap<string, boolean> traceEnabledValues;

        /** @brief XML path used for boot-time persistent trace control. */
        filesystempath configurationPathValue;

        /** @brief Monotonic reference point shared by all entries from this instance. */
        steadytimestamp startTime;

        /** @brief Highest compatibility severity accepted by the legacy methods. */
        XWalkTraceLevel levelValue;

        /** @brief Serializes configuration lookup, timing capture, and file writes. */
        mutable mutexhandle traceMutex;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /** @brief Returns the process-wide macro trace instance. */
        static XWalkTrace& globalInstance();

        /** @brief Discards compatibility callback records from the global instance. */
        static void discardOutput(contextpointer context, XWalkTraceLevel level,
            stringview message) noexcept;

        /** @brief Converts and validates one numeric compatibility level. */
        static XWalkTraceLevel parseLevel(uint8 level);

        /** @brief Converts and validates one lowercase compatibility level name. */
        static XWalkTraceLevel parseLevel(stringview levelName);

        /** @brief Validates one typed compatibility level. */
        static XWalkTraceLevel validateLevel(XWalkTraceLevel level);

        /** @brief Returns the stable lowercase name of one compatibility level. */
        static stringview nameForLevel(XWalkTraceLevel level) noexcept;

        /** @brief Returns a basename without exposing a build-machine path. */
        static string sanitizedSourceName(stringview sourceFile);

        /** @brief Validates `RPI.<digits>` or `CTRL.<digits>`, including leading zeros. */
        static boolean isValidUid(stringview uid) noexcept;

        /** @brief Persists one known UID flag and updates the in-memory lookup. */
        boolean setTraceEnabled(stringview uid, boolean enabled);

        /** @brief Opens the selected log and loads priority and UID flags once. */
        void initialize(const filesystempath& configurationPath,
            const filesystempath& logPath);

        /** @brief Loads XML flags or retains safe disabled defaults on failure. */
        void loadConfiguration(const filesystempath& configurationPath);

        /** @brief Reports configuration failure directly to the initialized log. */
        void writeConfigurationDiagnostic(stringview category, stringview message);

        /** @brief Reports whether one compatibility severity passes its threshold. */
        boolean accepts(XWalkTraceLevel level) const noexcept;

        /** @brief Appends one completely formatted record while holding the mutex. */
        void writeRecordLocked(stringview component, stringview category,
            stringview uid, stringview sourceFile, uint32 sourceLine,
            stringview message) const;

        /** @brief Writes one compatibility record after threshold filtering. */
        void write(XWalkTraceLevel level, stringview message) const;

        /** @brief Reports a compatibility level change when debug is accepted. */
        void reportLevelChange() const;

        /** @brief Formats one validated printf-style message. */
        template<typename... FormatArguments>
        static string formatMessage(cstring format, FormatArguments... arguments)
        {
            if (format == nullptr)
            {
                XHAL_THROW_INVALID_ARGUMENT("Trace format must not be null");
            }

            if constexpr (sizeof...(FormatArguments) == 0U)
            {
                return string(format);
            }
            else
            {
                const int32 requiredLength =
                    static_cast<int32>(std::snprintf(nullptr, 0, format, arguments...));
                if (requiredLength < 0)
                {
                    XHAL_THROW_RUNTIME_ERROR("Trace message formatting failed");
                }

                const size outputLength = static_cast<size>(requiredLength);
                string message(outputLength + 1U, '\0');
                const int32 writtenLength = static_cast<int32>(
                    std::snprintf(message.data(), message.size(), format, arguments...));
                if (writtenLength != requiredLength)
                {
                    XHAL_THROW_RUNTIME_ERROR("Trace message formatting was incomplete");
                }
                message.resize(outputLength);
                return message;
            }
        }

        /** @brief Writes one tagged record after rechecking XML configuration. */
        void writeTagged(uint8 priority, stringview component, stringview uid,
            stringview sourceFile, uint32 sourceLine, stringview message);

        /** @brief Writes one untagged warning, error, or assertion record. */
        void writeCategory(stringview component, stringview category,
            stringview sourceFile, uint32 sourceLine, stringview message);

    public:
        /**
         * @brief Constructs a compatibility trace with the generated XML configuration.
         * @param[in,out] outputContext Optional non-owning callback context.
         * @param[in] output Non-null synchronous output callback.
         * @param[in] level Initial critical-through-debug threshold.
         * @throws std::invalid_argument If `output` is null or `level` is invalid.
         * @throws filesystemerror If the log directory cannot be created.
         * @throws std::runtime_error If the log file cannot be opened.
         */
        explicit XWalkTrace(contextpointer outputContext, traceoutputcallback output,
            XWalkTraceLevel level = XWalkTraceLevel::Warning);

        /**
         * @brief Constructs a compatibility trace from a numeric severity.
         * @param[in,out] outputContext Optional non-owning callback context.
         * @param[in] output Non-null synchronous output callback.
         * @param[in] level Initial severity number from zero through four.
         */
        XWalkTrace(contextpointer outputContext, traceoutputcallback output, uint8 level);

        /**
         * @brief Constructs a compatibility trace from a lowercase severity name.
         * @param[in,out] outputContext Optional non-owning callback context.
         * @param[in] output Non-null synchronous output callback.
         * @param[in] levelName Initial critical-through-debug name.
         */
        XWalkTrace(contextpointer outputContext, traceoutputcallback output,
            stringview levelName);

        /** @brief Closes the owned log stream. */
        ~XWalkTrace();

        XWalkTrace(const XWalkTrace&) = delete;
        XWalkTrace& operator=(const XWalkTrace&) = delete;
        XWalkTrace(XWalkTrace&&) = delete;
        XWalkTrace& operator=(XWalkTrace&&) = delete;

        /**
         * @brief Reconfigures the process-wide macro instance for an application or test.
         * @param[in] configurationPath XML configuration loaded once.
         * @param[in] logPath Append-only log destination.
         */
        static void configureGlobal(const filesystempath& configurationPath,
            const filesystempath& logPath);

        /**
         * @brief Enables one existing UID in XML during application boot.
         * @param[in] uid Complete `RPI.<digits>` or `CTRL.<digits>` identifier.
         * @return `true` after XML and memory are updated; otherwise `false`.
         */
        static boolean enableGlobalTrace(stringview uid);

        /**
         * @brief Disables one existing UID in XML during application boot.
         * @param[in] uid Complete `RPI.<digits>` or `CTRL.<digits>` identifier.
         * @return `true` after XML and memory are updated; otherwise `false`.
         */
        static boolean disableGlobalTrace(stringview uid);

        /**
         * @brief Tests both priority and individual UID flags without formatting.
         * @param[in] priority Priority from zero through three.
         * @param[in] uid Complete tagged identifier.
         * @return `true` only when both flags are enabled.
         */
        static boolean globalTraceIsEnabled(uint8 priority, stringview uid);

        /**
         * @brief Formats and writes one enabled process-wide tagged record.
         * @param[in] priority Priority from zero through three.
         * @param[in] component `HAL` or `CTRL` label.
         * @param[in] uid Complete scanner-validated identifier.
         * @param[in] sourceFile Compiler-provided caller path.
         * @param[in] sourceLine One-based macro invocation line.
         * @param[in] format Printf-style message format.
         * @param[in] arguments Values consumed by `format`.
         */
        template<typename... FormatArguments>
        static void globalWriteTagged(uint8 priority, stringview component,
            stringview uid, stringview sourceFile, uint32 sourceLine,
            cstring format, FormatArguments... arguments)
        {
            globalInstance().writeTagged(priority, component, uid, sourceFile,
                sourceLine, formatMessage(format, arguments...));
        }

        /**
         * @brief Formats and writes one process-wide unfiltered category record.
         * @param[in] component `HAL`, `CTRL`, or `TRACE` label.
         * @param[in] category `WARNING`, `ERROR`, or `VERBOSE`.
         * @param[in] sourceFile Compiler-provided caller path.
         * @param[in] sourceLine One-based macro invocation line.
         * @param[in] format Printf-style message format.
         * @param[in] arguments Values consumed by `format`.
         */
        template<typename... FormatArguments>
        static void globalWriteCategory(stringview component, stringview category,
            stringview sourceFile, uint32 sourceLine, cstring format,
            FormatArguments... arguments)
        {
            globalInstance().writeCategory(component, category, sourceFile,
                sourceLine, formatMessage(format, arguments...));
        }

        /**
         * @brief Writes one numeric process-wide assertion signal record.
         * @param[in] component `HAL` or `CTRL` label.
         * @param[in] signalNumber Application-defined assertion signal.
         * @param[in] sourceFile Compiler-provided caller path.
         * @param[in] sourceLine One-based macro invocation line.
         */
        static void globalWriteAssertion(stringview component, int32 signalNumber,
            stringview sourceFile, uint32 sourceLine);

        /**
         * @brief Selects a typed compatibility severity threshold.
         * @param[in] level Valid critical-through-debug severity.
         * @throws std::out_of_range If `level` is invalid.
         */
        void setLevel(XWalkTraceLevel level);

        /**
         * @brief Selects a numeric compatibility severity threshold.
         * @param[in] level Severity number from zero through four.
         * @throws std::out_of_range If `level` exceeds four.
         */
        void setLevel(uint8 level);

        /**
         * @brief Selects a textual compatibility severity threshold.
         * @param[in] levelName Supported lowercase severity name.
         * @throws std::invalid_argument If `levelName` is unsupported.
         */
        void setLevel(stringview levelName);

        /**
         * @brief Returns the configured compatibility threshold.
         * @return Current critical-through-debug threshold.
         */
        XWalkTraceLevel level() const noexcept;

        /**
         * @brief Returns the lowercase compatibility threshold name.
         * @return Static non-owning current threshold name.
         */
        stringview levelName() const noexcept;

        /**
         * @brief Emits a compatibility critical record when enabled.
         * @param[in] message Non-owning text consumed synchronously.
         */
        void critical(stringview message) const;

        /**
         * @brief Emits a compatibility error record when enabled.
         * @param[in] message Non-owning text consumed synchronously.
         */
        void error(stringview message) const;

        /**
         * @brief Emits a compatibility warning record when enabled.
         * @param[in] message Non-owning text consumed synchronously.
         */
        void warning(stringview message) const;

        /**
         * @brief Emits a compatibility informational record when enabled.
         * @param[in] message Non-owning text consumed synchronously.
         */
        void info(stringview message) const;

        /**
         * @brief Emits a compatibility debug record when enabled.
         * @param[in] message Non-owning text consumed synchronously.
         */
        void debug(stringview message) const;
};

} /* namespace xwalk::hal */

/******************************************************************************
 * Public trace macros
 ******************************************************************************/

/** @brief Stringifies one UID token sequence without compiling it as C++ syntax. */
#define XWALK_STRINGIFY_IMPL(VALUE) #VALUE
/** @brief Expands and safely stringifies one UID token sequence. */
#define XWALK_STRINGIFY(VALUE) XWALK_STRINGIFY_IMPL(VALUE)

/**
 * @brief Implements filtered tagged-trace macros.
 * @details Checks priority and UID flags before evaluating formatting arguments.
 */
#define XWALK_TRACE_UID(COMPONENT, PRIORITY, UID, ...) \
    do \
    { \
        const xwalk::hal::boolean xwalkTraceEnabled = \
            xwalk::hal::XWalkTrace::globalTraceIsEnabled( \
                static_cast<xwalk::hal::uint8>(PRIORITY), XWALK_STRINGIFY(UID)); \
        if (xwalkTraceEnabled) \
        { \
            xwalk::hal::XWalkTrace::globalWriteTagged( \
                static_cast<xwalk::hal::uint8>(PRIORITY), COMPONENT, \
                XWALK_STRINGIFY(UID), __FILE__, static_cast<xwalk::hal::uint32>(__LINE__), \
                __VA_ARGS__); \
        } \
    } while (false)

/** @brief Emits one highest-priority HAL `RPI.<number>` trace when enabled. */
#define XWALK_HAL_TRACE_UID0(UID, ...) XWALK_TRACE_UID("HAL", 0U, UID, __VA_ARGS__)
/** @brief Emits one priority-one HAL `RPI.<number>` trace when enabled. */
#define XWALK_HAL_TRACE_UID1(UID, ...) XWALK_TRACE_UID("HAL", 1U, UID, __VA_ARGS__)
/** @brief Emits one priority-two HAL `RPI.<number>` trace when enabled. */
#define XWALK_HAL_TRACE_UID2(UID, ...) XWALK_TRACE_UID("HAL", 2U, UID, __VA_ARGS__)
/** @brief Emits one lowest-priority HAL `RPI.<number>` trace when enabled. */
#define XWALK_HAL_TRACE_UID3(UID, ...) XWALK_TRACE_UID("HAL", 3U, UID, __VA_ARGS__)

/** @brief Emits one highest-priority Controller `CTRL.<number>` trace when enabled. */
#define XWALK_CTRL_TRACE_UID0(UID, ...) XWALK_TRACE_UID("CTRL", 0U, UID, __VA_ARGS__)
/** @brief Emits one priority-one Controller `CTRL.<number>` trace when enabled. */
#define XWALK_CTRL_TRACE_UID1(UID, ...) XWALK_TRACE_UID("CTRL", 1U, UID, __VA_ARGS__)
/** @brief Emits one priority-two Controller `CTRL.<number>` trace when enabled. */
#define XWALK_CTRL_TRACE_UID2(UID, ...) XWALK_TRACE_UID("CTRL", 2U, UID, __VA_ARGS__)
/** @brief Emits one lowest-priority Controller `CTRL.<number>` trace when enabled. */
#define XWALK_CTRL_TRACE_UID3(UID, ...) XWALK_TRACE_UID("CTRL", 3U, UID, __VA_ARGS__)

/** @brief Implements one unfiltered warning or error macro with caller location. */
#define XWALK_TRACE_CATEGORY(COMPONENT, CATEGORY, ...) \
    do \
    { \
        xwalk::hal::XWalkTrace::globalWriteCategory(COMPONENT, CATEGORY, __FILE__, \
            static_cast<xwalk::hal::uint32>(__LINE__), __VA_ARGS__); \
    } while (false)

/** @brief Emits one untagged HAL warning. */
#define XWALK_HAL_WARNINGS(...) XWALK_TRACE_CATEGORY("HAL", "WARNING", __VA_ARGS__)
/** @brief Emits one untagged HAL error. */
#define XWALK_HAL_ERROR(...) XWALK_TRACE_CATEGORY("HAL", "ERROR", __VA_ARGS__)
/** @brief Emits one untagged Controller warning. */
#define XWALK_CTRL_WARNINGS(...) XWALK_TRACE_CATEGORY("CTRL", "WARNING", __VA_ARGS__)
/** @brief Emits one untagged Controller error. */
#define XWALK_CTRL_ERROR(...) XWALK_TRACE_CATEGORY("CTRL", "ERROR", __VA_ARGS__)

/**
 * @brief Always emits one untagged verbose record regardless of XML filtering.
 * @details Accepts one printf-style format followed by optional formatting arguments.
 */
#define XWALK_VERBOSE(...) XWALK_TRACE_CATEGORY("TRACE", "VERBOSE", __VA_ARGS__)

/** @brief Implements compile-time type checking and logging for assertion signals. */
#define XWALK_TRACE_ASSERT(COMPONENT, SIGNAL_NUMBER) \
    do \
    { \
        using XWalkAssertionSignalType = typename std::remove_cv< \
            typename std::remove_reference<decltype(SIGNAL_NUMBER)>::type>::type; \
        static_assert(std::is_integral<XWalkAssertionSignalType>::value, \
            "xWalk assertion signal must be numeric"); \
        xwalk::hal::XWalkTrace::globalWriteAssertion(COMPONENT, \
            static_cast<xwalk::hal::int32>(SIGNAL_NUMBER), __FILE__, \
            static_cast<xwalk::hal::uint32>(__LINE__)); \
    } while (false)

/** @brief Emits one numeric HAL assertion signal without raising an OS signal. */
#define XWALK_HAL_ASSERT(SIGNAL_NUMBER) XWALK_TRACE_ASSERT("HAL", SIGNAL_NUMBER)
/** @brief Emits one numeric Controller assertion signal without raising an OS signal. */
#define XWALK_CTRL_ASSERT(SIGNAL_NUMBER) XWALK_TRACE_ASSERT("CTRL", SIGNAL_NUMBER)

#endif /* XHAL_RPI5CAR_TRACE_H */
