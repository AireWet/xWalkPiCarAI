/******************************************************************************
 * @file        xHal_Rpi5CarConfig.h
 * @brief       Declares the section-aware xWalk configuration store.
 *
 * @details
 * Defines validated in-memory configuration editing with explicit reload and
 * atomic persistence while preserving comments and unrelated file content.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig
 *
 * @author      Joxy John
 * @date        2026-07-30
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_CONFIG_H
#define XHAL_RPI5CAR_CONFIG_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarConfigTypes.h"

#include <filesystem>

/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkConfig
 * @brief Maintains a section-aware human-readable configuration file.
 *
 * @details
 * Loads named sections into owned memory, supports explicit mutation and reload,
 * and commits through a same-directory replacement file. Operations through one
 * instance are serialized; separate instances require external synchronization.
 */
class XWalkConfig
{
    private:
        /**************************************************************************
         * Private data members
         **************************************************************************/

        /** @brief Owned path identifying the persistent configuration file. */
        filesystempath filePathValue;

        /** @brief In-memory sections changed by `read`, `get`, `set`, or `setSection`. */
        configsections sectionsValue;

        /** @brief Protects the mutable memory image and file operations for this instance. */
        mutable mutexhandle mutexObject;

    protected:
        /**************************************************************************
         * Protected member functions
         **************************************************************************/

        /**
         * @brief Validates a named section.
         *
         * @param[in] sectionName
         * Non-empty trimmed section name without brackets or line terminators.
         *
         * @throws std::invalid_argument
         * If the name cannot be represented unambiguously.
         */
        static void validateSectionName(stringview sectionName);

        /**
         * @brief Validates a configuration option name.
         *
         * @param[in] optionName
         * Non-empty trimmed name without `=`, brackets, or line terminators.
         *
         * @throws std::invalid_argument
         * If the name cannot be represented unambiguously.
         */
        static void validateOptionName(stringview optionName);

        /**
         * @brief Validates a single-line configuration value.
         *
         * @param[in] value
         * Value that must not contain carriage return or line feed.
         *
         * @throws std::invalid_argument
         * If the value spans more than one line.
         */
        static void validateValue(stringview value);

        /**
         * @brief Removes leading and trailing ASCII spaces and horizontal tabs.
         *
         * @param[in] text
         * Character sequence to normalize.
         *
         * @return
         * Owned normalized text.
         */
        static string trim(stringview text);

        /**
         * @brief Extracts and validates a section header.
         *
         * @param[in] line
         * Trimmed line beginning with an opening bracket.
         *
         * @return
         * Validated section name without brackets.
         *
         * @throws std::runtime_error
         * If the line is not a complete unambiguous section header.
         */
        static string parseSectionHeader(stringview line);

        /**
         * @brief Parses configuration lines into an owned section map.
         *
         * @param[in] lines
         * Ordered file content without newline characters.
         *
         * @return
         * Parsed sections, including the empty-name default section.
         *
         * @throws std::runtime_error
         * If a section header or option name is malformed.
         */
        static configsections parseSections(const stringvector& lines);

        /**
         * @brief Appends options not already represented by existing file lines.
         *
         * @param[in,out] outputLines
         * Merged output receiving serialized options.
         *
         * @param[in,out] remainingSections
         * Writable copy whose completed section is removed.
         *
         * @param[in] sectionName
         * Section whose remaining options are appended.
         */
        static void appendMissingOptions(stringvector& outputLines,
            configsections& remainingSections, const string& sectionName);

        /**
         * @brief Merges the current memory image with existing file lines.
         *
         * @param[in] originalLines
         * Existing ordered file content without newline characters.
         *
         * @return
         * Updated lines retaining comments, blank lines, and unknown content.
         *
         * @throws std::runtime_error
         * If an existing section header is malformed.
         */
        stringvector mergeLines(const stringvector& originalLines) const;

        /**
         * @brief Reads all configuration lines without newline characters.
         *
         * @return
         * Lines in their original order.
         *
         * @throws std::runtime_error
         * If the file cannot be opened or completely read.
         */
        stringvector readLines() const;

        /**
         * @brief Atomically replaces the configuration file with supplied lines.
         *
         * @param[in] lines
         * Complete ordered file content without newline characters.
         *
         * @post
         * A successful call leaves the configuration path containing `lines`.
         *
         * @throws std::runtime_error
         * If creation, writing, permission copying, or replacement fails.
         */
        void writeLines(const stringvector& lines) const;

