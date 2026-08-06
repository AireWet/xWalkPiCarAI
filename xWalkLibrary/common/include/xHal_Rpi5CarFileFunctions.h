/******************************************************************************
 * @file        xHal_Rpi5CarFileFunctions.h
 * @brief       Declares reusable filesystem operations for xWalk modules.
 *
 * @details
 * Provides the common boundary for filesystem inspection, directory creation,
 * entry removal, permission updates, and path replacement.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibraryCommon
 *
 * @author      Joxy John
 * @date        2026-07-29
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#ifndef XHAL_RPI5CAR_FILE_FUNCTIONS_H
#define XHAL_RPI5CAR_FILE_FUNCTIONS_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarTypes.h"

#include <filesystem>
#include <fstream>

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
 * Constants
 ******************************************************************************/

/** @brief Opens a character file for output and discards its previous content. */
inline constexpr fileopenmode FILE_OPEN_WRITE_TRUNCATE = std::ios::out | std::ios::trunc;
/** @brief Opens a file for binary input without character translation. */
inline constexpr fileopenmode FILE_OPEN_READ_BINARY = std::ios::in | std::ios::binary;

/** @brief Replaces the complete permission-bit value instead of adding or removing selected bits. */
inline constexpr filesystempermissionoptions FILE_PERMISSION_REPLACE =
    std::filesystem::perm_options::replace;

/******************************************************************************
 * Inline function definitions
 ******************************************************************************/

/**
 * @brief Reads one line of character data from an input file stream.
 *
 * @param[in,out] file
 * Open input stream whose read position advances past the extracted line.
 *
 * @param[out] line
 * Extracted characters without the terminating newline.
 *
 * @return
 * `true` when a line is extracted; otherwise `false` at end-of-file or on failure.
 */
inline boolean readFileLine(inputfilestream& file, string& line)
{
    return static_cast<boolean>(std::getline(file, line));
}

/**
 * @brief Reads the complete binary contents of one regular file.
 *
 * @param[in] path
 * Existing regular-file path to open and read.
 *
 * @return
 * Every byte represented in an owned character string, including null bytes.
 *
 * @throws std::runtime_error
 * If the file cannot be opened or read completely.
 */
