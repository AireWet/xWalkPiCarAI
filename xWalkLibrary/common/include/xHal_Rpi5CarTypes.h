/******************************************************************************
 * @file        xHal_Rpi5CarTypes.h
 * @brief       Declares shared scalar, container, and callback types.
 *
 * @details
 * Defines the project type vocabulary used by xWalk HAL, Agent, and Controller
 * interfaces and their host-test callback implementations.
 *
 * @project     xWalk Firmware
 * @module      xWalkLibraryCommon
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

#ifndef XHAL_RPI5CAR_TYPES_H
#define XHAL_RPI5CAR_TYPES_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarStandardHeaders.h"

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
 * Type definitions
 ******************************************************************************/

/** @brief Boolean value used by HAL interfaces and callbacks. */
using boolean = bool;
/** @brief IEEE single-precision floating-point value used by PCM backends. */
using float32 = float;
/** @brief IEEE double-precision floating-point value. */
using float64 = double;

/** @brief Signed integer with exactly 32 bits. */
using int32 = std::int32_t;
/** @brief Unsigned integer with exactly 8 bits. */
using uint8 = std::uint8_t;
/** @brief Unsigned integer with exactly 16 bits. */
using uint16 = std::uint16_t;
/** @brief Unsigned integer with exactly 32 bits. */
using uint32 = std::uint32_t;
/** @brief Unsigned integer with exactly 64 bits. */
using uint64 = std::uint64_t;
/** @brief Unsigned type used for container sizes and indices. */
using size = std::size_t;
/** @brief Non-owning pointer to immutable null-terminated character data. */
using cstring = const char*;
/** @brief Owned dynamically sized character string. */
using string = std::string;
/** @brief Standard invalid-argument exception reported by public validation. */
using invalidargument = std::invalid_argument;
/** @brief Standard range exception reported when a value exceeds supported limits. */
using outofrange = std::out_of_range;
/** @brief Standard length exception reported when a requested size is unsupported. */
using lengtherror = std::length_error;
/** @brief Standard domain exception reported when an input is mathematically invalid. */
using domainerror = std::domain_error;
/** @brief Standard logic exception reported when an object is in an invalid state. */
using logicerror = std::logic_error;
/** @brief Standard runtime exception reported when an operation cannot complete. */
using runtimeerror = std::runtime_error;
/** @brief Standard overflow exception reported when a result exceeds its upper bound. */
using overflowerror = std::overflow_error;
/** @brief Standard underflow exception reported when a result falls below its lower bound. */
using underflowerror = std::underflow_error;
/** @brief Standard system exception reported with an operating-system error code. */
using systemerror = std::system_error;
/** @brief Standard allocation exception reported when storage cannot be acquired. */
using badallocation = std::bad_alloc;
/** @brief Standard cast exception reported when a runtime cast fails. */
using badcast = std::bad_cast;
/** @brief Standard type-information exception reported for an invalid type query. */
using badtypeid = std::bad_typeid;
/** @brief Standard function exception reported when an empty callable is invoked. */
using badfunctioncall = std::bad_function_call;
/** @brief Standard optional exception reported when an empty value is accessed. */
using badoptionalaccess = std::bad_optional_access;
/** @brief Standard variant exception reported when the inactive alternative is accessed. */
using badvariantaccess = std::bad_variant_access;
/** @brief Standard weak-pointer exception reported when shared ownership has expired. */
using badweakpointer = std::bad_weak_ptr;
/** @brief Standard base exception used at process and callback error boundaries. */
using standardexception = std::exception;
/** @brief Non-owning pointer to mutable character data. */
using charpointer = char*;
/** @brief Non-owning opaque callback context; nullability is callback-specific. */
using contextpointer = void*;

/** @brief Dynamically sized sequence of bytes. */
using bytevector = std::vector<uint8>;
/** @brief Dynamically sized sequence of owned character strings. */
using stringvector = std::vector<string>;
/** @brief Dynamically sized sequence of mutable character pointers. */
using charpointervector = std::vector<charpointer>;
/** @brief Dynamically sized sequence of byte payloads. */
using bytevectorvector = std::vector<bytevector>;
/** @brief Dynamically sized sequence of unsigned 32-bit values. */
using uint32vector = std::vector<uint32>;
/** @brief Dynamically sized sequence of double-precision values. */
using float64vector = std::vector<float64>;
/** @brief Dynamically sized sequence of double-precision value sequences. */
using float64vectorvector = std::vector<float64vector>;
/** @brief Ordered set of unique byte values. */
using byteset = std::set<uint8>;

/**
 * @brief Fixed-size sequence containing values of one element type.
 *
 * @tparam ElementType
 * Stored element type.
 *
 * @tparam Length
 * Number of elements available in the sequence.
 */
template<typename ElementType, size Length>
using fixedarray = std::array<ElementType, Length>;

