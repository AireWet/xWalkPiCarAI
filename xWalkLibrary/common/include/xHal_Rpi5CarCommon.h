/******************************************************************************
 * @file        xHal_Rpi5CarCommon.h
 * @brief       Aggregates shared xWalk HAL facilities and hardware constants.
 *
 * @details
 * Exposes the common functions, exceptions, math operations, project types,
 * callback bridges, and Robot HAT register constants used by HAL modules.
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

#ifndef XHAL_RPI5CAR_COMMON_H
#define XHAL_RPI5CAR_COMMON_H

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarCommonFunctions.h"
#include "xHal_Rpi5CarExceptions.h"
#include "xHal_Rpi5CarFileFunctions.h"
#include "xHal_Rpi5CarMath.h"
#include "xHal_Rpi5CarTypes.h"

/******************************************************************************
 * Function-like macros
 ******************************************************************************/

/**
 * @brief Creates a non-owning I2C probe callback for a backend type.
 *
 * @details
 * The generated callback casts the supplied context to `BACKEND_TYPE` and
 * forwards the address to `probeDevice`.
 *
 * @warning
 * The context must point to a live `BACKEND_TYPE` object for every callback.
 */
#define XHAL_I2C_PROBE_CALLBACK(BACKEND_TYPE) +[](xwalk::hal::contextpointer context, \
    xwalk::hal::uint8 address) -> xwalk::hal::boolean \
    { \
        BACKEND_TYPE& backend = *static_cast<BACKEND_TYPE*>(context); \
        return backend.probeDevice(address); \
    }

/**
 * @brief Creates a non-owning I2C register-write callback for a backend type.
 *
 * @details
 * The generated callback casts the supplied context to `BACKEND_TYPE` and
 * forwards the address, register, and payload to `writeRegisterDevice`.
 *
 * @warning
 * The context must point to a live `BACKEND_TYPE` object for every callback.
 */
#define XHAL_I2C_WRITE_REGISTER_CALLBACK(BACKEND_TYPE) +[](xwalk::hal::contextpointer context, \
    xwalk::hal::uint8 address, xwalk::hal::uint8 reg, const xwalk::hal::bytevector& data) \
    { \
        BACKEND_TYPE& backend = *static_cast<BACKEND_TYPE*>(context); \
        backend.writeRegisterDevice(address, reg, data); \
    }

/**
 * @brief Creates a non-throwing I2C register-write status callback for a backend type.
 * @warning The context must point to a live `BACKEND_TYPE` object for every callback.
 */
#define XHAL_I2C_TRY_WRITE_REGISTER_CALLBACK(BACKEND_TYPE) +[](xwalk::hal::contextpointer context, \
    xwalk::hal::uint8 address, xwalk::hal::uint8 reg, const xwalk::hal::bytevector& data) noexcept \
    -> xwalk::hal::boolean \
    { \
        BACKEND_TYPE& backend = *static_cast<BACKEND_TYPE*>(context); \
        return backend.tryWriteRegisterDevice(address, reg, data); \
    }

/**
 * @brief Creates a non-owning I2C read callback for a backend type.
 *
 * @warning
 * The context must point to a live `BACKEND_TYPE` object for every callback.
 */
#define XHAL_I2C_READ_CALLBACK(BACKEND_TYPE) +[](xwalk::hal::contextpointer context, \
    xwalk::hal::uint8 address, xwalk::hal::size length) -> xwalk::hal::bytevector \
    { \
        BACKEND_TYPE& backend = *static_cast<BACKEND_TYPE*>(context); \
        return backend.readDevice(address, length); \
    }

/**
 * @brief Creates a non-owning I2C register-read callback for a backend type.
 *
 * @warning
 * The context must point to a live `BACKEND_TYPE` object for every callback.
 */
#define XHAL_I2C_READ_REGISTER_CALLBACK(BACKEND_TYPE) +[](xwalk::hal::contextpointer context, \
    xwalk::hal::uint8 address, xwalk::hal::uint8 reg, xwalk::hal::size length) \
    -> xwalk::hal::bytevector \
    { \
        BACKEND_TYPE& backend = *static_cast<BACKEND_TYPE*>(context); \
        return backend.readRegisterDevice(address, reg, length); \
    }

/******************************************************************************
 * Constants
 ******************************************************************************/

