/******************************************************************************
 * @file        xHal_Rpi5CarTrace.h
 * @brief       Declares filtered xWalk trace recording and public trace macros.
 *
 * @details
 * Provides compatibility severity methods and process-wide component macros
 * with UID, priority, source-location, timestamp, and elapsed-time data.
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

#include "xHal_Rpi5CarErrorSignals.h"
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
     * @brief Owns trace filtering, configuration, formatting, terminal, and file
     * output.
     *
     * @details
     * Tagged traces use one ordered global, module, and per-UID runtime registry.
     * Warnings, errors, and assertions bypass tagged-trace filtering. One
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

            /** @brief Persistent fallback state for every present and future trace. */
            boolean globalTraceEnabledValue{false};

            /** @brief Explicit module states indexed by canonical module name. */
            orderedmap<string, boolean> moduleEnabledValues;

            /** @brief Individual UID overrides loaded once from XML. */
            orderedmap<string, boolean> traceEnabledValues;

            /** @brief Scanner-generated source locations indexed by complete UID. */
            orderedmap<string, XWalkTraceSourceLocation> traceSourceLocations;

            /** @brief Mutable generated trace catalogue path loaded at startup. */
            filesystempath configurationPathValue;

            /** @brief Active append-only log path selected during initialization. */
            filesystempath logPathValue;

            /** @brief Most recent actionable runtime trace-configuration error. */
            string traceConfigurationErrorValue;

            /** @brief Monotonic reference point shared by all entries from this instance.
             */
            steadytimestamp startTime;

            /** @brief Highest compatibility severity accepted by the legacy methods. */
            XWalkTraceLevel levelValue;

            /** @brief Serializes configuration lookup, timing capture, and file writes.
             */
            mutable mutexhandle traceMutex;

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /** @brief Returns the process-wide macro trace instance. */
            static XWalkTrace& globalInstance(const filesystempath* configurationPath = nullptr,
                                              const filesystempath* logPath = nullptr);

            /** @brief Discards compatibility callback records from the global instance.
             */
            static void discardOutput(contextpointer context, XWalkTraceLevel level, stringview message) noexcept;

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

            /** @brief Validates one registered module tag followed by a numeric ID. */
            static boolean isValidUid(stringview uid) noexcept;

            /** @brief Applies one known UID state to the in-memory lookup. */
            boolean setTraceEnabled(stringview uid, boolean enabled);

            /** @brief Applies one module state and clears earlier tag overrides. */
            boolean setModuleTracesEnabled(stringview module, boolean enabled);

            /** @brief Applies one global state and clears every earlier override. */
            boolean setAllTracesEnabled(boolean enabled);

            /** @brief Resolves one known UID through its tag, module, and global state.
             */
            boolean traceIsEnabled(stringview uid) const;

            /** @brief Atomically writes effective runtime states to the configured XML.
             */
            boolean persistConfiguration();

            /** @brief Loads and atomically applies one JSON trace configuration. */
            boolean loadJsonConfiguration(const filesystempath& configurationPath);

            /** @brief Opens the selected log and loads catalogue IDs and states once. */
            void initialize(const filesystempath& configurationPath, const filesystempath& logPath);

            /** @brief Loads XML catalogue entries and persistent runtime states. */
            void loadConfiguration(const filesystempath& configurationPath);

            /** @brief Reports configuration failure directly to the initialized log. */
            void writeConfigurationDiagnostic(stringview category, stringview message);

            /** @brief Reports whether one compatibility severity passes its threshold. */
            boolean accepts(XWalkTraceLevel level) const noexcept;

            /** @brief Emits one formatted record to the terminal and log while locked. */
            void writeRecordLocked(stringview component,
                                   stringview category,
                                   stringview uid,
                                   stringview sourceFile,
                                   uint32 sourceLine,
                                   stringview message) const;

            /** @brief Writes one compatibility record after threshold filtering. */
            void write(XWalkTraceLevel level, stringview message) const;

            /** @brief Reports a compatibility level change when debug is accepted. */
            void reportLevelChange() const;

            /** @brief Constructs a trace with explicit initial catalogue and log paths.
             */
            XWalkTrace(contextpointer outputContext,
                       traceoutputcallback output,
                       XWalkTraceLevel level,
                       const filesystempath& configurationPath,
                       const filesystempath& logPath);

            /** @brief Formats one validated printf-style message. */
            static string formatMessage(stringview message)
            {
                return string(message);
            }

            /** @brief Formats one validated printf-style message and its arguments. */
            template <typename... FormatArguments>
            static string formatMessage(cstring format, FormatArguments... arguments)
            {
                if (format == nullptr)
                {
                    throw invalidargument("Trace format must not be null");
                }

                if constexpr (sizeof...(FormatArguments) == 0U)
                {
                    return string(format);
                }
                else
                {
                    const int32 requiredLength = static_cast<int32>(std::snprintf(nullptr, 0, format, arguments...));
                    if (requiredLength < 0)
                    {
                        throw runtimeerror("Trace message formatting failed");
                    }

                    const size outputLength = static_cast<size>(requiredLength);
                    string message(outputLength + 1U, '\0');
                    const int32 writtenLength =
                        static_cast<int32>(std::snprintf(message.data(), message.size(), format, arguments...));
                    if (writtenLength != requiredLength)
                    {
                        throw runtimeerror("Trace message formatting was incomplete");
                    }
                    message.resize(outputLength);
                    return message;
                }
            }

            /** @brief Writes one tagged record after rechecking XML configuration. */
            void writeTagged(stringview component, stringview uid, stringview message);

            /** @brief Writes one untagged warning, error, or assertion record. */
            void writeCategory(stringview component,
                               stringview category,
                               stringview selector,
                               stringview sourceFile,
                               uint32 sourceLine,
                               stringview message);

        public:
            /**
             * @brief Constructs a compatibility trace with the generated XML
             * configuration.
             * @param[in,out] outputContext Optional non-owning callback context.
             * @param[in] output Non-null synchronous output callback.
             * @param[in] level Initial critical-through-debug threshold.
             * @throws std::invalid_argument If `output` is null or `level` is invalid.
             * @throws filesystemerror If the log directory cannot be created.
             * @throws std::runtime_error If the log file cannot be opened.
             */
            explicit XWalkTrace(contextpointer outputContext,
                                traceoutputcallback output,
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
            XWalkTrace(contextpointer outputContext, traceoutputcallback output, stringview levelName);

            /** @brief Closes the owned log stream. */
            ~XWalkTrace();

            XWalkTrace(const XWalkTrace&) = delete;
            XWalkTrace& operator=(const XWalkTrace&) = delete;
            XWalkTrace(XWalkTrace&&) = delete;
            XWalkTrace& operator=(XWalkTrace&&) = delete;

            /**
             * @brief Reconfigures the process-wide macro instance for an application or
             * test.
             * @param[in] configurationPath Mutable XML configuration loaded once.
             * @param[in] logPath Append-only log destination.
             */
            static void configureGlobal(const filesystempath& configurationPath, const filesystempath& logPath);

            /**
             * @brief Enables one existing UID in XML during application boot.
             * @param[in] uid Complete registered module trace identifier.
             * @return `true` after XML and memory are updated; otherwise `false`.
             */
            static boolean enableGlobalTrace(stringview uid);

            /**
             * @brief Disables one existing UID in XML during application boot.
             * @param[in] uid Complete registered module trace identifier.
             * @return `true` after XML and memory are updated; otherwise `false`.
             */
            static boolean disableGlobalTrace(stringview uid);

            /**
             * @brief Enables every priority and scanner-known UID in the global XML
             * configuration.
             * @return `true` after every XML and memory flag is updated; otherwise
             * `false`.
             */
            static boolean enableAllGlobalTraces();

            /**
             * @brief Disables every priority and scanner-known UID in the global XML
             * configuration.
             * @return `true` after every XML and memory flag is updated; otherwise
             * `false`.
             */
            static boolean disableAllGlobalTraces();

            /**
             * @brief Persists the disabled state for every normal trace.
             * @return `true` after XML and memory are updated; otherwise `false`.
             */
            static boolean resetGlobalTraceConfiguration();

            /**
             * @brief Applies one selector or JSON filename to the shared registry.
             * @param[in] argument Valid selector or `.json` configuration path.
             * @return `true` when the complete argument validates and is applied.
             */
            static boolean applyGlobalTraceArgument(stringview argument);

            /**
             * @brief Returns the latest runtime trace-configuration error.
             * @return Owned diagnostic text, or empty text after success.
             */
            static string globalTraceConfigurationError();

            /**
             * @brief Resolves one known UID without formatting its arguments.
             * @param[in] uid Complete tagged identifier.
             * @return `true` only when both flags are enabled.
             */
            static boolean globalTraceIsEnabled(stringview uid);

            /**
             * @brief Formats and writes one enabled process-wide tagged record.
             * @param[in] component `HAL`, `CTRL`, `RPIAGENT`, or `LIB` label.
             * @param[in] uid Complete scanner-validated identifier.
             * @param[in] format Printf-style message format.
             * @param[in] arguments Values consumed by `format`.
             */
            template <typename... FormatArguments>
            static void
            globalWriteTagged(stringview component, stringview uid, cstring format, FormatArguments... arguments)
            {
                globalInstance().writeTagged(component, uid, formatMessage(format, arguments...));
            }

            /**
             * @brief Formats and writes one process-wide unfiltered category record.
             * @param[in] component Registered module or `TRACE` label.
             * @param[in] category `WARNING`, `ERROR`, or `VERBOSE`.
             * @param[in] selector Public short error or signal selector name.
             * @param[in] sourceFile Compiler-provided caller path.
             * @param[in] sourceLine One-based macro invocation line.
             * @param[in] format Printf-style message format.
             * @param[in] arguments Values consumed by `format`.
             */
            template <typename... FormatArguments>
            static string globalWriteCategory(stringview component,
                                              stringview category,
                                              stringview selector,
                                              stringview sourceFile,
                                              uint32 sourceLine,
                                              cstring format,
                                              FormatArguments... arguments)
            {
                const string message = formatMessage(format, arguments...);
                globalInstance().writeCategory(component, category, selector, sourceFile, sourceLine, message);
                return message;
            }

            /** @brief Writes one already-formatted category message. */
            static string globalWriteCategory(stringview component,
                                              stringview category,
                                              stringview selector,
                                              stringview sourceFile,
                                              uint32 sourceLine,
                                              stringview message)
            {
                const string ownedMessage(message);
                globalInstance().writeCategory(component, category, selector, sourceFile, sourceLine, ownedMessage);
                return ownedMessage;
            }

            /**
             * @brief Writes one numeric process-wide assertion signal record.
             * @param[in] component Registered module label.
             * @param[in] signalNumber Application-defined assertion signal.
             * @param[in] sourceFile Compiler-provided caller path.
             * @param[in] sourceLine One-based macro invocation line.
             */
            static void
            globalWriteAssertion(stringview component, int32 signalNumber, stringview sourceFile, uint32 sourceLine);

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

/** @brief Stringifies one UID token sequence without compiling it as C++
 * syntax. */
#define XWALK_STRINGIFY_IMPL(VALUE) #VALUE
/** @brief Expands and safely stringifies one UID token sequence. */
#define XWALK_STRINGIFY(VALUE) XWALK_STRINGIFY_IMPL(VALUE)

/**
 * @brief Implements filtered tagged-trace macros.
 * @details Checks registry state before evaluating formatting arguments.
 */
#define XWALK_TRACE_UID(COMPONENT, UID, ...)                                                                           \
    do                                                                                                                 \
    {                                                                                                                  \
        const xwalk::hal::boolean xwalkTraceEnabled =                                                                  \
            xwalk::hal::XWalkTrace::globalTraceIsEnabled(XWALK_STRINGIFY(UID));                                        \
        if (xwalkTraceEnabled)                                                                                         \
        {                                                                                                              \
            xwalk::hal::XWalkTrace::globalWriteTagged(COMPONENT, XWALK_STRINGIFY(UID), __VA_ARGS__);                   \
        }                                                                                                              \
    } while (false)

/** @brief Declares one module trace macro with an exact formatting-argument
 * count. */
#define XWALK_HAL_TRACE_UID0(UID, FORMAT) XWALK_TRACE_UID("HAL", UID, FORMAT)
#define XWALK_HAL_TRACE_UID1(UID, FORMAT, ARG1) XWALK_TRACE_UID("HAL", UID, FORMAT, ARG1)
#define XWALK_HAL_TRACE_UID2(UID, FORMAT, ARG1, ARG2) XWALK_TRACE_UID("HAL", UID, FORMAT, ARG1, ARG2)
#define XWALK_HAL_TRACE_UID3(UID, FORMAT, ARG1, ARG2, ARG3) XWALK_TRACE_UID("HAL", UID, FORMAT, ARG1, ARG2, ARG3)
#define XWALK_HAL_TRACE_UID4(UID, FORMAT, ARG1, ARG2, ARG3, ARG4)                                                      \
    XWALK_TRACE_UID("HAL", UID, FORMAT, ARG1, ARG2, ARG3, ARG4)
#define XWALK_HAL_TRACE_UID5(UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)                                                \
    XWALK_TRACE_UID("HAL", UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)

/** @brief Declares one Controller trace macro with an exact formatting-argument
 * count. */
#define XWALK_CTRL_TRACE_UID0(UID, FORMAT) XWALK_TRACE_UID("CTRL", UID, FORMAT)
#define XWALK_CTRL_TRACE_UID1(UID, FORMAT, ARG1) XWALK_TRACE_UID("CTRL", UID, FORMAT, ARG1)
#define XWALK_CTRL_TRACE_UID2(UID, FORMAT, ARG1, ARG2) XWALK_TRACE_UID("CTRL", UID, FORMAT, ARG1, ARG2)
#define XWALK_CTRL_TRACE_UID3(UID, FORMAT, ARG1, ARG2, ARG3) XWALK_TRACE_UID("CTRL", UID, FORMAT, ARG1, ARG2, ARG3)
#define XWALK_CTRL_TRACE_UID4(UID, FORMAT, ARG1, ARG2, ARG3, ARG4)                                                     \
    XWALK_TRACE_UID("CTRL", UID, FORMAT, ARG1, ARG2, ARG3, ARG4)
#define XWALK_CTRL_TRACE_UID5(UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)                                               \
    XWALK_TRACE_UID("CTRL", UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)

/** @brief Declares one Agent trace macro with an exact formatting-argument
 * count. */
#define XWALK_RPIAGENT_TRACE_UID0(UID, FORMAT) XWALK_TRACE_UID("RPIAGENT", UID, FORMAT)
#define XWALK_RPIAGENT_TRACE_UID1(UID, FORMAT, ARG1) XWALK_TRACE_UID("RPIAGENT", UID, FORMAT, ARG1)
#define XWALK_RPIAGENT_TRACE_UID2(UID, FORMAT, ARG1, ARG2) XWALK_TRACE_UID("RPIAGENT", UID, FORMAT, ARG1, ARG2)
#define XWALK_RPIAGENT_TRACE_UID3(UID, FORMAT, ARG1, ARG2, ARG3)                                                       \
    XWALK_TRACE_UID("RPIAGENT", UID, FORMAT, ARG1, ARG2, ARG3)
#define XWALK_RPIAGENT_TRACE_UID4(UID, FORMAT, ARG1, ARG2, ARG3, ARG4)                                                 \
    XWALK_TRACE_UID("RPIAGENT", UID, FORMAT, ARG1, ARG2, ARG3, ARG4)
#define XWALK_RPIAGENT_TRACE_UID5(UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)                                           \
    XWALK_TRACE_UID("RPIAGENT", UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)

/** @brief Declares one Library trace macro with an exact formatting-argument
 * count. */
#define XWALK_LIB_TRACE_UID0(UID, FORMAT) XWALK_TRACE_UID("LIB", UID, FORMAT)
#define XWALK_LIB_TRACE_UID1(UID, FORMAT, ARG1) XWALK_TRACE_UID("LIB", UID, FORMAT, ARG1)
#define XWALK_LIB_TRACE_UID2(UID, FORMAT, ARG1, ARG2) XWALK_TRACE_UID("LIB", UID, FORMAT, ARG1, ARG2)
#define XWALK_LIB_TRACE_UID3(UID, FORMAT, ARG1, ARG2, ARG3) XWALK_TRACE_UID("LIB", UID, FORMAT, ARG1, ARG2, ARG3)
#define XWALK_LIB_TRACE_UID4(UID, FORMAT, ARG1, ARG2, ARG3, ARG4)                                                      \
    XWALK_TRACE_UID("LIB", UID, FORMAT, ARG1, ARG2, ARG3, ARG4)
#define XWALK_LIB_TRACE_UID5(UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)                                                \
    XWALK_TRACE_UID("LIB", UID, FORMAT, ARG1, ARG2, ARG3, ARG4, ARG5)

/** @brief Implements one selector-tagged warning or error with caller location.
 */
#define XWALK_TRACE_CATEGORY(COMPONENT, CATEGORY, SELECTOR, SELECTOR_NAME, ...)                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        static_cast<void>(sizeof(SELECTOR));                                                                           \
        static_cast<void>(xwalk::hal::XWalkTrace::globalWriteCategory(                                                 \
            COMPONENT, CATEGORY, SELECTOR_NAME, __FILE__, static_cast<xwalk::hal::uint32>(__LINE__), __VA_ARGS__));    \
    } while (false)

