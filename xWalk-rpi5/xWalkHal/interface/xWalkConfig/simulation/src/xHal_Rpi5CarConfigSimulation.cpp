/******************************************************************************
 * @file        xHal_Rpi5CarConfigSimulation.cpp
 * @brief       Implements the device-safe xWalkConfig simulation.
 *
 * @details
 * Writes only below a build-selected data directory and verifies both public
 * configuration APIs through persistence and reconstruction.
 *
 * @project     xWalk Firmware
 * @module      xWalkConfig Host Simulation
 *
 * @author      Joxy John
 * @date        2026-08-10
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

#include "xHal_Rpi5CarConfigSimulation.h"

#include "xHal_Rpi5CarConfig.h"
#include "xHal_Rpi5CarConfigStore.h"
#include "xHal_Rpi5CarTrace.h"

/******************************************************************************
 * Namespace definitions
 ******************************************************************************/

namespace xwalk::hal::sim {

/**
 * @brief Runs representative section and flat-store persistence operations.
 * @param[in] dataDirectory Writable simulation-owned directory.
 * @return Zero when values persist and reload successfully; otherwise one.
 */
int32 runConfigSimulation(const filesystempath &dataDirectory) {
  static_cast<void>(createDirectories(dataDirectory));
  const filesystempath sectionPath = dataDirectory / "simulation.ini";
  const filesystempath storePath = dataDirectory / "simulation.config";

  {
    XWalkConfig configuration(sectionPath.string(),
                              "xWalkConfig host simulation");
    configuration.setSection("motor", {{"speed", "42"}, {"mode", "safe"}});
    configuration.write();
  }
  const XWalkConfig reloadedConfiguration(sectionPath.string());
  const boolean sectionPersisted =
      reloadedConfiguration.section("motor").at("speed") == "42";

  {
    XWalkConfigStore store(storePath.string());
    store.set("calibration", "verified");
  }
  const XWalkConfigStore reloadedStore(storePath.string());
  const boolean storePersisted = reloadedStore.get("calibration") == "verified";
  const boolean simulationSucceeded = sectionPersisted && storePersisted;
  const string ownedDataDirectory = dataDirectory.string();
  XWALK_HAL_TRACE_UID1(
      RPI .109,
      "xWalkConfig simulation persisted section and store values below %s",
      ownedDataDirectory.c_str());
  return simulationSucceeded ? 0 : 1;
}

} /* namespace xwalk::hal::sim */
