/******************************************************************************
 * @file        xAgent_Rpi5CarLocalVoiceChatbotResponse.cpp
 * @brief       Implements hidden-thinking removal for spoken model responses.
 *
 * @project     xWalk Firmware
 * @module      xWalkLocalVoiceChatbot
 *
 * @author      Joxy John
 * @date        2026-08-02
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

#include "xAgent_Rpi5CarLocalVoiceChatbot.h"

#include <algorithm>
#include <cctype>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

namespace
{

agent::string lowercase(agent::stringview text)
{
    agent::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
        [](char value) -> char
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
        });
    return result;
}

void removePairedTag(agent::string& text, agent::stringview tagName)
{
    const agent::string opening = agent::string("<") + agent::string(tagName);
    const agent::string closing = agent::string("</") + agent::string(tagName) + ">";
    while (true)
    {
        const agent::string lowerText = lowercase(text);
        const agent::size start = lowerText.find(opening);
        if (start == agent::string::npos)
        {
            return;
        }
        const agent::size openingEnd = lowerText.find('>', start + opening.size());
        if (openingEnd == agent::string::npos)
        {
            text.erase(start);
            return;
        }
        const agent::size end = lowerText.find(closing, openingEnd + 1U);
        if (end == agent::string::npos)
        {
            text.erase(start);
            return;
        }
        text.erase(start, (end + closing.size()) - start);
    }
}

void removeDelimitedSections(agent::string& text, agent::stringview delimiter)
{
    while (true)
    {
        const agent::size start = text.find(delimiter);
        if (start == agent::string::npos)
        {
            return;
        }
        const agent::size end = text.find(delimiter, start + delimiter.size());
        if (end == agent::string::npos)
        {
            text.erase(start);
            return;
        }
        text.erase(start, (end + delimiter.size()) - start);
    }
}

void removeMarker(agent::string& text, agent::stringview marker)
{
    while (true)
    {
        const agent::string lowerText = lowercase(text);
        const agent::size position = lowerText.find(marker);
        if (position == agent::string::npos)
        {
            return;
        }
        text.erase(position, marker.size());
    }
}

agent::string trimResponse(agent::stringview text)
{
    agent::string compact;
    compact.reserve(text.size());
    for (char value : text)
    {
        if (value == '\n')
        {
            const agent::boolean processingLoopRequested{true};
            while (processingLoopRequested)
            {
                const agent::boolean whitespaceAvailable =
                    static_cast<agent::boolean>(
                        !compact.empty() &&
                ((compact.back() == ' ') || (compact.back() == '\t')));
                if (whitespaceAvailable == false)
                {
                    break;
                }
                compact.pop_back();
            }
        }
        compact.push_back(value);
    }
    const auto visible = [](char value) -> bool
    {
        return std::isspace(static_cast<unsigned char>(value)) == 0;
    };
    const auto begin = std::find_if(compact.begin(), compact.end(), visible);
    const auto end = std::find_if(compact.rbegin(), compact.rend(), visible).base();
    return (begin < end) ? agent::string(begin, end) : agent::string{};
}

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

agent::string XWalkLocalVoiceChatbot::stripThinking(agent::stringview response)
{
    agent::string result(response);
    removePairedTag(result, "think");
    removePairedTag(result, "thinking");
    removeDelimitedSections(result, "```");
    removeMarker(result, "[thinking]");
    removeMarker(result, "[/thinking]");
    return trimResponse(result);
}

} /* namespace xwalk::agent */