/** @brief Raises one selected C++ exception after its error record is written.
 */
#define XWALK_ERROR_ACTION_XWALK_INVAL(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_INVAL>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_RANGE(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_RANGE>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_LENGTH(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_LENGTH>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_DOMAIN(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_DOMAIN>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_LOGIC(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_LOGIC>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_RUNTIME(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_RUNTIME>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_OVERFLOW(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_OVERFLOW>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_UNDERFLOW(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_UNDERFLOW>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_SYSTEM(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_SYSTEM>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_ALLOC(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_ALLOC>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_CAST(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_CAST>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_TYPEID(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_TYPEID>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_FUNCTION(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_FUNCTION>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_OPTIONAL(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_OPTIONAL>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_VARIANT(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_VARIANT>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_WEAKPTR(MESSAGE) throw xwalk::hal::makeErrorSignal<XWALK_WEAKPTR>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_EXCEPTION(MESSAGE) static_cast<void>(MESSAGE)

/** @brief Completes signal-condition reporting without raising the signal
 * again. */
#define XWALK_ERROR_ACTION_XWALK_ABORT(MESSAGE) static_cast<void>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_FLOAT(MESSAGE) static_cast<void>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_ILL(MESSAGE) static_cast<void>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_SEGV(MESSAGE) static_cast<void>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_TERM(MESSAGE) static_cast<void>(MESSAGE)
#define XWALK_ERROR_ACTION_XWALK_INT(MESSAGE) static_cast<void>(MESSAGE)
#if defined(SIGPIPE)
    #define XWALK_ERROR_ACTION_XWALK_PIPE(MESSAGE) static_cast<void>(MESSAGE)