/** @brief Default number of conversation messages retained by a language-model backend. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_DEFAULT_MAXIMUM_MESSAGES 20U
/** @brief Maximum messages retained by the Ollama provider backend. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_MESSAGES 200U
/** @brief Maximum bytes accepted for one model, endpoint, instruction, or message text. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TEXT_BYTES 262'144U
/** @brief Maximum raw bytes read from one optional model image. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_IMAGE_BYTES 4'194'304U
/** @brief Maximum serialized Ollama JSON request bytes. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_REQUEST_BYTES 8'388'608U
/** @brief Maximum Ollama HTTP response bytes. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_RESPONSE_BYTES 1'048'576U
/** @brief Default Ollama request timeout in milliseconds. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_DEFAULT_TIMEOUT_MS 120'000U
/** @brief Maximum Ollama request timeout in milliseconds. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_TIMEOUT_MS 300'000U
/** @brief Maximum accepted deployment API-key length in bytes. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_API_KEY_BYTES 8'192U
/** @brief Default bounded completion output-token request. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_DEFAULT_MAXIMUM_OUTPUT_TOKENS 1'024U
/** @brief Maximum accepted completion output-token request. */
#define XHAL_RPI5CAR_LANGUAGE_MODEL_HTTP_MAXIMUM_OUTPUT_TOKENS 1'000'000U
/** @brief Default bounded microphone-recognition interval in milliseconds. */
#define XHAL_RPI5CAR_SPEECH_TO_TEXT_DEFAULT_TIMEOUT_MS 30'000U
/** @brief Longest accepted microphone-recognition interval in milliseconds. */
#define XHAL_RPI5CAR_SPEECH_TO_TEXT_MAXIMUM_TIMEOUT_MS 300'000U
/** @brief ALSA speech capture sample rate in Hertz. */
#define XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_RATE_HZ 16'000U
/** @brief Mono channel count used for speech capture. */
#define XHAL_RPI5CAR_SPEECH_CAPTURE_CHANNEL_COUNT 1U
/** @brief Byte count of one signed sixteen-bit speech sample. */
#define XHAL_RPI5CAR_SPEECH_CAPTURE_SAMPLE_BYTES 2U
/** @brief Maximum frames requested by one speech capture read. */
#define XHAL_RPI5CAR_SPEECH_CAPTURE_PERIOD_FRAMES 1'024U
/** @brief Maximum signed sixteen-bit PCM frames written per text-to-speech operation. */
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_PERIOD_FRAMES 1'024U
/** @brief Maximum synthesized PCM payload retained for one speech request, in bytes. */
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_MAXIMUM_PCM_BYTES 16'777'216U
/** @brief Byte count of one signed sixteen-bit synthesized speech sample. */
#define XHAL_RPI5CAR_TEXT_TO_SPEECH_SAMPLE_BYTES 2U
/** @brief Numeric severity assigned to critical trace records. */
#define XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL 0U
/** @brief Numeric severity assigned to error trace records. */
#define XHAL_RPI5CAR_TRACE_LEVEL_ERROR 1U
/** @brief Numeric severity assigned to warning trace records and the default threshold. */
#define XHAL_RPI5CAR_TRACE_LEVEL_WARNING 2U
/** @brief Numeric severity assigned to informational trace records. */
#define XHAL_RPI5CAR_TRACE_LEVEL_INFO 3U
/** @brief Numeric severity assigned to debug trace records. */
#define XHAL_RPI5CAR_TRACE_LEVEL_DEBUG 4U
/** @brief Number of supported trace severities. */
#define XHAL_RPI5CAR_TRACE_LEVEL_COUNT 5U
/** @brief Lowercase text accepted for the critical trace severity. */
#define XHAL_RPI5CAR_TRACE_LEVEL_CRITICAL_NAME "critical"
/** @brief Lowercase text accepted for the error trace severity. */
#define XHAL_RPI5CAR_TRACE_LEVEL_ERROR_NAME "error"
/** @brief Lowercase text accepted for the warning trace severity. */
#define XHAL_RPI5CAR_TRACE_LEVEL_WARNING_NAME "warning"
/** @brief Lowercase text accepted for the informational trace severity. */
#define XHAL_RPI5CAR_TRACE_LEVEL_INFO_NAME "info"
/** @brief Lowercase text accepted for the debug trace severity. */
#define XHAL_RPI5CAR_TRACE_LEVEL_DEBUG_NAME "debug"
/** @brief Prefix used when reporting a trace threshold change. */
#define XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_PREFIX "Set trace level to ["
/** @brief Suffix used when reporting a trace threshold change. */
#define XHAL_RPI5CAR_TRACE_LEVEL_CHANGE_SUFFIX "]"
/** @brief Number of supported tagged-trace priorities. */
#define XHAL_RPI5CAR_TRACE_PRIORITY_COUNT 4U
/** @brief Working-directory-relative directory receiving xWalk trace records. */
#define XHAL_RPI5CAR_TRACE_LOG_DIRECTORY "log"
/** @brief Append-only xWalk trace filename. */
#define XHAL_RPI5CAR_TRACE_LOG_FILENAME "xWalkTrace.log"