/**
 * @brief Ordered key-value container.
 *
 * @tparam KeyType
 * Key type used for ordered lookup.
 *
 * @tparam ValueType
 * Stored mapped-value type.
 */
template<typename KeyType, typename ValueType>
using orderedmap = std::map<KeyType, ValueType>;

/** @brief Exclusive owning pointer used for dynamically created library objects. */
template<typename ValueType>
using owningpointer = std::unique_ptr<ValueType>;

/** @brief Fixed list containing the three supported Robot HAT I2C addresses. */
using i2caddressarray = fixedarray<uint8, 3U>;
/** @brief Fixed list containing one period value for each of seven PWM timers. */
using pwmperiodarray = fixedarray<uint32, 7U>;

/** @brief Mutual-exclusion object used to protect shared HAL state. */
using mutexhandle = std::mutex;
/** @brief Scope-bound lock that owns a `mutexhandle` until destruction. */
using mutexlock = std::lock_guard<mutexhandle>;
/** @brief Movable lock used while waiting for a shared-state transition. */
using uniquemutexlock = std::unique_lock<mutexhandle>;
/** @brief Wait-and-notify primitive used for bounded transaction waits. */
using conditionvariable = std::condition_variable;
/** @brief Monotonic clock used for transaction deadlines. */
using steadyclock = std::chrono::steady_clock;
/** @brief Monotonic timestamp used for transaction deadlines. */
using steadytimestamp = steadyclock::time_point;
/** @brief Signed millisecond duration used for bounded waits. */
using millisecondduration = std::chrono::milliseconds;
/** @brief Result returned by condition-variable timed waits. */
using conditionstatus = std::cv_status;
/** @brief Atomic Boolean used to coordinate a hardware event worker. */
using atomicboolean = std::atomic<boolean>;
/** @brief Joinable execution thread used by an optional hardware backend. */
using threadhandle = std::thread;
/** @brief Optional unsigned byte, used when an I2C address may be omitted. */
using optionaluint8 = std::optional<uint8>;
/** @brief Optional double-precision value. */
using optionalfloat64 = std::optional<float64>;
/** @brief Non-owning read-only view of character data. */
using stringview = std::string_view;
/** @brief Owned platform-native filesystem path. */
using filesystempath = std::filesystem::path;
/** @brief Snapshot of filesystem object type and permission information. */
using filesystemstatus = std::filesystem::file_status;
/** @brief Filesystem permission-bit representation. */
using filesystempermissions = std::filesystem::perms;
/** @brief Filesystem option controlling how permission bits are modified. */
using filesystempermissionoptions = std::filesystem::perm_options;
/** @brief Exception reported by throwing filesystem operations. */
using filesystemerror = std::filesystem::filesystem_error;
/** @brief Non-throwing standard-library error-code result. */
using errorcode = std::error_code;
/** @brief File stream used to read character data. */
using inputfilestream = std::ifstream;
/** @brief File stream used to write character data. */
using outputfilestream = std::ofstream;
/** @brief Bitmask describing how a file stream is opened. */
using fileopenmode = std::ios::openmode;

/**
 * @brief Callback used to probe an I2C address.
 *
 * @param[in] context
 * Non-owning backend context supplied when the I2C object is constructed.
 *
 * @param[in] address
 * Seven-bit I2C address to probe.
 *
 * @return
 * `true` when a device responds; otherwise `false`.
 */
using i2cprobecallback = boolean (*)(contextpointer context, uint8 address);

/**
 * @brief Callback used to write a payload to an I2C device register.
 *
 * @param[in] context
 * Non-owning backend context supplied when the I2C object is constructed.
 *
 * @param[in] address
 * Seven-bit I2C destination address.
 *
 * @param[in] reg
 * Eight-bit destination register address.
 *
 * @param[in] data
 * Non-empty payload to write, expressed in bytes.
 */
using i2cwriteregistercallback = void (*)(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data);

/**
 * @brief Non-throwing callback used for fail-safe I2C register writes.
 * @return `true` when the complete payload was written; otherwise `false`.
 */
using i2ctrywriteregistercallback = boolean (*)(contextpointer context, uint8 address, uint8 reg,
    const bytevector& data) noexcept;

/**
 * @brief Callback used to read bytes directly from an I2C device.
 *
 * @param[in] context
 * Non-owning backend context supplied when the I2C object is constructed.
 *
 * @param[in] address
 * Seven-bit I2C source address.
 *
 * @param[in] length
 * Number of bytes requested; valid range is backend-specific and must be non-zero.
 *
 * @return
 * Bytes returned by the device in bus order.
 */
using i2creadcallback = bytevector (*)(contextpointer context, uint8 address, size length);

/**
 * @brief Callback used to read bytes beginning at an I2C register.
 *
 * @param[in] context
 * Non-owning backend context supplied when the I2C object is constructed.
 *
 * @param[in] address
 * Seven-bit I2C source address.
 *
 * @param[in] reg
 * First eight-bit register address to read.
 *
 * @param[in] length
 * Non-zero number of consecutive bytes requested.
 *
 * @return
 * Bytes returned by the device in ascending register order.
 */
