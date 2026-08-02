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

XWalkHal::string lowercase(XWalkHal::stringview text)
{
    XWalkHal::string result(text);
    std::transform(result.begin(), result.end(), result.begin(),
        [](char value) -> char
        {
            return static_cast<char>(std::tolower(static_cast<unsigned char>(value)));
        });
    return result;
}

void removePairedTag(XWalkHal::string& text, XWalkHal::stringview tagName)
{
    const XWalkHal::string opening = XWalkHal::string("<") + XWalkHal::string(tagName);
    const XWalkHal::string closing = XWalkHal::string("</") + XWalkHal::string(tagName) + ">";
    while (true)
    {
        const XWalkHal::string lowerText = lowercase(text);
        const XWalkHal::size start = lowerText.find(opening);
        if (start == XWalkHal::string::npos)
        {
            return;
        }
        const XWalkHal::size openingEnd = lowerText.find('>', start + opening.size());
        if (openingEnd == XWalkHal::string::npos)
        {
            text.erase(start);
            return;
        }
        const XWalkHal::size end = lowerText.find(closing, openingEnd + 1U);
        if (end == XWalkHal::string::npos)
        {
            text.erase(start);
            return;
        }
        text.erase(start, (end + closing.size()) - start);
    }
}

void removeDelimitedSections(XWalkHal::string& text, XWalkHal::stringview delimiter)
{
    while (true)
    {
        const XWalkHal::size start = text.find(delimiter);
        if (start == XWalkHal::string::npos)
        {
            return;
        }
        const XWalkHal::size end = text.find(delimiter, start + delimiter.size());
        if (end == XWalkHal::string::npos)
        {
            text.erase(start);
            return;
        }
        text.erase(start, (end + delimiter.size()) - start);
    }
}

void removeMarker(XWalkHal::string& text, XWalkHal::stringview marker)
{
    while (true)
    {
        const XWalkHal::string lowerText = lowercase(text);
        const XWalkHal::size position = lowerText.find(marker);
        if (position == XWalkHal::string::npos)
        {
            return;
        }
        text.erase(position, marker.size());
    }
}

XWalkHal::string trimResponse(XWalkHal::stringview text)
{
    XWalkHal::string compact;
    compact.reserve(text.size());
    for (char value : text)
    {
        if (value == '\n')
        {
            while (!compact.empty() &&
                ((compact.back() == ' ') || (compact.back() == '\t')))
            {
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
    return (begin < end) ? XWalkHal::string(begin, end) : XWalkHal::string{};
}

} /* namespace */

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::agent
{

hal::string XWalkLocalVoiceChatbot::stripThinking(hal::stringview response)
{
    hal::string result(response);
    removePairedTag(result, "think");
    removePairedTag(result, "thinking");
    removeDelimitedSections(result, "```");
    removeMarker(result, "[thinking]");
    removeMarker(result, "[/thinking]");
    return trimResponse(result);
}

} /* namespace xwalk::agent */