        /**
         * @brief Creates the parent directory and initial file when absent.
         *
         * @param[in] description
         * Optional text written as `# ` comment lines only when creating the file.
         *
         * @post
         * The configured path identifies a regular file.
         *
         * @throws std::runtime_error
         * If the path is invalid or initial content cannot be written.
         *
         * @throws filesystemerror
         * If filesystem inspection or parent-directory creation fails.
         */
        void ensureFileExists(stringview description) const;

    public:
        /**************************************************************************
         * Public constructors and destructor
         **************************************************************************/

        /**
         * @brief Constructs and loads a section-aware configuration file.
         *
         * @param[in] filePath
         * Non-empty filesystem path copied into the object.
         *
         * @param[in] description
         * Optional multiline description used only when a new file is created.
         *
         * @post
         * The file exists and its current sections are loaded into memory.
         *
         * @throws std::invalid_argument
         * If `filePath` is empty.
         *
         * @throws filesystemerror
         * If filesystem inspection or parent-directory creation fails.
         *
         * @throws std::runtime_error
         * If file creation, reading, or configuration parsing fails.
         */
        explicit XWalkConfig(stringview filePath, stringview description = {});

        /** @brief Destroys the object without deleting or implicitly writing its file. */
        ~XWalkConfig();

        /**************************************************************************
         * Public special member functions
         **************************************************************************/

        /** @brief Disables move construction because the object contains a mutex. */
        XWalkConfig(XWalkConfig&&) = delete;
        /** @brief Disables copying because the object contains a mutex. */
        XWalkConfig(const XWalkConfig&) = delete;
        /** @brief Disables move assignment because the object contains a mutex. */
        XWalkConfig& operator=(XWalkConfig&&) = delete;
        /** @brief Disables copy assignment because the object contains a mutex. */
        XWalkConfig& operator=(const XWalkConfig&) = delete;

        /**************************************************************************
         * Public member functions
         **************************************************************************/

        /**
         * @brief Reloads all configuration sections from the file.
         *
         * @return
         * Owned copy of the newly loaded section map.
         *
         * @post
         * Unsaved in-memory changes are replaced by current file content.
         *
         * @throws filesystemerror
         * If recreation of a removed file or parent directory fails.
         *
         * @throws std::runtime_error
         * If file creation, reading, or configuration parsing fails.
         */
        configsections read();

        /**
         * @brief Persists the current in-memory configuration.
         *
         * @post
         * Existing entries are updated and newly added entries are serialized.
         *
         * @throws filesystemerror
         * If recreation of a removed file or parent directory fails.
         *
         * @throws std::runtime_error
         * If file reading, parsing, writing, or replacement fails.
         */
        void write();

        /**
         * @brief Retrieves one option and inserts its default when absent.
         *
         * @param[in] sectionName
         * Empty for the default section or a valid named section.
         *
         * @param[in] optionName
         * Valid option name to retrieve.
         *
         * @param[in] defaultValue
         * Single-line value copied into memory when the option is absent.
         *
         * @return
         * Stored value or an owned copy of `defaultValue` when newly inserted.
         *
         * @throws std::out_of_range
         * If `sectionName` is not loaded.
         *
         * @throws std::invalid_argument
         * If a name or `defaultValue` cannot be represented safely.
         */
        string get(stringview sectionName, stringview optionName, stringview defaultValue = {});

        /**
         * @brief Changes one option in an existing in-memory section.
         *
         * @param[in] sectionName
         * Empty for the default section or a valid named section.
         *
         * @param[in] optionName
         * Valid option name to add or replace.
         *
         * @param[in] value
         * Single-line value copied into memory.
         *
         * @throws std::out_of_range
         * If `sectionName` is not loaded.
         *
         * @throws std::invalid_argument
         * If a name or `value` cannot be represented safely.
         */
        void set(stringview sectionName, stringview optionName, stringview value);

        /**
         * @brief Returns an owned copy of one loaded section.
         *
         * @param[in] sectionName
         * Empty for the default section or a valid named section.
         *
         * @return
         * Ordered option-value pairs in the requested section.
         *
         * @throws std::out_of_range
         * If `sectionName` is not loaded.
         *
         * @throws std::invalid_argument
         * If a non-empty `sectionName` is invalid.
         */
        configsection section(stringview sectionName) const;

        /**
         * @brief Adds or replaces one complete in-memory section.
         *
         * @param[in] sectionName
         * Empty for the default section or a valid named section.
         *
         * @param[in] sectionValue
         * Valid option-value pairs copied into memory.
         *
         * @throws std::invalid_argument
         * If the section, an option, or a value cannot be represented safely.
         */
        void setSection(stringview sectionName, const configsection& sectionValue);

        /**
         * @brief Returns the owned configuration-file path as a string.
         *
         * @return
         * Platform-native path representation copied into an owned string.
         */
        string filePath() const;
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_CONFIG_H */