using i2creadregistercallback = bytevector (*)(contextpointer context, uint8 address, uint8 reg,
    size length);

} /* namespace xwalk::hal */

/**
 * @namespace xwalk::agent
 * @brief Contains Agent components and their shared type vocabulary.
 */
namespace xwalk::agent
{

/******************************************************************************
 * Shared Agent type imports
 ******************************************************************************/

using hal::boolean;
using hal::float32;
using hal::float64;
using hal::int32;
using hal::uint8;
using hal::uint16;
using hal::uint32;
using hal::uint64;
using hal::size;
using hal::cstring;
using hal::string;
using hal::invalidargument;
using hal::outofrange;
using hal::lengtherror;
using hal::domainerror;
using hal::logicerror;
using hal::runtimeerror;
using hal::overflowerror;
using hal::underflowerror;
using hal::systemerror;
using hal::badallocation;
using hal::badcast;
using hal::badtypeid;
using hal::badfunctioncall;
using hal::badoptionalaccess;
using hal::badvariantaccess;
using hal::badweakpointer;
using hal::standardexception;
using hal::charpointer;
using hal::contextpointer;
using hal::bytevector;
using hal::stringvector;
using hal::charpointervector;
using hal::bytevectorvector;
using hal::uint32vector;
using hal::float64vector;
using hal::float64vectorvector;
using hal::byteset;
using hal::fixedarray;
using hal::orderedmap;
using hal::owningpointer;
using hal::mutexhandle;
using hal::mutexlock;
using hal::uniquemutexlock;
using hal::conditionvariable;
using hal::steadyclock;
using hal::steadytimestamp;
using hal::millisecondduration;
using hal::conditionstatus;
using hal::atomicboolean;
using hal::threadhandle;
using hal::optionaluint8;
using hal::optionalfloat64;
using hal::stringview;
using hal::filesystempath;
using hal::filesystemstatus;
using hal::filesystempermissions;
using hal::filesystempermissionoptions;
using hal::filesystemerror;
using hal::errorcode;
using hal::inputfilestream;
using hal::outputfilestream;
using hal::fileopenmode;

} /* namespace xwalk::agent */

/**
 * @namespace xwalk::controller
 * @brief Contains Controller components and their shared type vocabulary.
 */
namespace xwalk::controller
{

/******************************************************************************
 * Shared Controller type imports
 ******************************************************************************/

using hal::boolean;
using hal::float32;
using hal::float64;
using hal::int32;
using hal::uint8;
using hal::uint16;
using hal::uint32;
using hal::uint64;
using hal::size;
using hal::cstring;
using hal::string;
using hal::invalidargument;
using hal::outofrange;
using hal::lengtherror;
using hal::domainerror;
using hal::logicerror;
using hal::runtimeerror;
using hal::overflowerror;
using hal::underflowerror;
using hal::systemerror;
using hal::badallocation;
using hal::badcast;
using hal::badtypeid;
using hal::badfunctioncall;
using hal::badoptionalaccess;
using hal::badvariantaccess;
using hal::badweakpointer;
using hal::standardexception;
using hal::charpointer;
using hal::contextpointer;
using hal::bytevector;
using hal::stringvector;
using hal::charpointervector;
using hal::bytevectorvector;
using hal::uint32vector;
using hal::float64vector;
using hal::float64vectorvector;
using hal::byteset;
using hal::fixedarray;
using hal::orderedmap;
using hal::owningpointer;
using hal::mutexhandle;
using hal::mutexlock;
using hal::uniquemutexlock;
using hal::conditionvariable;
using hal::steadyclock;
using hal::steadytimestamp;
using hal::millisecondduration;
using hal::conditionstatus;
using hal::atomicboolean;
using hal::threadhandle;
using hal::optionaluint8;
using hal::optionalfloat64;
using hal::stringview;
using hal::filesystempath;
using hal::filesystemstatus;
using hal::filesystempermissions;
using hal::filesystempermissionoptions;
using hal::filesystemerror;
using hal::errorcode;
using hal::inputfilestream;
using hal::outputfilestream;
using hal::fileopenmode;

} /* namespace xwalk::controller */

/**
 * @namespace hal
 * @brief Provides the concise xWalk HAL namespace alias.
 */
namespace hal = xwalk::hal;

/**
 * @namespace agent
 * @brief Provides the concise xWalk Agent namespace alias.
 */
namespace agent = xwalk::agent;

/**
 * @namespace ctrl
 * @brief Provides the concise xWalk Controller namespace alias.
 */
namespace ctrl = xwalk::controller;

/**
 * @namespace XWalkHal
 * @brief Provides the legacy alias for the xWalk HAL namespace.
 */
namespace XWalkHal = xwalk::hal;

#endif /* XHAL_RPI5CAR_TYPES_H */
