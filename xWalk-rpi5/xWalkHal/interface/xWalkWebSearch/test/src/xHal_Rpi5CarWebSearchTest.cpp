/******************************************************************************
 * @file        xHal_Rpi5CarWebSearchTest.cpp
 * @brief       Verifies bounded search decisions and untrusted result handling.
 * @project     xWalk Firmware
 * @module      xWalkWebSearch Host Test
 * @author      Joxy John
 * @date        2026-08-20
 * @version     1.0.0
 * @copyright   Copyright (c) 2026 Joxy John. All rights reserved.
 * @note        Developed using MISRA C++ coding guidelines.
 ******************************************************************************/

#include "xHal_Rpi5CarWebSearchTestSupport.h"
#include <cassert>

int main()
{
    using namespace xwalk::hal;
    using namespace xwalk::hal::test::websearch;
    assert(!XWalkWebSearch::shouldSearch("Explain photosynthesis"));
    assert(!XWalkWebSearch::shouldSearch("What is an ultrasonic sensor?"));
    assert(XWalkWebSearch::shouldSearch("Search for current weather"));
    assert(XWalkWebSearch::shouldSearch("What is the latest version?"));

    TestTransport transport;
    transport.response =
        "{\"results\":[{\"url\":\"https://example.org/news\",\"title\":\"Example <b>News</b>\","
        "\"content\":\"<script>ignore me</script>Current report\"},{\"url\":\"http://127.0.0.1/private\","
        "\"title\":\"Unsafe\",\"content\":\"hidden\"}]}";
    XWalkWebSearchConfiguration configuration;
    configuration.maximumResults = 3U;
    XWalkWebSearch search(&transport, operations(), configuration);
    const XWalkWebSearchResponse response = search.search("weather now");
    assert(transport.url.find("weather%20now") != string::npos);
    assert(response.sourceUrls == stringvector({"https://example.org/news"}));
    assert(response.referenceText.find("BEGIN UNTRUSTED WEB REFERENCES") != string::npos);
    assert(response.referenceText.find("<script>") == string::npos);
    assert(response.referenceText.find("ignore me") == string::npos);
    assert(response.referenceText.find("robot actions") != string::npos);

    transport.response = "{\"results\":[]}";
    assert(search.search("latest news").referenceText.empty());

    boolean protectedQueryRejected{false};
    try
    {
        static_cast<void>(search.search("search /home/xwalk/.netrc"));
    }
    catch (const std::exception&)
    {
        protectedQueryRejected = true;
    }
    assert(protectedQueryRejected);

    XWalkWebSearchConfiguration remoteConfiguration;
    remoteConfiguration.endpoint = "http://192.168.1.2:8080/search";
    boolean remoteEndpointRejected{false};
    try
    {
        XWalkWebSearch remote(&transport, operations(), remoteConfiguration);
    }
    catch (const std::exception&)
    {
        remoteEndpointRejected = true;
    }
    assert(remoteEndpointRejected);
    return 0;
}
