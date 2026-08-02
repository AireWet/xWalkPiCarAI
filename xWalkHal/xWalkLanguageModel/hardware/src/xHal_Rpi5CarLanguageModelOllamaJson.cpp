/******************************************************************************
 * @file        xHal_Rpi5CarLanguageModelOllamaJson.cpp
 * @brief       Implements bounded Ollama JSON and image conversion.
 *
 * @details
 * Serializes non-streaming chat requests, base64-encodes bounded images, and
 * decodes final assistant content including escaped Unicode values.
 *
 * @project     xWalk Firmware
 * @module      xWalkLanguageModel Ollama Backend
 *
 * @author      Joxy John
 * @date        2026-08-01
 * @version     1.0.0
 *
 * @copyright
 * Copyright (c) 2026 Joxy John.
 * All rights reserved.
 *
 * @note
 * Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLanguageModelOllama.h"

#include "xHal_Rpi5CarFileFunctions.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

/**
 * @namespace xwalk::hal
 * @brief Contains hardware abstraction components for the xWalk firmware.
 */
namespace xwalk::hal
{

/******************************************************************************
 * Protected member function definitions
 ******************************************************************************/

/**
 * @brief Reads and base64-encodes one bounded image.
 *
 * @param[in] imagePath Empty path or existing image file path.
 * @return Empty text for no image, otherwise base64 data without a URI prefix.
 * @throws std::out_of_range If the raw image limit is exceeded.
 * @throws std::runtime_error If the image cannot be read.
 */
string XWalkLanguageModelOllama::encodeImage(stringview imagePath)
{
    if (imagePath.empty())
    {
        return {};
    }
    const filesystempath path{imagePath};
    if (filesystemFileSize(path) > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_IMAGE_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama image exceeds its bounded byte count");
    }
    const string contents = readFileContents(path);
    if (contents.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_IMAGE_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama image changed beyond its bounded byte count");
    }
    constexpr stringview alphabet{
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};
    const size encodedGroups = (contents.size() + 2U) / 3U;
    string encoded{};
    encoded.reserve(encodedGroups * 4U);
    size byteOffset{};
    while (byteOffset < contents.size())
    {
        const uint32 first = static_cast<uint8>(
            static_cast<unsigned char>(contents[byteOffset]));
        const boolean hasSecond = (byteOffset + 1U) < contents.size();
        const boolean hasThird = (byteOffset + 2U) < contents.size();
        const uint32 second = hasSecond ? static_cast<uint8>(
            static_cast<unsigned char>(contents[byteOffset + 1U])) : 0U;
        const uint32 third = hasThird ? static_cast<uint8>(
            static_cast<unsigned char>(contents[byteOffset + 2U])) : 0U;
        const uint32 combined = (first << 16U) | (second << 8U) | third;
        encoded.push_back(alphabet[(combined >> 18U) & 0x3FU]);
        encoded.push_back(alphabet[(combined >> 12U) & 0x3FU]);
        encoded.push_back(hasSecond ? alphabet[(combined >> 6U) & 0x3FU] : '=');
        encoded.push_back(hasThird ? alphabet[combined & 0x3FU] : '=');
        byteOffset += 3U;
    }
    return encoded;
}

/**
 * @brief Serializes one complete non-streaming Ollama chat request.
 *
 * @param[in] currentPrompt Validated current user message.
 * @return Complete JSON request within the configured byte limit.
 * @throws std::out_of_range If serialization exceeds the request limit.
 */
string XWalkLanguageModelOllama::buildRequest(
    const XWalkLanguageModelOllamaMessage& currentPrompt) const
{
    string request{"{\"model\":"};
    appendJsonString(request, modelValue);
    request += ",\"stream\":false";
    if (dialectValue == XWalkLanguageModelHttpDialect::OpenAiChatCompletions)
    {
        request += ",\"max_tokens\":";
        request += std::to_string(maximumOutputTokensValue);
    }
    request += ",\"messages\":[";
    boolean needsComma = false;
    const auto appendWithSeparator = [&](const XWalkLanguageModelOllamaMessage& message)
    {
        if (needsComma)
        {
            request.push_back(',');
        }
        appendMessageJson(request, message);
        needsComma = true;
    };
    if (!instructionsValue.empty())
    {
        appendWithSeparator({XWalkLanguageModelRole::System, instructionsValue, {}});
    }
    if (!welcomeValue.empty())
    {
        appendWithSeparator({XWalkLanguageModelRole::Assistant, welcomeValue, {}});
    }
    for (const XWalkLanguageModelOllamaMessage& message : history)
    {
        appendWithSeparator(message);
    }
    appendWithSeparator(currentPrompt);
    request += "]}";
    if (request.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_REQUEST_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama request exceeds its bounded byte count");
    }
    return request;
}

/**
 * @brief Appends one JSON string with escaping and request-bound checks.
 *
 * @param[in,out] output Request JSON receiving escaped bytes.
 * @param[in] value UTF-8 bytes retained only for this call.
 * @throws std::out_of_range If serialization exceeds the request limit.
 */
void XWalkLanguageModelOllama::appendJsonString(string& output, stringview value)
{
    constexpr stringview hexadecimal{"0123456789ABCDEF"};
    output.push_back('"');
    for (const char character : value)
    {
        const uint8 byte = static_cast<uint8>(static_cast<unsigned char>(character));
        if ((character == '"') || (character == '\\'))
        {
            output.push_back('\\');
            output.push_back(character);
        }
        else if (byte < 0x20U)
        {
            output += "\\u00";
            output.push_back(hexadecimal[(byte >> 4U) & 0x0FU]);
            output.push_back(hexadecimal[byte & 0x0FU]);
        }
        else
        {
            output.push_back(character);
        }
        if (output.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_REQUEST_BYTES)
        {
            XHAL_THROW_OUT_OF_RANGE("Ollama request exceeds its bounded byte count");
        }
    }
    output.push_back('"');
}

/**
 * @brief Appends one complete Ollama message object to a JSON request.
 *
 * @param[in,out] output Request JSON receiving one message.
 * @param[in] message Validated owned message.
 * @throws std::out_of_range If serialization exceeds the request limit.
 */
void XWalkLanguageModelOllama::appendMessageJson(string& output,
    const XWalkLanguageModelOllamaMessage& message) const
{
    output += "{\"role\":";
    appendJsonString(output, roleName(message.role));
    output += ",\"content\":";
    if (message.imageBase64.empty())
    {
        appendJsonString(output, message.content);
    }
    else if (dialectValue == XWalkLanguageModelHttpDialect::Ollama)
    {
        appendJsonString(output, message.content);
        output += ",\"images\":[";
        appendJsonString(output, message.imageBase64);
        output.push_back(']');
    }
    else
    {
        output += "[{\"type\":\"text\",\"text\":";
        appendJsonString(output, message.content);
        output += "},{\"type\":\"image_url\",\"image_url\":{\"url\":";
        appendJsonString(output,
            string("data:image/jpeg;base64,") + message.imageBase64);
        output += "}}]";
    }
    output.push_back('}');
    if (output.size() > XHAL_RPI5CAR_LANGUAGE_MODEL_OLLAMA_MAXIMUM_REQUEST_BYTES)
    {
        XHAL_THROW_OUT_OF_RANGE("Ollama request exceeds its bounded byte count");
    }
}

/**
 * @brief Returns the Ollama JSON role name for one validated role.
 *
 * @param[in] role Valid system, user, or assistant role.
 * @return Static lowercase role name.
 * @throws std::invalid_argument If the role is unsupported.
 */
stringview XWalkLanguageModelOllama::roleName(XWalkLanguageModelRole role)
{
    validateRole(role);
    if (role == XWalkLanguageModelRole::System)
    {
        return "system";
    }
    if (role == XWalkLanguageModelRole::User)
    {
        return "user";
    }
    return "assistant";
}

/**
 * @brief Extracts and decodes `message.content` from an Ollama response.
 *
 * @param[in] responseJson Complete bounded provider response.
 * @return Owned decoded response content, which may be empty.
 * @throws std::runtime_error If required JSON structure is malformed or absent.
 */
string XWalkLanguageModelOllama::extractResponseContent(stringview responseJson)
{
    const size messageOffset = responseJson.find("\"message\"");
    const size contentOffset = (messageOffset == stringview::npos) ? stringview::npos :
        responseJson.find("\"content\"", messageOffset + 9U);
    const size colonOffset = (contentOffset == stringview::npos) ? stringview::npos :
        responseJson.find(':', contentOffset + 9U);
    if (colonOffset == stringview::npos)
    {
        XHAL_THROW_RUNTIME_ERROR("Ollama response has no assistant content");
    }
    size quoteOffset = colonOffset + 1U;
    while ((quoteOffset < responseJson.size()) &&
        ((responseJson[quoteOffset] == ' ') || (responseJson[quoteOffset] == '\t') ||
        (responseJson[quoteOffset] == '\r') || (responseJson[quoteOffset] == '\n')))
    {
        ++quoteOffset;
    }
    if ((quoteOffset >= responseJson.size()) || (responseJson[quoteOffset] != '"'))
    {
        XHAL_THROW_RUNTIME_ERROR("Ollama assistant content is not a JSON string");
    }
    return decodeJsonString(responseJson, quoteOffset);
}

/**
 * @brief Decodes one JSON string beginning at its opening quote.
 *
 * @param[in] json Complete JSON response.
 * @param[in] quoteOffset Offset of the opening quote.
 * @return Owned decoded Unicode text.
 * @throws std::runtime_error If escaping or Unicode data is malformed.
 */
string XWalkLanguageModelOllama::decodeJsonString(stringview json, size quoteOffset)
{
    string output{};
    size byteOffset = quoteOffset + 1U;
    while (byteOffset < json.size())
    {
        const char character = json[byteOffset];
        ++byteOffset;
        if (character == '"')
        {
            return output;
        }
        if (character != '\\')
        {
            if (static_cast<uint8>(static_cast<unsigned char>(character)) < 0x20U)
            {
                XHAL_THROW_RUNTIME_ERROR("Ollama response contains invalid JSON text");
            }
            output.push_back(character);
            continue;
        }
        if (byteOffset >= json.size())
        {
            XHAL_THROW_RUNTIME_ERROR("Ollama response ends during JSON escaping");
        }
        const char escape = json[byteOffset];
        ++byteOffset;
        if ((escape == '"') || (escape == '\\') || (escape == '/'))
        {
            output.push_back(escape);
        }
        else if (escape == 'b')
        {
            output.push_back('\b');
        }
        else if (escape == 'f')
        {
            output.push_back('\f');
        }
        else if (escape == 'n')
        {
            output.push_back('\n');
        }
        else if (escape == 'r')
        {
            output.push_back('\r');
        }
        else if (escape == 't')
        {
            output.push_back('\t');
        }
        else if (escape == 'u')
        {
            if ((json.size() - byteOffset) < 4U)
            {
                XHAL_THROW_RUNTIME_ERROR("Ollama response has incomplete Unicode escaping");
            }
            uint32 codePoint{};
            for (size digitOffset = 0U; digitOffset < 4U; ++digitOffset)
            {
                codePoint = (codePoint << 4U) | hexadecimalDigit(json[byteOffset + digitOffset]);
            }
            byteOffset += 4U;
            if ((codePoint >= 0xD800U) && (codePoint <= 0xDBFFU))
            {
                if (((json.size() - byteOffset) < 6U) || (json[byteOffset] != '\\') ||
                    (json[byteOffset + 1U] != 'u'))
                {
                    XHAL_THROW_RUNTIME_ERROR("Ollama response has incomplete Unicode pair");
                }
                uint32 lowSurrogate{};
                for (size digitOffset = 0U; digitOffset < 4U; ++digitOffset)
                {
                    lowSurrogate = (lowSurrogate << 4U) |
                        hexadecimalDigit(json[byteOffset + 2U + digitOffset]);
                }
                if ((lowSurrogate < 0xDC00U) || (lowSurrogate > 0xDFFFU))
                {
                    XHAL_THROW_RUNTIME_ERROR("Ollama response has invalid Unicode pair");
                }
                const uint32 highValue = codePoint - 0xD800U;
                const uint32 lowValue = lowSurrogate - 0xDC00U;
                codePoint = 0x10000U + (highValue << 10U) + lowValue;
                byteOffset += 6U;
            }
            else if ((codePoint >= 0xDC00U) && (codePoint <= 0xDFFFU))
            {
                XHAL_THROW_RUNTIME_ERROR("Ollama response has isolated Unicode surrogate");
            }
            appendUtf8(output, codePoint);
        }
        else
        {
            XHAL_THROW_RUNTIME_ERROR("Ollama response has unsupported JSON escaping");
        }
    }
    XHAL_THROW_RUNTIME_ERROR("Ollama response has an unterminated JSON string");
}

/**
 * @brief Decodes one hexadecimal JSON Unicode digit.
 *
 * @param[in] value ASCII hexadecimal digit.
 * @return Numeric digit from zero through fifteen.
 * @throws std::runtime_error If `value` is not hexadecimal.
 */
uint8 XWalkLanguageModelOllama::hexadecimalDigit(char value)
{
    if ((value >= '0') && (value <= '9'))
    {
        return static_cast<uint8>(value - '0');
    }
    if ((value >= 'A') && (value <= 'F'))
    {
        return static_cast<uint8>((value - 'A') + 10);
    }
    if ((value >= 'a') && (value <= 'f'))
    {
        return static_cast<uint8>((value - 'a') + 10);
    }
    XHAL_THROW_RUNTIME_ERROR("Ollama response has invalid Unicode hexadecimal data");
}

/**
 * @brief Appends one validated Unicode code point as UTF-8.
 *
 * @param[in,out] output Decoded response receiving UTF-8 bytes.
 * @param[in] codePoint Unicode scalar value from zero through `0x10FFFF`.
 * @throws std::runtime_error If `codePoint` is not a Unicode scalar value.
 */
void XWalkLanguageModelOllama::appendUtf8(string& output, uint32 codePoint)
{
    if (codePoint <= 0x7FU)
    {
        output.push_back(static_cast<char>(codePoint));
    }
    else if (codePoint <= 0x7FFU)
    {
        output.push_back(static_cast<char>(0xC0U | (codePoint >> 6U)));
        output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
    }
    else if (codePoint <= 0xFFFFU)
    {
        output.push_back(static_cast<char>(0xE0U | (codePoint >> 12U)));
        output.push_back(static_cast<char>(0x80U | ((codePoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
    }
    else if (codePoint <= 0x10FFFFU)
    {
        output.push_back(static_cast<char>(0xF0U | (codePoint >> 18U)));
        output.push_back(static_cast<char>(0x80U | ((codePoint >> 12U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | ((codePoint >> 6U) & 0x3FU)));
        output.push_back(static_cast<char>(0x80U | (codePoint & 0x3FU)));
    }
    else
    {
        XHAL_THROW_RUNTIME_ERROR("Ollama response has an invalid Unicode code point");
    }
}

} /* namespace xwalk::hal */