#endif
#if defined(SIGHUP)
    #define XWALK_ERROR_ACTION_XWALK_HANG(MESSAGE) static_cast<void>(MESSAGE)
#endif
#if defined(SIGTRAP)
    #define XWALK_ERROR_ACTION_XWALK_TRAP(MESSAGE) static_cast<void>(MESSAGE)
#endif

/** @brief Implements one formatted error and its selector-specific action. */
#define XWALK_TRACE_ERROR(COMPONENT, ACTION, SELECTOR, SELECTOR_NAME, ...)                                             \
    do                                                                                                                 \
    {                                                                                                                  \
        static_cast<void>(sizeof(SELECTOR));                                                                           \
        const xwalk::hal::string xwalkErrorMessage = xwalk::hal::XWalkTrace::globalWriteCategory(                      \
            COMPONENT, "ERROR", SELECTOR_NAME, __FILE__, static_cast<xwalk::hal::uint32>(__LINE__), __VA_ARGS__);      \
        ACTION(xwalkErrorMessage);                                                                                     \
    } while (false)

/** @brief Emits one selector-tagged variadic HAL warning. */
#define XWALK_HAL_WARNING(SELECTOR, ...) XWALK_TRACE_CATEGORY("HAL", "WARNING", SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic HAL error. */
#define XWALK_HAL_ERROR(SELECTOR, ...)                                                                                 \
    XWALK_TRACE_ERROR("HAL", XWALK_ERROR_ACTION_##SELECTOR, SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic Controller warning. */
#define XWALK_CTRL_WARNING(SELECTOR, ...) XWALK_TRACE_CATEGORY("CTRL", "WARNING", SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic Controller error. */
#define XWALK_CTRL_ERROR(SELECTOR, ...)                                                                                \
    XWALK_TRACE_ERROR("CTRL", XWALK_ERROR_ACTION_##SELECTOR, SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic Agent warning. */
#define XWALK_RPIAGENT_WARNING(SELECTOR, ...)                                                                          \
    XWALK_TRACE_CATEGORY("RPIAGENT", "WARNING", SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic Agent error. */
#define XWALK_RPIAGENT_ERROR(SELECTOR, ...)                                                                            \
    XWALK_TRACE_ERROR("RPIAGENT", XWALK_ERROR_ACTION_##SELECTOR, SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic Library warning. */
#define XWALK_LIB_WARNING(SELECTOR, ...) XWALK_TRACE_CATEGORY("LIB", "WARNING", SELECTOR, #SELECTOR, __VA_ARGS__)
/** @brief Emits one selector-tagged variadic Library error. */
#define XWALK_LIB_ERROR(SELECTOR, ...)                                                                                 \
    XWALK_TRACE_ERROR("LIB", XWALK_ERROR_ACTION_##SELECTOR, SELECTOR, #SELECTOR, __VA_ARGS__)

/**
 * @brief Preserves the legacy untagged verbose API as a disabled no-op.
 * @details Normal diagnostics must use one scanner-registered UID macro.
 */
#define XWALK_VERBOSE(...)                                                                                             \
    do                                                                                                                 \
    {                                                                                                                  \
    } while (false)

/** @brief Implements compile-time type checking and logging for assertion
 * signals. */
#define XWALK_TRACE_ASSERT(COMPONENT, SIGNAL_NUMBER)                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        using XWalkAssertionSignalType =                                                                               \
            typename std::remove_cv<typename std::remove_reference<decltype(SIGNAL_NUMBER)>::type>::type;              \
        static_assert(std::is_integral<XWalkAssertionSignalType>::value, "xWalk assertion signal must be numeric");    \
        xwalk::hal::XWalkTrace::globalWriteAssertion(COMPONENT,                                                        \
                                                     static_cast<xwalk::hal::int32>(SIGNAL_NUMBER),                    \
                                                     __FILE__,                                                         \
                                                     static_cast<xwalk::hal::uint32>(__LINE__));                       \
    } while (false)

/** @brief Emits one numeric HAL assertion signal without raising an OS signal.
 */
#define XWALK_HAL_ASSERT(SIGNAL_NUMBER) XWALK_TRACE_ASSERT("HAL", SIGNAL_NUMBER)
/** @brief Emits one numeric Controller assertion signal without raising an OS
 * signal. */
#define XWALK_CTRL_ASSERT(SIGNAL_NUMBER) XWALK_TRACE_ASSERT("CTRL", SIGNAL_NUMBER)
/** @brief Emits one numeric Agent assertion signal without raising an OS
 * signal. */
#define XWALK_RPIAGENT_ASSERT(SIGNAL_NUMBER) XWALK_TRACE_ASSERT("RPIAGENT", SIGNAL_NUMBER)
/** @brief Emits one numeric Library assertion signal without raising an OS
 * signal. */
#define XWALK_LIB_ASSERT(SIGNAL_NUMBER) XWALK_TRACE_ASSERT("LIB", SIGNAL_NUMBER)

#endif /* XHAL_RPI5CAR_TRACE_H */