#ifndef XWALK_TRACE_CONFIG_PATH
/** @brief Generated trace XML path overridden by the xWalkTrace CMake target. */
#define XWALK_TRACE_CONFIG_PATH "xWalkTrace.xml"
#endif
/** @brief Suffix used for a same-directory configuration replacement file. */
#define XHAL_RPI5CAR_CONFIG_REPLACEMENT_SUFFIX ".tmp"
/** @brief Prefix written before each initial configuration description line. */
#define XHAL_RPI5CAR_CONFIG_COMMENT_PREFIX "# "
/** @brief Separator used when serializing a configuration option and value. */
#define XHAL_RPI5CAR_CONFIG_ASSIGNMENT_SEPARATOR " = "
/** @brief Base register address for the twenty PWM channel outputs. */
#define XHAL_RPI5CAR_PWM_CHANNEL_REG       0x20U
/** @brief Base prescaler register for PWM timers zero through three. */
#define XHAL_RPI5CAR_PWM_PRESCALER_REG     0x40U
/** @brief Base period register for PWM timers zero through three. */
#define XHAL_RPI5CAR_PWM_PERIOD_REG        0x44U
/** @brief Base prescaler register for PWM timers four through six. */
#define XHAL_RPI5CAR_PWM_PRESCALER_REG_2   0x50U
/** @brief Base period register for PWM timers four through six. */
#define XHAL_RPI5CAR_PWM_PERIOD_REG_2      0x54U
/** @brief Robot HAT PWM timer input clock frequency in Hertz. */
#define XHAL_RPI5CAR_PWM_CLOCK_HZ          72'000'000.0
/** @brief Default Robot HAT PWM output frequency in Hertz. */
#define XHAL_RPI5CAR_PWM_DEFAULT_FREQUENCY_HZ 50.0
/** @brief Number of prescaler values checked by the PWM frequency search. */
#define XHAL_RPI5CAR_PWM_SEARCH_CANDIDATE_COUNT 10
/** @brief Prescaler offsets searched on either side of the ideal value. */
#define XHAL_RPI5CAR_PWM_SEARCH_RADIUS      5
/** @brief Lowest valid prescaler candidate considered by the search. */
#define XHAL_RPI5CAR_PWM_MIN_PRESCALER_CANDIDATE 1
/** @brief Highest valid Robot HAT PWM channel index. */
#define XHAL_RPI5CAR_PWM_MAX_CHANNEL       19U
/** @brief Number of channels mapped four-at-a-time to timers zero to three. */
#define XHAL_RPI5CAR_PWM_DIRECT_CHANNEL_COUNT 16U
/** @brief Number of consecutive channels sharing each direct PWM timer. */
#define XHAL_RPI5CAR_PWM_CHANNELS_PER_TIMER 4U
/** @brief Highest channel mapped to PWM timer four. */
#define XHAL_RPI5CAR_PWM_TIMER_FOUR_MAX_CHANNEL 17U
/** @brief Channel mapped to PWM timer five. */
#define XHAL_RPI5CAR_PWM_TIMER_FIVE_CHANNEL 18U
/** @brief PWM timer index used by channels sixteen and seventeen. */
#define XHAL_RPI5CAR_PWM_TIMER_FOUR        4U
/** @brief PWM timer index used by channel eighteen. */
#define XHAL_RPI5CAR_PWM_TIMER_FIVE        5U
/** @brief PWM timer index used by channel nineteen. */
#define XHAL_RPI5CAR_PWM_TIMER_SIX         6U
/** @brief Number of independently driven color channels in one RGB LED. */
#define XHAL_RPI5CAR_RGB_LED_CHANNEL_COUNT 3U
/** @brief Array index and PWM dependency assigned to the red LED channel. */
#define XHAL_RPI5CAR_RGB_LED_RED_CHANNEL 0U
/** @brief Array index and PWM dependency assigned to the green LED channel. */
#define XHAL_RPI5CAR_RGB_LED_GREEN_CHANNEL 1U
/** @brief Array index and PWM dependency assigned to the blue LED channel. */
#define XHAL_RPI5CAR_RGB_LED_BLUE_CHANNEL 2U
/** @brief Numeric selector for a common-cathode RGB LED connection. */
#define XHAL_RPI5CAR_RGB_LED_COMMON_CATHODE 0U
/** @brief Numeric selector for a common-anode RGB LED connection. */
#define XHAL_RPI5CAR_RGB_LED_COMMON_ANODE 1U
/** @brief Highest valid intensity for an eight-bit RGB component. */
#define XHAL_RPI5CAR_RGB_LED_MAX_COMPONENT 255U
/** @brief Highest valid packed color in the `0xRRGGBB` representation. */
#define XHAL_RPI5CAR_RGB_LED_MAX_PACKED_COLOR 0xFFFFFFU
/** @brief Bit mask selecting the red component from packed `0xRRGGBB` data. */
#define XHAL_RPI5CAR_RGB_LED_RED_MASK 0xFF0000U
/** @brief Bit mask selecting the green component from packed `0xRRGGBB` data. */
#define XHAL_RPI5CAR_RGB_LED_GREEN_MASK 0x00FF00U
/** @brief Bit mask selecting the blue component from packed `0xRRGGBB` data. */
#define XHAL_RPI5CAR_RGB_LED_BLUE_MASK 0x0000FFU
/** @brief Bit displacement of the red component in packed `0xRRGGBB` data. */
#define XHAL_RPI5CAR_RGB_LED_RED_SHIFT 16U
/** @brief Bit displacement of the green component in packed `0xRRGGBB` data. */
#define XHAL_RPI5CAR_RGB_LED_GREEN_SHIFT 8U
/** @brief Bit displacement of the blue component in packed `0xRRGGBB` data. */
#define XHAL_RPI5CAR_RGB_LED_BLUE_SHIFT 0U
/** @brief Number of hexadecimal digits required for one RGB color. */
#define XHAL_RPI5CAR_RGB_LED_HEX_DIGIT_COUNT 6U
/** @brief Scale converting normalized LED intensity to PWM duty-cycle percent. */
#define XHAL_RPI5CAR_RGB_LED_PERCENT_SCALE 100.0
/** @brief Signed 16-bit mono PCM sample rate used by generated music tones, in Hertz. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_RATE_HZ 44'100U
/** @brief Number of interleaved channels in generated music tones. */
#define XHAL_RPI5CAR_MUSIC_CHANNEL_COUNT 1U
/** @brief Number of bytes in one generated signed 16-bit PCM sample. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_BYTES 2U
/** @brief Most significant byte position in one little-endian PCM sample. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_HIGH_BYTE_INDEX 1U
/** @brief Least significant byte position in one little-endian PCM sample. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_LOW_BYTE_INDEX 0U
/** @brief Maximum number of PCM streams owned by one shared audio backend. */
#define XHAL_RPI5CAR_AUDIO_MAXIMUM_STREAM_COUNT 8U
/** @brief Maximum interleaved channel count accepted by the shared audio backend. */
#define XHAL_RPI5CAR_AUDIO_MAXIMUM_CHANNEL_COUNT 8U
/** @brief Maximum ALSA period size accepted by the shared audio backend, in frames. */
#define XHAL_RPI5CAR_AUDIO_MAXIMUM_PERIOD_FRAMES 4'096U
/** @brief Default ALSA output latency requested by the shared audio backend, in microseconds. */
#define XHAL_RPI5CAR_AUDIO_DEFAULT_LATENCY_US 100'000U
/** @brief Maximum ALSA underrun recovery attempts for one bounded write. */
#define XHAL_RPI5CAR_AUDIO_RECOVERY_ATTEMPT_COUNT 3U
/** @brief Mask selecting one byte from a packed signed PCM sample. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_BYTE_MASK 0xFFU
/** @brief Bit displacement of the high byte in a signed 16-bit PCM sample. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_HIGH_BYTE_SHIFT 8U
/** @brief Positive peak magnitude of a signed 16-bit PCM waveform. */
#define XHAL_RPI5CAR_MUSIC_SAMPLE_PEAK 32'767.0
/** @brief Complete angular period of a sine waveform, in radians. */
#define XHAL_RPI5CAR_MUSIC_TWO_PI_RADIANS 6.28318530717958647692
/** @brief Default upper time-signature value. */
#define XHAL_RPI5CAR_MUSIC_DEFAULT_TIME_SIGNATURE_TOP 4U
/** @brief Default lower time-signature note denominator. */
#define XHAL_RPI5CAR_MUSIC_DEFAULT_TIME_SIGNATURE_BOTTOM 4U
/** @brief Default tempo in beats per minute. */
#define XHAL_RPI5CAR_MUSIC_DEFAULT_TEMPO_BPM 120.0
/** @brief Number of elements in a music time-signature or tempo pair. */
#define XHAL_RPI5CAR_MUSIC_PAIR_VALUE_COUNT 2U
/** @brief Index of the upper value in a music time-signature pair. */
#define XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_TOP_INDEX 0U
/** @brief Index of the lower value in a music time-signature pair. */
#define XHAL_RPI5CAR_MUSIC_TIME_SIGNATURE_BOTTOM_INDEX 1U
/** @brief Index of beats per minute in a music tempo pair. */
#define XHAL_RPI5CAR_MUSIC_TEMPO_BPM_INDEX 0U
/** @brief Index of the whole-note beat value in a music tempo pair. */
#define XHAL_RPI5CAR_MUSIC_TEMPO_NOTE_VALUE_INDEX 1U
/** @brief Whole-note duration represented as a fraction of one whole note. */
#define XHAL_RPI5CAR_MUSIC_WHOLE_NOTE 1.0
/** @brief Half-note duration represented as a fraction of one whole note. */
#define XHAL_RPI5CAR_MUSIC_HALF_NOTE 0.5
/** @brief Quarter-note duration represented as a fraction of one whole note. */
#define XHAL_RPI5CAR_MUSIC_QUARTER_NOTE 0.25
/** @brief Eighth-note duration represented as a fraction of one whole note. */
#define XHAL_RPI5CAR_MUSIC_EIGHTH_NOTE 0.125
/** @brief Sixteenth-note duration represented as a fraction of one whole note. */
#define XHAL_RPI5CAR_MUSIC_SIXTEENTH_NOTE 0.0625
/** @brief A4 reference frequency used for equal-temperament conversion, in Hertz. */
#define XHAL_RPI5CAR_MUSIC_NOTE_BASE_FREQUENCY_HZ 440.0
/** @brief MIDI note number assigned to the A4 frequency reference. */
#define XHAL_RPI5CAR_MUSIC_NOTE_BASE_INDEX 69
/** @brief Lowest numeric MIDI note accepted by the frequency interface. */
#define XHAL_RPI5CAR_MUSIC_MINIMUM_NOTE_INDEX 0
/** @brief Lowest MIDI note represented by the Python-compatible note names. */
#define XHAL_RPI5CAR_MUSIC_FIRST_NAMED_NOTE_INDEX 21
/** @brief Highest MIDI note accepted by the frequency interface. */
#define XHAL_RPI5CAR_MUSIC_MAXIMUM_NOTE_INDEX 108
/** @brief Number of equal-temperament semitones in one octave. */
#define XHAL_RPI5CAR_MUSIC_SEMITONES_PER_OCTAVE 12.0
/** @brief Lowest supported key-signature displacement, in semitones. */
#define XHAL_RPI5CAR_MUSIC_MINIMUM_KEY_SIGNATURE (-7)
/** @brief Highest supported key-signature displacement, in semitones. */
#define XHAL_RPI5CAR_MUSIC_MAXIMUM_KEY_SIGNATURE 7
/** @brief One-sharp key signature corresponding to G major. */
#define XHAL_RPI5CAR_MUSIC_KEY_G_MAJOR 1
/** @brief Two-sharp key signature corresponding to D major. */
#define XHAL_RPI5CAR_MUSIC_KEY_D_MAJOR 2
/** @brief Three-sharp key signature corresponding to A major. */
#define XHAL_RPI5CAR_MUSIC_KEY_A_MAJOR 3
/** @brief Four-sharp key signature corresponding to E major. */
#define XHAL_RPI5CAR_MUSIC_KEY_E_MAJOR 4
/** @brief Five-sharp key signature corresponding to B major. */
#define XHAL_RPI5CAR_MUSIC_KEY_B_MAJOR 5
/** @brief Six-sharp key signature corresponding to F-sharp major. */
#define XHAL_RPI5CAR_MUSIC_KEY_F_SHARP_MAJOR 6
/** @brief Seven-sharp key signature corresponding to C-sharp major. */
#define XHAL_RPI5CAR_MUSIC_KEY_C_SHARP_MAJOR 7
/** @brief One-flat key signature corresponding to F major. */
#define XHAL_RPI5CAR_MUSIC_KEY_F_MAJOR (-1)
/** @brief Two-flat key signature corresponding to B-flat major. */
#define XHAL_RPI5CAR_MUSIC_KEY_B_FLAT_MAJOR (-2)
/** @brief Three-flat key signature corresponding to E-flat major. */
#define XHAL_RPI5CAR_MUSIC_KEY_E_FLAT_MAJOR (-3)
/** @brief Four-flat key signature corresponding to A-flat major. */
#define XHAL_RPI5CAR_MUSIC_KEY_A_FLAT_MAJOR (-4)
/** @brief Five-flat key signature corresponding to D-flat major. */
#define XHAL_RPI5CAR_MUSIC_KEY_D_FLAT_MAJOR (-5)
/** @brief Six-flat key signature corresponding to G-flat major. */
#define XHAL_RPI5CAR_MUSIC_KEY_G_FLAT_MAJOR (-6)
/** @brief Seven-flat key signature corresponding to C-flat major. */
#define XHAL_RPI5CAR_MUSIC_KEY_C_FLAT_MAJOR (-7)
/** @brief Positive semitone direction used by textual sharp signatures. */
#define XHAL_RPI5CAR_MUSIC_KEY_SIGNATURE_SHARP 1
/** @brief Negative semitone direction used by textual flat signatures. */
#define XHAL_RPI5CAR_MUSIC_KEY_SIGNATURE_FLAT (-1)
/** @brief Lowest accepted audio volume, in percent. */
#define XHAL_RPI5CAR_MUSIC_MINIMUM_VOLUME_PERCENT 0.0
/** @brief Highest accepted audio volume, in percent. */
#define XHAL_RPI5CAR_MUSIC_MAXIMUM_VOLUME_PERCENT 100.0
/** @brief Divisor converting percent volume to a normalized backend value. */
#define XHAL_RPI5CAR_MUSIC_VOLUME_PERCENT_DIVISOR 100.0
/** @brief Scale used for Python-compatible rounding to two decimal places. */
#define XHAL_RPI5CAR_MUSIC_HUNDREDTH_SCALE 100.0
/** @brief Compatibility divisor applied to requested generated-tone duration. */
#define XHAL_RPI5CAR_MUSIC_TONE_DURATION_DIVISOR 2.0
/** @brief Number of seconds in one minute for tempo conversion. */
#define XHAL_RPI5CAR_MUSIC_SECONDS_PER_MINUTE 60.0
/** @brief Maximum frames written by the ALSA music adapter in one operation. */
#define XHAL_RPI5CAR_MUSIC_ALSA_PERIOD_FRAMES 1'024U
/** @brief Delay between paused music state checks in milliseconds. */
#define XHAL_RPI5CAR_MUSIC_ALSA_PAUSE_POLL_INTERVAL_MS 10U
/** @brief Byte count of the fixed RIFF and WAVE identifiers. */
#define XHAL_RPI5CAR_MUSIC_WAVE_IDENTIFIER_BYTES 4U
/** @brief Minimum byte count of a PCM WAVE format chunk. */
#define XHAL_RPI5CAR_MUSIC_WAVE_FORMAT_CHUNK_BYTES 16U
/** @brief Numeric WAVE format identifier for uncompressed PCM. */
#define XHAL_RPI5CAR_MUSIC_WAVE_PCM_FORMAT 1U
/** @brief Supported sample width for music WAVE playback, in bits. */
#define XHAL_RPI5CAR_MUSIC_WAVE_SAMPLE_BITS 16U
/** @brief Maximum concurrently retained playback tasks in one speaker controller. */
#define XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT 8U
/** @brief Audio frames passed to the speaker backend in one bounded write operation. */
#define XHAL_RPI5CAR_SPEAKER_CHUNK_FRAME_COUNT 1'024U
/** @brief Delay between paused-task state checks in milliseconds. */
#define XHAL_RPI5CAR_SPEAKER_PAUSE_POLL_INTERVAL_MS 10U
/** @brief Index returned internally when no speaker task slot matches a request. */
#define XHAL_RPI5CAR_SPEAKER_INVALID_TASK_INDEX XHAL_RPI5CAR_SPEAKER_MAXIMUM_TASK_COUNT
/** @brief Maximum bytes read by the built-in bounded speaker decoder. */
#define XHAL_RPI5CAR_SPEAKER_MAXIMUM_INPUT_BYTES 16'777'216U
/** @brief Maximum interleaved samples retained by one decoded speaker task. */
#define XHAL_RPI5CAR_SPEAKER_MAXIMUM_DECODED_SAMPLE_COUNT 2'000'000U
/** @brief Numeric WAVE format identifier for uncompressed integer PCM. */
#define XHAL_RPI5CAR_SPEAKER_WAVE_PCM_FORMAT 1U
/** @brief Supported integer sample width for built-in WAVE decoding, in bits. */
#define XHAL_RPI5CAR_SPEAKER_WAVE_SAMPLE_BITS 16U
/** @brief Positive divisor normalizing signed sixteen-bit PCM samples. */
#define XHAL_RPI5CAR_SPEAKER_PCM16_NORMALIZATION_DIVISOR 32'768.0
/** @brief Default Linux firmware device-tree root used for Robot HAT discovery. */
#define XHAL_RPI5CAR_DEVICE_TREE_ROOT "/proc/device-tree"
/** @brief Substring identifying candidate HAT nodes below the device-tree root. */
#define XHAL_RPI5CAR_DEVICE_HAT_NODE_MARKER "hat"
/** @brief UUID identifying the supported Robot HAT v5 hardware revision. */
#define XHAL_RPI5CAR_DEVICE_ROBOT_HAT_V5_UUID "9daeea78-0000-076e-0032-582369ac3e02"
/** @brief Device-tree property containing the HAT product name. */
#define XHAL_RPI5CAR_DEVICE_PRODUCT_PROPERTY "product"
/** @brief Device-tree property containing the hexadecimal HAT product identifier. */
#define XHAL_RPI5CAR_DEVICE_PRODUCT_ID_PROPERTY "product_id"
/** @brief Device-tree property containing the hexadecimal HAT product version. */
#define XHAL_RPI5CAR_DEVICE_PRODUCT_VERSION_PROPERTY "product_ver"
/** @brief Device-tree property containing the HAT UUID. */
#define XHAL_RPI5CAR_DEVICE_UUID_PROPERTY "uuid"
/** @brief Device-tree property containing the HAT vendor name. */
#define XHAL_RPI5CAR_DEVICE_VENDOR_PROPERTY "vendor"
/** @brief Default and Robot HAT v4 speaker-enable GPIO line offset. */
#define XHAL_RPI5CAR_DEVICE_DEFAULT_SPEAKER_ENABLE_PIN 20U
/** @brief Robot HAT v5 speaker-enable GPIO line offset. */
#define XHAL_RPI5CAR_DEVICE_V5_SPEAKER_ENABLE_PIN 12U
/** @brief GPIO line connected to the Robot HAT microcontroller reset input. */
#define XHAL_RPI5CAR_BOARD_CONTROL_MCU_RESET_PIN 5U
/** @brief ADC channel connected to the divided Robot HAT battery supply. */
#define XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_ADC_CHANNEL 4U
/** @brief Delay maintaining each MCU reset level in milliseconds. */
#define XHAL_RPI5CAR_BOARD_CONTROL_RESET_INTERVAL_MS 10U
/** @brief Ratio converting divided ADC voltage to battery voltage. */
#define XHAL_RPI5CAR_BOARD_CONTROL_BATTERY_DIVIDER_RATIO 3.0
/** @brief Speaker priming duration required after enabling power, in milliseconds. */
#define XHAL_RPI5CAR_BOARD_CONTROL_SPEAKER_PRIME_DURATION_MS 500U
/** @brief First Robot HAT address considered for firmware-version acquisition. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_1 XHAL_RPI5CAR_I2C_ADDRESS_1
/** @brief Second Robot HAT address considered for firmware-version acquisition. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_ADDRESS_2 XHAL_RPI5CAR_I2C_ADDRESS_2
/** @brief Register containing the first Robot HAT firmware-version byte. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_REGISTER 0x05U
/** @brief Number of major, minor, and patch bytes in the firmware version. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_VERSION_BYTE_COUNT 3U
/** @brief Zero-based index of the firmware major-version byte. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_MAJOR_INDEX 0U
/** @brief Zero-based index of the firmware minor-version byte. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_MINOR_INDEX 1U
/** @brief Zero-based index of the firmware patch-version byte. */
#define XHAL_RPI5CAR_FIRMWARE_INFO_PATCH_INDEX 2U
/** @brief Robot HAT Python library version represented by this compatibility port. */
#define XHAL_RPI5CAR_ROBOT_HAT_LIBRARY_VERSION "2.5.5"
/** @brief Default and Robot HAT v4 motor-driver mode identifier. */
#define XHAL_RPI5CAR_DEVICE_DEFAULT_MOTOR_MODE 1U
/** @brief Robot HAT v5 motor-driver mode identifier. */
#define XHAL_RPI5CAR_DEVICE_V5_MOTOR_MODE 2U
/** @brief Maximum hexadecimal digits accepted for a 32-bit device-tree value. */
#define XHAL_RPI5CAR_DEVICE_MAXIMUM_HEX_DIGITS 8U
/** @brief Duty cycle that energizes a passive buzzer with a symmetric waveform. */
#define XHAL_RPI5CAR_BUZZER_ON_DUTY_CYCLE_PERCENT 50.0
/** @brief Duty cycle that disables a passive buzzer output. */
#define XHAL_RPI5CAR_BUZZER_OFF_DUTY_CYCLE_PERCENT 0.0
/** @brief Scale converting buzzer playback duration from seconds to microseconds. */
#define XHAL_RPI5CAR_BUZZER_MICROSECONDS_PER_SECOND 1'000'000.0
/** @brief Divisor splitting a finite buzzer duration into sounding and silent halves. */
#define XHAL_RPI5CAR_BUZZER_DURATION_HALF_DIVISOR 2.0
/** @brief Default number of complete on/off cycles in one LED blink sequence. */
#define XHAL_RPI5CAR_LED_DEFAULT_BLINK_COUNT 1U
/** @brief Default interval between LED transitions in seconds. */
#define XHAL_RPI5CAR_LED_DEFAULT_TOGGLE_DELAY_SECONDS 0.1
/** @brief Default inactive pause after one LED blink sequence in seconds. */
#define XHAL_RPI5CAR_LED_DEFAULT_PAUSE_SECONDS 0.0
/** @brief Scale converting LED blink timing from seconds to microseconds. */
#define XHAL_RPI5CAR_LED_MICROSECONDS_PER_SECOND 1'000'000.0
/** @brief Number of LED transitions required for one complete blink cycle. */
#define XHAL_RPI5CAR_LED_TOGGLES_PER_CYCLE 2U
/** @brief Highest blink count whose transition count fits the project `uint32` type. */
#define XHAL_RPI5CAR_LED_MAX_BLINK_COUNT \
    (XHAL_RPI5CAR_UINT32_MAX / XHAL_RPI5CAR_LED_TOGGLES_PER_CYCLE)
