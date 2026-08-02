/******************************************************************************
 * @file        xHal_Rpi5CarUtilsTest.cpp
 * @brief       Verifies backend-neutral Robot HAT utility behavior.
 *
 * @details
 * Exercises output routing, volume clamping, command and lookup forwarding,
 * mapping validation, lazy caching, stderr restoration, and callback checks.
 *
 * @project     xWalk Firmware
 * @module      xWalkUtils Host Test
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

/******************************************************************************
 * Includes
 ******************************************************************************/

#include "xHal_Rpi5CarLazyReader.h"

#include "xHal_Rpi5CarTestFunctions.h"
#include "xHal_Rpi5CarStderrGuard.h"
#include "xHal_Rpi5CarUtils.h"

#include <cassert>

/******************************************************************************
 * Anonymous namespace
 ******************************************************************************/

/** @brief Contains host-test state and callbacks private to this translation unit. */
namespace
{

using namespace xwalk::hal;

/******************************************************************************
 * Structure declarations
 ******************************************************************************/

/** @brief Records every simulated platform-service interaction. */
struct TestUtilsBackend
{
    XWalkUtilityColor color{XWalkUtilityColor::White}; /**< Most recent output color. */
    string message{}; /**< Most recent output message. */
    string ending{}; /**< Most recent output terminator. */
    string command{}; /**< Most recent command text. */
    string user{}; /**< Most recent optional command user. */
    string group{}; /**< Most recent optional command group. */
    string executable{}; /**< Most recent executable lookup. */
    string interfaceName{}; /**< Most recent network-interface lookup. */
    uint8 volumePercent{}; /**< Most recent clamped volume percentage. */
    boolean flush{}; /**< Most recent output flush request. */
    uint32 outputCount{}; /**< Number of output callback entries. */
};

/** @brief Supplies deterministic lazy-reader clock and value state. */
struct TestLazyBackend
{
    uint64 currentTimeUs{}; /**< Current simulated monotonic time in microseconds. */
    uint32 nextValue{10U}; /**< Value returned by the next acquisition. */
    uint32 readCount{}; /**< Number of acquisition callback entries. */
};

/** @brief Records standard-error redirect and restore operations. */
struct TestRedirectBackend
{
    uint32 redirectCount{}; /**< Number of redirect callback entries. */
    uint32 restoreCount{}; /**< Number of restore callback entries. */
    int32 token{}; /**< Most recent restore token. */
};

/******************************************************************************
 * Private function definitions
 ******************************************************************************/

/** @brief Records one colored output request. */
void writeOutput(contextpointer context, XWalkUtilityColor color,
    stringview message, stringview ending, boolean flush)
{
    TestUtilsBackend& backend = *static_cast<TestUtilsBackend*>(context);
    backend.color = color;
    backend.message = string(message);
    backend.ending = string(ending);
    backend.flush = flush;
    ++backend.outputCount;
}

/** @brief Records one clamped system-volume request. */
void setVolume(contextpointer context, uint8 volumePercent)
{
    static_cast<TestUtilsBackend*>(context)->volumePercent = volumePercent;
}

/** @brief Records and completes one simulated command request. */
XWalkCommandResult runCommand(contextpointer context, stringview command,
    stringview user, stringview group)
{
    TestUtilsBackend& backend = *static_cast<TestUtilsBackend*>(context);
    backend.command = string(command);
    backend.user = string(user);
    backend.group = string(group);
    return {7, "combined output"};
}

/** @brief Records an executable lookup and recognizes one test command. */
boolean executableExists(contextpointer context, stringview executable)
{
    TestUtilsBackend& backend = *static_cast<TestUtilsBackend*>(context);
    backend.executable = string(executable);
    return executable == "available";
}

/** @brief Records an interface lookup and supplies one Ethernet address. */
string ipAddress(contextpointer context, stringview interfaceName)
{
    TestUtilsBackend& backend = *static_cast<TestUtilsBackend*>(context);
    backend.interfaceName = string(interfaceName);
    return (interfaceName == "eth0") ? string("192.0.2.10") : string{};
}

/** @brief Supplies one deterministic effective username. */
string username(contextpointer context)
{
    static_cast<void>(context);
    return "robot";
}

/** @brief Creates the complete simulated utility callback table. */
XWalkUtilsCallbacks utilityCallbacks()
{
    return {&writeOutput, &setVolume, &runCommand, &executableExists,
        &ipAddress, &username};
}

/** @brief Returns the simulated monotonic time in microseconds. */
uint64 lazyClock(contextpointer context)
{
    return static_cast<TestLazyBackend*>(context)->currentTimeUs;
}

/** @brief Acquires and increments one simulated lazy-reader value. */
uint32 lazyRead(contextpointer context)
{
    TestLazyBackend& backend = *static_cast<TestLazyBackend*>(context);
    ++backend.readCount;
    const uint32 acquiredValue = backend.nextValue;
    ++backend.nextValue;
    return acquiredValue;
}

/** @brief Begins simulated standard-error suppression and returns token 42. */
int32 redirectError(contextpointer context)
{
    TestRedirectBackend& backend = *static_cast<TestRedirectBackend*>(context);
    ++backend.redirectCount;
    return 42;
}

/** @brief Records simulated standard-error restoration. */
void restoreError(contextpointer context, int32 restoreToken)
{
    TestRedirectBackend& backend = *static_cast<TestRedirectBackend*>(context);
    ++backend.restoreCount;
    backend.token = restoreToken;
}

/** @brief Verifies colored-output routing and volume clamping. */
void testOutputAndVolume()
{
    TestUtilsBackend backend;
    XWalkUtils utilities(&backend, utilityCallbacks());

    utilities.info("information");
    assert(backend.color == XWalkUtilityColor::White);
    utilities.debug("diagnostic", "", true);
    assert(backend.color == XWalkUtilityColor::Gray);
    assert(backend.message == "diagnostic");
    assert(backend.ending.empty());
    assert(backend.flush);
    utilities.warning("warning");
    assert(backend.color == XWalkUtilityColor::Yellow);
    utilities.error("error");
    assert(backend.color == XWalkUtilityColor::Red);
    assert(backend.outputCount == 4U);

    utilities.setVolume(-10);
    assert(backend.volumePercent == 0U);
    utilities.setVolume(45);
    assert(backend.volumePercent == 45U);
    utilities.setVolume(110);
    assert(backend.volumePercent == 100U);
}

/** @brief Verifies command, executable, network, and username forwarding. */
void testPlatformQueries()
{
    TestUtilsBackend backend;
    XWalkUtils utilities(&backend, utilityCallbacks());

    const XWalkCommandResult result = utilities.runCommand("test", "user", "group");
    assert(result.status == 7);
    assert(result.output == "combined output");
    assert(backend.command == "test");
    assert(backend.user == "user");
    assert(backend.group == "group");
    assert(utilities.commandExists("available"));
    assert(!utilities.isInstalled("missing"));
    assert(utilities.checkExecutable("available"));
    assert(backend.executable == "available");

    assert(utilities.ipAddress() == "192.0.2.10");
    assert(backend.interfaceName == "eth0");
    assert(utilities.ipAddress("wlan0").empty());
    const stringvector interfaces{"usb0", "eth0", "wlan0"};
    assert(utilities.ipAddress(interfaces) == "192.0.2.10");
    assert(utilities.username() == "robot");
}

/** @brief Verifies numeric mapping and invalid-range rejection. */
void testMapping()
{
    assert(XWalkUtils::mapping(5.0, 0.0, 10.0, 0.0, 100.0) == 50.0);
    assert(XWalkUtils::mapping(15.0, 0.0, 10.0, 0.0, 100.0) == 150.0);

    xwalk::hal::test::expectFailure([&]()
    {
        static_cast<void>(XWalkUtils::mapping(1.0, 2.0, 2.0, 3.0, 4.0));
    });
}

/** @brief Verifies strict-interval lazy caching and refresh behavior. */
void testLazyReader()
{
    TestLazyBackend backend;
    XWalkLazyReader<uint32> reader(&backend, &lazyRead, &lazyClock, 10U);

    assert(reader.read() == 10U);
    assert(backend.readCount == 1U);
    backend.currentTimeUs = 10'000U;
    assert(reader.read() == 10U);
    assert(backend.readCount == 1U);
    backend.currentTimeUs = 10'001U;
    assert(reader.read() == 11U);
    assert(backend.readCount == 2U);
}

/** @brief Verifies scope-bound redirect restoration and retained token delivery. */
void testStderrGuard()
{
    TestRedirectBackend backend;
    {
        XWalkStderrGuard guard(&backend, &redirectError, &restoreError);
        assert(backend.redirectCount == 1U);
        assert(backend.restoreCount == 0U);
        static_cast<void>(guard);
    }
    assert(backend.restoreCount == 1U);
    assert(backend.token == 42);
}

/** @brief Verifies rejection of incomplete callback bindings and invalid colors. */
void testValidation()
{
    TestUtilsBackend backend;
    const fixedarray<XWalkUtilsCallbacks, 6U> incompleteCallbacks{
        XWalkUtilsCallbacks{nullptr, &setVolume, &runCommand, &executableExists,
            &ipAddress, &username},
        XWalkUtilsCallbacks{&writeOutput, nullptr, &runCommand, &executableExists,
            &ipAddress, &username},
        XWalkUtilsCallbacks{&writeOutput, &setVolume, nullptr, &executableExists,
            &ipAddress, &username},
        XWalkUtilsCallbacks{&writeOutput, &setVolume, &runCommand, nullptr,
            &ipAddress, &username},
        XWalkUtilsCallbacks{&writeOutput, &setVolume, &runCommand, &executableExists,
            nullptr, &username},
        XWalkUtilsCallbacks{&writeOutput, &setVolume, &runCommand, &executableExists,
            &ipAddress, nullptr}};
    for (const XWalkUtilsCallbacks& callbacks : incompleteCallbacks)
    {
        xwalk::hal::test::expectFailure([&]()
        {
            XWalkUtils utilities(&backend, callbacks);
            static_cast<void>(utilities);
        });
    }

    XWalkUtils utilities(&backend, utilityCallbacks());
    xwalk::hal::test::expectFailure([&]()
    {
        utilities.printColor("invalid", static_cast<XWalkUtilityColor>(255U));
    });

    TestRedirectBackend redirectBackend;
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkStderrGuard guard(&redirectBackend, nullptr, &restoreError);
        static_cast<void>(guard);
    });

    TestLazyBackend lazyBackend;
    xwalk::hal::test::expectFailure([&]()
    {
        XWalkLazyReader<uint32> reader(&lazyBackend, nullptr, &lazyClock);
        static_cast<void>(reader);
    });
}

} /* namespace */

/******************************************************************************
 * Global function definitions
 ******************************************************************************/

/**
 * @brief Runs every host-side utility test.
 *
 * @return
 * Zero when every assertion passes; a failed assertion terminates the process.
 */
int main()
{
    testOutputAndVolume();
    testPlatformQueries();
    testMapping();
    testLazyReader();
    testStderrGuard();
    testValidation();
    return 0;
}
