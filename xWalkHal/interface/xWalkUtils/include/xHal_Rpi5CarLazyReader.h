/******************************************************************************
 * @file        xHal_Rpi5CarLazyReader.h
 * @brief       Declares a bounded-rate generic value reader.
 *
 * @details
 * Ports the Python `LazyReader` cache using an injected acquisition function
 * and monotonic clock while retaining the most recently acquired value.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils
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

#ifndef XHAL_RPI5CAR_LAZY_READER_H
#define XHAL_RPI5CAR_LAZY_READER_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarUtilsTypes.h"

#include "xHal_Rpi5CarTrace.h"
/******************************************************************************
 * Namespace declarations
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal {

/******************************************************************************
 * Class declarations
 ******************************************************************************/

/**
 * @class XWalkLazyReader
 * @brief Caches a callback-provided value for a configured interval.
 *
 * @tparam ValueType
 * Default-constructible value type returned by the injected read callback.
 *
 * @details
 * The first call always acquires a value. Later calls reacquire only when the
 * elapsed interval is strictly greater than the configured interval, matching
 * the Python implementation. Calls require external serialization.
 *
 * @note
 * Use a value-like type or a non-owning pointer type. Do not cache a project
 * component object by value; create that object in `main()` and cache a
 * pointer.
 */
template <typename ValueType> class XWalkLazyReader final {
private:
  /**************************************************************************
   * Private data members
   **************************************************************************/

  /** @brief Nullable non-owning context used by read and clock callbacks. */
  contextpointer backendContextPointer;

  /** @brief Non-null callback that acquires and returns one value. */
  utilityreadcallback<ValueType> readCallback;

  /** @brief Non-null callback providing monotonic microsecond time. */
  utilityclockcallback clockCallback;

  /** @brief Refresh interval converted to microseconds during construction. */
  uint64 intervalUs;

  /** @brief Monotonic acquisition timestamp in microseconds. */
  uint64 lastReadTimeUs;

  /** @brief Most recently acquired value retained by this reader. */
  ValueType cachedValue;

  /** @brief `true` after the first successful acquisition. */
  boolean containsValue;

protected:
  /**************************************************************************
   * Protected member functions
   **************************************************************************/

  /**
   * @brief Validates both required callbacks.
   *
   * @param[in] readOperation
   * Value-acquisition callback that must be non-null.
   *
   * @param[in] clockOperation
   * Monotonic-clock callback that must be non-null.
   *
   * @throws std::invalid_argument
   * If either callback is null.
   */
  static void validateCallbacks(utilityreadcallback<ValueType> read,
                                utilityclockcallback clock) {
    if ((read == nullptr) || (clock == nullptr)) {
      XWALK_HAL_ERROR(XWALK_INVAL, "Lazy reader callbacks must not be null");
    }
  }

public:
  /**************************************************************************
   * Public constructors and destructor
   **************************************************************************/

  /**
   * @brief Constructs an empty cache from caller-owned callbacks.
   *
   * @param[in,out] backendContext
   * Nullable non-owning context used by both callbacks.
   *
   * @param[in] readOperation
   * Non-null callback that acquires one value synchronously.
   *
   * @param[in] clockOperation
   * Non-null callback returning monotonic microseconds.
   *
   * @param[in] intervalMs
   * Cache interval in milliseconds; zero reacquires when time advances.
   *
   * @throws std::invalid_argument
   * If either callback is null.
   */
  XWalkLazyReader(
      contextpointer backendContext,
      utilityreadcallback<ValueType> readOperation,
      utilityclockcallback clockOperation,
      uint32 intervalMs = XHAL_RPI5CAR_UTILS_DEFAULT_LAZY_INTERVAL_MS)
      : backendContextPointer(backendContext), readCallback(readOperation),
        clockCallback(clockOperation),
        intervalUs(static_cast<uint64>(intervalMs) *
                   XHAL_RPI5CAR_UTILS_MICROSECONDS_PER_MILLISECOND),
        lastReadTimeUs(0U), cachedValue{}, containsValue(false) {
    validateCallbacks(readOperation, clockOperation);
  }

  /** @brief Destroys the cache without releasing callback resources. */
  ~XWalkLazyReader() = default;

  /**************************************************************************
   * Public special member functions
   **************************************************************************/

  /** @brief Disables copying of callback-bound cache state. */
  XWalkLazyReader(const XWalkLazyReader &) = delete;
  /** @brief Disables copy assignment of callback-bound cache state. */
  XWalkLazyReader &operator=(const XWalkLazyReader &) = delete;
  /** @brief Disables moving because callback context identity is retained. */
  XWalkLazyReader(XWalkLazyReader &&) = delete;
  /** @brief Disables move assignment because callback context identity is
   * retained. */
  XWalkLazyReader &operator=(XWalkLazyReader &&) = delete;

  /**************************************************************************
   * Public member functions
   **************************************************************************/

  /**
   * @brief Returns a cached value and refreshes it only after the interval.
   *
   * @return
   * Const reference valid until the next successful refresh or destruction.
   *
   * @note
   * Read and clock exceptions propagate without replacing cached state.
   */
  const ValueType &read() {
    const uint64 currentTimeUs = clockCallback(backendContextPointer);
    const uint64 elapsedTimeUs = currentTimeUs - lastReadTimeUs;
    if ((!containsValue) || (elapsedTimeUs > intervalUs)) {
      ValueType acquiredValue = readCallback(backendContextPointer);
      cachedValue = acquiredValue;
      lastReadTimeUs = currentTimeUs;
      containsValue = true;
    }
    return cachedValue;
  }
};

} /* namespace xwalk::hal */

#endif /* XHAL_RPI5CAR_LAZY_READER_H */