inline string readFileContents(const filesystempath& path)
{
    inputfilestream file(path, FILE_OPEN_READ_BINARY);
    const hal::boolean openNotMatched =
        static_cast<hal::boolean>(
            !file.is_open());
    if (openNotMatched)
    {
        XHAL_THROW_RUNTIME_ERROR("File could not be opened for binary input");
    }
    const string contents{
        std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    const hal::boolean streamReadFailed =
        static_cast<hal::boolean>(
            file.bad());
    if (streamReadFailed)
    {
        XHAL_THROW_RUNTIME_ERROR("File could not be read completely");
    }
    return contents;
}

/**
 * @brief Reports whether a filesystem entry exists.
 *
 * @param[in] path
 * Filesystem path to inspect.
 *
 * @return
 * `true` when an entry exists at `path`; otherwise `false`.
 *
 * @throws filesystemerror
 * If the filesystem cannot inspect the path.
 */
inline boolean filesystemEntryExists(const filesystempath& path)
{
    return std::filesystem::exists(path);
}

/**
 * @brief Reports whether a filesystem path identifies a regular file.
 *
 * @param[in] path
 * Filesystem path to inspect.
 *
 * @return
 * `true` when `path` identifies a regular file; otherwise `false`.
 *
 * @throws filesystemerror
 * If the filesystem cannot inspect the path.
 */
inline boolean isRegularFile(const filesystempath& path)
{
    return std::filesystem::is_regular_file(path);
}

/**
 * @brief Reports whether a path is a readable regular file without throwing.
 * @param[in] path Exact filesystem path to inspect and open for reading.
 * @return `true` only when metadata identifies a regular file and an input stream opens successfully.
 */
inline boolean isReadableRegularFile(const filesystempath& path) noexcept
{
    errorcode operationError;
    const filesystemstatus status = std::filesystem::status(path, operationError);
    const hal::boolean operationErrorStatusInvalid =
        static_cast<hal::boolean>(
            operationError || !std::filesystem::is_regular_file(status));
    if (operationErrorStatusInvalid)
    {
        return false;
    }
    inputfilestream input(path, FILE_OPEN_READ_BINARY);
    return input.is_open();
}

/**
 * @brief Resolves a packaged resource without depending on the working directory.
 * @param[in] dataDirectory Absolute installed data root or test override.
 * @param[in] requestedPath Absolute path or path relative to `dataDirectory`.
 * @return Lexically normalized absolute or data-root-relative path.
 */
inline filesystempath resolveResourcePath(const filesystempath& dataDirectory,
    const filesystempath& requestedPath)
{
    return (requestedPath.is_absolute() ? requestedPath :
        (dataDirectory / requestedPath)).lexically_normal();
}

/**
 * @brief Returns the size of one regular file.
 *
 * @param[in] path
 * Existing regular-file path to inspect.
 *
 * @return
 * File size in bytes represented as an unsigned 64-bit value.
 *
 * @throws filesystemerror
 * If filesystem metadata cannot be read.
 *
 * @throws std::out_of_range
 * If the platform size exceeds the project unsigned 64-bit range.
 */
inline uint64 filesystemFileSize(const filesystempath& path)
{
    const auto platformSize = std::filesystem::file_size(path);
    const hal::boolean fileSizeTooLarge =
        static_cast<hal::boolean>(
            platformSize > std::numeric_limits<uint64>::max());
    if (fileSizeTooLarge)
    {
        XHAL_THROW_OUT_OF_RANGE("File size exceeds the supported unsigned range");
    }
    return static_cast<uint64>(platformSize);
}

/**
 * @brief Lists the filename of every direct child in one directory.
 *
 * @param[in] directoryPath
 * Existing directory whose immediate entries are enumerated.
 *
 * @return
 * Entry filenames in platform enumeration order without parent paths.
 *
 * @throws filesystemerror
 * If the directory cannot be enumerated.
 */
inline stringvector listFilesystemEntryNames(const filesystempath& directoryPath)
{
    stringvector entryNames{};
    for (const auto& entry : std::filesystem::directory_iterator(directoryPath))
    {
        entryNames.push_back(entry.path().filename().string());
    }
    return entryNames;
}

/**
 * @brief Creates every missing directory in a filesystem path.
 *
 * @param[in] path
 * Directory path to create.
 *
 * @return
 * `true` when at least one directory is created; otherwise `false`.
 *
 * @throws filesystemerror
 * If directory creation fails.
 */
inline boolean createDirectories(const filesystempath& path)
{
    return std::filesystem::create_directories(path);
}

/**
 * @brief Removes one file or empty directory.
 *
 * @param[in] path
 * Exact filesystem entry to remove.
 *
 * @return
 * `true` when an entry is removed; otherwise `false`.
 *
 * @throws filesystemerror
 * If removal fails for a reason other than the entry being absent.
 */
inline boolean removeFilesystemEntry(const filesystempath& path)
{
    return std::filesystem::remove(path);
}

/**
 * @brief Attempts to remove one file or empty directory without throwing.
 *
 * @param[in] path
 * Exact filesystem entry to remove.
 *
 * @param[out] operationError
 * Cleared on success or populated with the filesystem error.
 *
 * @return
 * `true` when an entry is removed; otherwise `false`.
 */
inline boolean removeFilesystemEntry(const filesystempath& path,
    errorcode& operationError) noexcept
{
    return std::filesystem::remove(path, operationError);
}

/**
 * @brief Retrieves filesystem metadata without throwing.
 *
 * @param[in] path
 * Filesystem path to inspect.
 *
 * @param[out] operationError
 * Cleared on success or populated with the filesystem error.
 *
 * @return
 * Status containing the detected entry type and permission bits.
 */
inline filesystemstatus filesystemStatus(const filesystempath& path,
    errorcode& operationError) noexcept
{
    return std::filesystem::status(path, operationError);
}

/**
 * @brief Retrieves filesystem metadata.
 *
 * @param[in] path
 * Filesystem path to inspect.
 *
 * @return
 * Status containing the detected entry type and permission bits.
 *
 * @throws filesystemerror
 * If the filesystem cannot inspect the path.
 */
inline filesystemstatus filesystemStatus(const filesystempath& path)
{
    return std::filesystem::status(path);
}

/**
 * @brief Replaces all permission bits on a filesystem entry without throwing.
 *
 * @param[in] path
 * Filesystem path whose permission bits are replaced.
 *
 * @param[in] permissions
 * Complete permission-bit value to apply.
 *
 * @param[out] operationError
 * Cleared on success or populated with the filesystem error.
 */
inline void replaceFilesystemPermissions(const filesystempath& path,
    filesystempermissions permissions, errorcode& operationError) noexcept
{
    std::filesystem::permissions(path, permissions, FILE_PERMISSION_REPLACE, operationError);
}

/**
 * @brief Renames or replaces a filesystem entry without throwing.
 *
 * @param[in] sourcePath
 * Existing entry to move.
 *
 * @param[in] destinationPath
 * Destination path supplied to the platform rename operation.
 *
 * @param[out] operationError
 * Cleared on success or populated with the filesystem error.
 */
inline void renameFilesystemEntry(const filesystempath& sourcePath,
    const filesystempath& destinationPath, errorcode& operationError) noexcept
{
    std::filesystem::rename(sourcePath, destinationPath, operationError);
}

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_FILE_FUNCTIONS_H */