/** @brief Delay chunk bounding LED blink-worker stop latency, in microseconds. */
#define XHAL_RPI5CAR_LED_STOP_POLL_INTERVAL_US 10'000U
/** @brief Logical GPIO level representing a pressed pull-up user button. */
#define XHAL_RPI5CAR_USER_BUTTON_PRESSED_LEVEL false
/** @brief User-button input polling interval in milliseconds. */
#define XHAL_RPI5CAR_USER_BUTTON_POLL_INTERVAL_MS 50U
/** @brief Default user-button long-press threshold in seconds. */
#define XHAL_RPI5CAR_USER_BUTTON_DEFAULT_LONG_PRESS_SECONDS 2.0
/** @brief Lowest accepted user-button long-press threshold in seconds. */
#define XHAL_RPI5CAR_USER_BUTTON_MIN_LONG_PRESS_SECONDS 2.0
/** @brief Highest accepted user-button long-press threshold in seconds. */
#define XHAL_RPI5CAR_USER_BUTTON_MAX_LONG_PRESS_SECONDS 5.0
/** @brief Scale converting user-button timing from seconds to microseconds. */
#define XHAL_RPI5CAR_USER_BUTTON_MICROSECONDS_PER_SECOND 1'000'000.0
/** @brief Maximum value accepted by a 16-bit Robot HAT register. */
#define XHAL_RPI5CAR_UINT16_MAX            0xFFFFU
/** @brief Maximum value representable by the project unsigned 32-bit type. */
#define XHAL_RPI5CAR_UINT32_MAX            0xFFFFFFFFU
/** @brief First supported Robot HAT seven-bit I2C address. */
#define XHAL_RPI5CAR_I2C_ADDRESS_1         0x14U
/** @brief Second supported Robot HAT seven-bit I2C address. */
#define XHAL_RPI5CAR_I2C_ADDRESS_2         0x15U
/** @brief Third supported Robot HAT seven-bit I2C address. */
#define XHAL_RPI5CAR_I2C_ADDRESS_3         0x16U
/** @brief Default Linux device node used for Raspberry Pi I2C bus one. */
#define XHAL_RPI5CAR_I2C_DEFAULT_DEVICE    "/dev/i2c-1"
/** @brief Default Linux SPI controller and chip-select device. */
#define XHAL_RPI5CAR_SPI_DEFAULT_DEVICE "/dev/spidev0.0"
/** @brief Default SPI clock frequency in Hertz. */
#define XHAL_RPI5CAR_SPI_DEFAULT_SPEED_HZ 500'000U
/** @brief Default SPI clock-polarity and clock-phase mode. */
#define XHAL_RPI5CAR_SPI_DEFAULT_MODE 0U
/** @brief Default number of bits transferred in one SPI word. */
#define XHAL_RPI5CAR_SPI_DEFAULT_BITS_PER_WORD 8U
/** @brief Highest standard SPI mode accepted by the backend. */
#define XHAL_RPI5CAR_SPI_MAXIMUM_MODE 3U
/** @brief Highest bits-per-word value representable by Linux `spidev`. */
#define XHAL_RPI5CAR_SPI_MAXIMUM_BITS_PER_WORD 32U
/** @brief Maximum bytes permitted in one bounded SPI transfer. */
#define XHAL_RPI5CAR_SPI_MAXIMUM_TRANSFER_BYTES 256U
/** @brief Number of attempts made for a Linux I2C operation. */
#define XHAL_RPI5CAR_I2C_RETRY_COUNT       5U
/** @brief Maximum SMBus block payload length in bytes. */
#define XHAL_RPI5CAR_I2C_SMBUS_BLOCK_MAX   32U
/** @brief First supported ADC seven-bit I2C address. */
#define XHAL_RPI5CAR_ADC_ADDRESS_1          XHAL_RPI5CAR_I2C_ADDRESS_1
/** @brief Second supported ADC seven-bit I2C address. */
#define XHAL_RPI5CAR_ADC_ADDRESS_2          XHAL_RPI5CAR_I2C_ADDRESS_2
/** @brief Highest supported ADC input channel index. */
#define XHAL_RPI5CAR_ADC_MAX_CHANNEL        7U
/** @brief Command bit selecting an ADC conversion read. */
#define XHAL_RPI5CAR_ADC_READ_COMMAND       0x10U
/** @brief Number of bytes returned for one ADC sample. */
#define XHAL_RPI5CAR_ADC_READ_LENGTH        2U
/** @brief ADC reference potential in volts. */
#define XHAL_RPI5CAR_ADC_REFERENCE_VOLTAGE  3.3
/** @brief Maximum count produced by the 12-bit ADC. */
#define XHAL_RPI5CAR_ADC_MAX_COUNT          4095U
/** @brief Default seven-bit I2C address of the ADXL345 accelerometer. */
#define XHAL_RPI5CAR_ADXL345_ADDRESS 0x53U
/** @brief Number of orthogonal acceleration axes reported by the ADXL345. */
#define XHAL_RPI5CAR_ADXL345_AXIS_COUNT 3U
/** @brief Zero-based index of the ADXL345 X axis. */
#define XHAL_RPI5CAR_ADXL345_X_AXIS 0U
/** @brief Zero-based index of the ADXL345 Y axis. */
#define XHAL_RPI5CAR_ADXL345_Y_AXIS 1U
/** @brief Zero-based index of the ADXL345 Z axis. */
#define XHAL_RPI5CAR_ADXL345_Z_AXIS 2U
/** @brief First little-endian data register for the ADXL345 X axis. */
#define XHAL_RPI5CAR_ADXL345_DATA_X_REGISTER 0x32U
/** @brief First little-endian data register for the ADXL345 Y axis. */
#define XHAL_RPI5CAR_ADXL345_DATA_Y_REGISTER 0x34U
/** @brief First little-endian data register for the ADXL345 Z axis. */
#define XHAL_RPI5CAR_ADXL345_DATA_Z_REGISTER 0x36U
/** @brief ADXL345 data-format configuration register. */
#define XHAL_RPI5CAR_ADXL345_DATA_FORMAT_REGISTER 0x31U
/** @brief ADXL345 power-control configuration register. */
#define XHAL_RPI5CAR_ADXL345_POWER_CONTROL_REGISTER 0x2DU
/** @brief Data-format value preserving the Python port's default range. */
#define XHAL_RPI5CAR_ADXL345_DATA_FORMAT_VALUE 0U
/** @brief Power-control value enabling ADXL345 measurement mode. */
#define XHAL_RPI5CAR_ADXL345_MEASUREMENT_MODE_VALUE 0x08U
/** @brief Number of little-endian bytes in one ADXL345 axis sample. */
#define XHAL_RPI5CAR_ADXL345_SAMPLE_LENGTH 2U
/** @brief Signed ADXL345 counts corresponding to one standard gravity. */
#define XHAL_RPI5CAR_ADXL345_COUNTS_PER_G 256.0
/** @brief Sign bit in one packed 16-bit ADXL345 sample. */
#define XHAL_RPI5CAR_ADXL345_SIGN_BIT 0x8000U
/** @brief Modulus used to sign-extend a packed 16-bit ADXL345 sample. */
#define XHAL_RPI5CAR_ADXL345_SIGNED_MODULUS 65'536
/** @brief Number of analog channels used by a Robot HAT line tracker. */
#define XHAL_RPI5CAR_LINE_TRACKER_CHANNEL_COUNT 3U
/** @brief Zero-based index of the left line-tracker channel. */
#define XHAL_RPI5CAR_LINE_TRACKER_LEFT_CHANNEL 0U
/** @brief Zero-based index of the middle line-tracker channel. */
#define XHAL_RPI5CAR_LINE_TRACKER_MIDDLE_CHANNEL 1U
/** @brief Zero-based index of the right line-tracker channel. */
#define XHAL_RPI5CAR_LINE_TRACKER_RIGHT_CHANNEL 2U
/** @brief Default grayscale status threshold in ADC counts. */
#define XHAL_RPI5CAR_GRAYSCALE_DEFAULT_REFERENCE 1'000
/** @brief Minimum channel spread recognized as a line, in calibrated counts. */
#define XHAL_RPI5CAR_LINE_TRACKER_LINE_DIFFERENCE 200
/** @brief Default cliff threshold in calibrated ADC counts. */
#define XHAL_RPI5CAR_LINE_TRACKER_DEFAULT_CLIFF_THRESHOLD 120
/** @brief Initial background reference in calibrated ADC counts. */
#define XHAL_RPI5CAR_LINE_TRACKER_BACKGROUND_REFERENCE 1'000.0
/** @brief Initial dark-line reference in calibrated ADC counts. */
#define XHAL_RPI5CAR_LINE_TRACKER_LINE_REFERENCE 200.0
/** @brief Fraction of a new sample applied to adaptive references. */
#define XHAL_RPI5CAR_LINE_TRACKER_REFERENCE_UPDATE_RATE 0.05
/** @brief Lowest normalized sensor weight considered meaningful. */
#define XHAL_RPI5CAR_LINE_TRACKER_MINIMUM_WEIGHT 0.2
/** @brief Edge-only weight offset used to extrapolate line position. */
#define XHAL_RPI5CAR_LINE_TRACKER_EDGE_WEIGHT_OFFSET 1.8
/** @brief Divisor mapping weighted position into the normalized output range. */
#define XHAL_RPI5CAR_LINE_TRACKER_POSITION_DIVISOR 1.5
/** @brief Lowest normalized line position. */
#define XHAL_RPI5CAR_LINE_TRACKER_MINIMUM_POSITION (-1.0)
/** @brief Highest normalized line position. */
#define XHAL_RPI5CAR_LINE_TRACKER_MAXIMUM_POSITION 1.0
/** @brief Decimal scale used to round calibration and position values. */
#define XHAL_RPI5CAR_LINE_TRACKER_ROUNDING_SCALE 100.0
/** @brief Default Linux GPIO character device for the first controller. */
#define XHAL_RPI5CAR_GPIO_DEFAULT_DEVICE    "/dev/gpiochip0"
/** @brief Default GPIO interrupt debounce interval in milliseconds. */
#define XHAL_RPI5CAR_GPIO_DEFAULT_DEBOUNCE_MS 200U
/** @brief GPIO event polling interval used to bound backend shutdown latency, in milliseconds. */
#define XHAL_RPI5CAR_GPIO_EVENT_POLL_MS     50
/** @brief Speed of sound used for ultrasonic conversion in meters per second. */
#define XHAL_RPI5CAR_ULTRASONIC_SOUND_SPEED_MPS (343.3)
/** @brief Ultrasonic inactive settling interval in microseconds. */
#define XHAL_RPI5CAR_ULTRASONIC_SETTLE_TIME_US (1'000U)
/** @brief Ultrasonic active trigger-pulse duration in microseconds. */
#define XHAL_RPI5CAR_ULTRASONIC_TRIGGER_TIME_US (10U)
/** @brief Default maximum ultrasonic echo acquisition interval in microseconds. */
#define XHAL_RPI5CAR_ULTRASONIC_DEFAULT_TIMEOUT_US (20'000U)
/** @brief Default attempts made by one ultrasonic reading operation. */
#define XHAL_RPI5CAR_ULTRASONIC_DEFAULT_ATTEMPTS (10U)
/** @brief Ultrasonic result in centimeters when every attempt times out. */
#define XHAL_RPI5CAR_ULTRASONIC_TIMEOUT_RESULT_CM (-1.0)
/** @brief Ultrasonic result in centimeters when an echo pulse is incomplete. */
#define XHAL_RPI5CAR_ULTRASONIC_INVALID_PULSE_RESULT_CM (-2.0)
/** @brief Default Robot HAT motor PWM frequency in Hertz. */
#define XHAL_RPI5CAR_MOTOR_DEFAULT_FREQUENCY_HZ 100.0
/** @brief Minimum signed motor command in percent. */
#define XHAL_RPI5CAR_MOTOR_MIN_SPEED_PERCENT (-100.0)
/** @brief Maximum signed motor command in percent. */
#define XHAL_RPI5CAR_MOTOR_MAX_SPEED_PERCENT 100.0
/** @brief First valid one-based motor identifier. */
#define XHAL_RPI5CAR_MOTOR_FIRST_ID          1U
/** @brief Second valid one-based motor identifier. */
#define XHAL_RPI5CAR_MOTOR_SECOND_ID         2U
/** @brief Minimum supported servo angle in degrees. */
#define XHAL_RPI5CAR_SERVO_MIN_ANGLE_DEG    (-90.0)
/** @brief Maximum supported servo angle in degrees. */
#define XHAL_RPI5CAR_SERVO_MAX_ANGLE_DEG    90.0
/** @brief Minimum servo command pulse duration in microseconds. */
#define XHAL_RPI5CAR_SERVO_MIN_PULSE_US     500.0
/** @brief Maximum servo command pulse duration in microseconds. */
#define XHAL_RPI5CAR_SERVO_MAX_PULSE_US     2500.0
/** @brief Servo PWM frame duration at 50 Hertz, in microseconds. */
#define XHAL_RPI5CAR_SERVO_FRAME_US         20000.0
/** @brief Servo PWM frequency in Hertz. */
#define XHAL_RPI5CAR_SERVO_FREQUENCY_HZ     50.0
/** @brief Servo PWM period in timer-count units. */
#define XHAL_RPI5CAR_SERVO_PERIOD           4095.0
/** @brief Lowest system-volume percentage accepted by the utilities module. */
#define XHAL_RPI5CAR_UTILS_MINIMUM_VOLUME_PERCENT 0
/** @brief Highest system-volume percentage accepted by the utilities module. */
#define XHAL_RPI5CAR_UTILS_MAXIMUM_VOLUME_PERCENT 100
/** @brief Default lazy-reader refresh interval in milliseconds. */
#define XHAL_RPI5CAR_UTILS_DEFAULT_LAZY_INTERVAL_MS 10'000U
/** @brief Number of microseconds represented by one millisecond. */
#define XHAL_RPI5CAR_UTILS_MICROSECONDS_PER_MILLISECOND 1'000U

#endif /* XHAL_RPI5CAR_COMMON_H */
