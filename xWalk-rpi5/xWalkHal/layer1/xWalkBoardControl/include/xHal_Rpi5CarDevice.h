/******************************************************************************
 * @file        xHal_Rpi5CarDevice.h
 * @brief       Declares Linux device-tree Robot HAT discovery.
 *
 * @details
 * Scans a configurable firmware device-tree root, recognizes supported Robot
 * HAT UUIDs, parses product metadata, and selects board-specific configuration.
 *
 * @project     xWalk Firmware
 * @module      xWalkBoardControl
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

#ifndef XHAL_RPI5CAR_DEVICE_H
#define XHAL_RPI5CAR_DEVICE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarDeviceTypes.h"

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
     * @class XWalkDevice
     * @brief Detects a supported Robot HAT and exposes immutable discovery results.
     *
     * @details
     * Owns only its device-tree root and detected value state. It does not create
     * GPIO, motor, speaker, or ADC objects and performs no hardware mutation.
     */
    class XWalkDevice
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Owned device-tree root used by construction and every refresh. */
            filesystempath deviceTreeRootValue{};
            /** @brief Most recent complete discovery result or documented defaults. */
            XWalkDeviceInformation informationValue{};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Resets metadata and board configuration to Python-compatible defaults.
             *
             * @post
             * Metadata is empty, detection is false, and Robot HAT v4 pin and motor values apply.
             */
            void resetInformation() noexcept;

            /**
             * @brief Finds the first candidate node containing a supported UUID.
             *
             * @return
             * Candidate node path, or an empty path when no supported HAT is present.
             *
             * @throws filesystemerror
             * If the device-tree root cannot be enumerated or inspected.
             *
             * @throws std::runtime_error
             * If a candidate UUID property cannot be read.
             */
            filesystempath findSupportedHatPath() const;

            /**
             * @brief Reads one property from a selected device-tree node.
             *
             * @param[in] hatPath
             * Existing selected HAT node path.
             *
             * @param[in] propertyName
             * Non-empty property filename below `hatPath`.
             *
             * @param[in] removeTrailingNull
             * `true` to remove one terminal null byte when present.
             *
             * @return
             * Complete property contents after the requested compatibility trimming.
             *
             * @throws std::runtime_error
             * If the property is absent, not regular, or cannot be read.
             */
            static string
            readProperty(const filesystempath& hatPath, stringview propertyName, boolean removeTrailingNull);

            /**
             * @brief Parses one unsigned hexadecimal device-tree property.
             *
             * @param[in] text
             * One through eight hexadecimal digits, optionally prefixed with `0x`.
             *
             * @param[in] propertyName
             * Non-null property name used in validation messages.
             *
             * @return
             * Parsed unsigned 32-bit value.
             *
             * @throws std::runtime_error
             * If the text is empty, malformed, or exceeds 32 bits.
             */
            static uint32 parseHexProperty(stringview text, cstring propertyName);

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a device detector and immediately scans its root.
             *
             * @param[in] deviceTreeRoot
             * Non-empty device-tree root path; defaults to `/proc/device-tree`.
             *
             * @post
             * `information()` contains detected metadata or default board values.
             *
             * @throws std::invalid_argument
             * If the root path is empty.
             *
             * @throws filesystemerror
             * If the root cannot be enumerated or inspected.
             *
             * @throws std::runtime_error
             * If a selected HAT property cannot be read or parsed.
             */
            explicit XWalkDevice(stringview deviceTreeRoot = XHAL_RPI5CAR_DEVICE_TREE_ROOT);

            /** @brief Destroys the detector without modifying its device-tree source. */
            ~XWalkDevice();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Enables value-like move construction. */
            XWalkDevice(XWalkDevice&&) = default;
            /** @brief Enables value-like copy construction. */
            XWalkDevice(const XWalkDevice&) = default;
            /** @brief Enables value-like move assignment. */
            XWalkDevice& operator=(XWalkDevice&&) = default;
            /** @brief Enables value-like copy assignment. */
            XWalkDevice& operator=(const XWalkDevice&) = default;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Repeats device-tree discovery from the configured root.
             *
             * @post
             * Previous results are replaced atomically at the object level after
             * every required property has been read and validated.
             *
             * @throws filesystemerror
             * If the root or a candidate property cannot be inspected.
             *
             * @throws std::runtime_error
             * If a selected HAT property cannot be read or parsed.
             */
            void refresh();

            /**
             * @brief Returns the most recent complete discovery result.
             *
             * @return
             * Product metadata, board configuration, model, and detection state.
             */
            const XWalkDeviceInformation& information() const noexcept;

            /**
             * @brief Returns the configured device-tree root.
             *
             * @return
             * Owned root path retained by this detector.
             */
            const filesystempath& deviceTreeRoot() const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_DEVICE_H */
