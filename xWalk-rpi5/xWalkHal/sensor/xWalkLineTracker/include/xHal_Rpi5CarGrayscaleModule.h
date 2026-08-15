/******************************************************************************
 * @file        xHal_Rpi5CarGrayscaleModule.h
 * @brief       Declares the three-channel grayscale sensing interface.
 *
 * @details
 * Defines raw ADC acquisition, configurable threshold references, and
 * black-or-white classification for three caller-owned sensor channels.
 *
 * @project     xWalk Firmware
 * @module      xWalkLineTracker
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

#ifndef XHAL_RPI5CAR_GRAYSCALE_MODULE_H
#define XHAL_RPI5CAR_GRAYSCALE_MODULE_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarAdc.h"
#include "xHal_Rpi5CarLineTrackerTypes.h"

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
     * @class XWalkGrayscaleModule
     * @brief Reads three caller-owned ADC channels and classifies black or white.
     *
     * @details
     * Stores bounded non-owning pointers ordered left, middle, and right. Each
     * reading is compared with its configured reference in raw ADC counts.
     */
    class XWalkGrayscaleModule
    {
        private:
            /**************************************************************************
             * Private data members
             **************************************************************************/

            /** @brief Non-owning ADC pointers that are never null after construction. */
            linetrackeradcpointers sensors{};

            /** @brief Per-channel black/white thresholds in raw ADC counts. */
            linetrackervalues referenceValues{1'000, 1'000, 1'000};

        protected:
            /**************************************************************************
             * Protected member functions
             **************************************************************************/

            /**
             * @brief Returns a validated ADC dependency by zero-based channel.
             *
             * @param[in] channel
             * Channel index in the inclusive range zero through two.
             *
             * @return
             * Caller-owned ADC object for the selected channel.
             *
             * @throws outofrange
             * If `channel` exceeds two.
             */
            XWalkAdc& sensorAt(uint32 channel) const;

        public:
            /**************************************************************************
             * Public constructors and destructor
             **************************************************************************/

            /**
             * @brief Constructs a three-channel grayscale module.
             *
             * @param[in] left
             * Left ADC dependency that must outlive this object.
             *
             * @param[in] middle
             * Middle ADC dependency that must outlive this object.
             *
             * @param[in] right
             * Right ADC dependency that must outlive this object.
             */
            XWalkGrayscaleModule(XWalkAdc& left, XWalkAdc& middle, XWalkAdc& right);

            /** @brief Destroys the module without releasing its non-owning ADC dependencies. */
            ~XWalkGrayscaleModule();

            /**************************************************************************
             * Public special member functions
             **************************************************************************/

            /** @brief Disables move construction to preserve dependency identities. */
            XWalkGrayscaleModule(XWalkGrayscaleModule&&) = delete;
            /** @brief Disables copying of ADC dependency bindings. */
            XWalkGrayscaleModule(const XWalkGrayscaleModule&) = delete;
            /** @brief Disables move assignment of ADC dependency bindings. */
            XWalkGrayscaleModule& operator=(XWalkGrayscaleModule&&) = delete;
            /** @brief Disables copy assignment of ADC dependency bindings. */
            XWalkGrayscaleModule& operator=(const XWalkGrayscaleModule&) = delete;

            /**************************************************************************
             * Public member functions
             **************************************************************************/

            /**
             * @brief Replaces all black/white reference thresholds.
             *
             * @param[in] references
             * Left, middle, and right reference values in raw ADC counts.
             *
             * @post
             * `reference()` equals `references`.
             */
            void setReference(const linetrackervalues& references) noexcept;

            /**
             * @brief Returns the active black/white thresholds.
             *
             * @return
             * Read-only left, middle, and right reference values in raw ADC counts.
             */
            const linetrackervalues& reference() const noexcept;

            /**
             * @brief Reads one raw grayscale channel.
             *
             * @param[in] channel
             * Channel index in the inclusive range zero through two.
             *
             * @return
             * Raw ADC sample count.
             *
             * @throws outofrange
             * If `channel` exceeds two.
             *
             * @throws runtimeerror
             * If the ADC transaction does not return a complete sample.
             */
            int32 readChannel(uint32 channel);

            /**
             * @brief Reads all three raw grayscale channels.
             *
             * @return
             * Left, middle, and right ADC sample counts.
             */
            linetrackervalues read();

            /**
             * @brief Reads and classifies all three grayscale channels.
             *
             * @return
             * Zero for values above their references and one for values at or below them.
             */
            linetrackerstatus readStatus();

            /**
             * @brief Classifies supplied grayscale values.
             *
             * @param[in] data
             * Left, middle, and right values in ADC counts.
             *
             * @return
             * Zero for values above their references and one for values at or below them.
             */
            linetrackerstatus readStatus(const linetrackervalues& data) const noexcept;
    };

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_GRAYSCALE_MODULE_H */
