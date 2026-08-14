# xWalkTool Overview

`xWalkTool` contains repository-maintenance, host-verification, Raspberry Pi provisioning, interface-generation,
and Device Tree assets. It is tooling around the xWalk firmware workspace, not a HAL or Agent runtime module.

Normal host builds do not run these tools automatically. Select a tool explicitly for the intended task and
review whether it is read-only, writes generated output, changes configuration, or modifies a Raspberry Pi.

## Directory contents

- `shell-agent/clean-build.sh` finds and removes generated CMake and Python output. See the
  [clean build guide](Clean%20Build%20Script%20Guide.md).
- `shell-agent/run-host-coverage.sh` runs host coverage in the foreground. See the
  [host coverage guide](Host%20Coverage%20Script%20Guide.md).
- `board-tool/` contains the installable, host-only historical Git-to-Jira importer under `py-src/xWalkJiraImport`.
  Its [package README](../../../../xWalkTool/py-agent/board-tool/README.md) documents installation and dry-run safety.
- `dev-tool/xHal_Rpi5CarDependencyInstaller` checks and installs mapped packages. Its guarded v5 mode can also
  back up and update the Raspberry Pi boot configuration and install the verified v5 overlay. See the
  [complete flag reference](Dependency%20Installer%20Script%20Flags.md).
- `shell-agent/setup-rpi.sh` plans, validates, or applies Raspberry Pi setup. See the
  [Raspberry Pi setup guide](Raspberry%20Pi%20Setup%20Script%20Guide.md).
- `shell-agent/provision-hardware.sh` persists exact target hardware identity. See the
  [hardware provisioning guide](Hardware%20Provisioning%20Script%20Guide.md).
- `dev-tool/xHal_Rpi5CarIwGenerator` validates or generates the active xWalkIW
  schema outputs. The [module README](../../../xWalkIW/README.md)
  documents its protocol and build contract.
- `dev-tool/xWalkLicenseTool` creates and opens authenticated model settings;
  `shell-agent/env-tool/license/xWalkEnv.sh` adds API credentials from `~/.netrc`. See the
  [licence tool guide](xWalk%20Licence%20Tool%20Guide.md).
- `shell-agent/env-tool/license/xWalkEnv.sh` sources authenticated values into the current shell. See the
  [environment loader guide](xWalk%20Environment%20Loader%20Guide.md).
- `shell-agent/env-tool/quality/` contains the checked-in Clang-Tidy, Cppcheck-suppression, and
  gcovr configurations used by the host quality workflows.
- `shell-agent/deploy-tool/` contains Debian metadata, systemd and tmpfiles definitions, the
  udev rule template, and host-only deployment tests.
- `shell-agent/env-tool/dtoverlays/` holds two compiled SunFounder Device Tree overlay assets. See the
  [overlay assets guide](Device%20Tree%20Overlay%20Assets%20Guide.md).

The authoritative source-level inventory remains [`xWalkTool/README.md`](../../../../xWalkTool/README.md).
Authenticated environment encryption and loading are summarized in the
[licence-key workflow](License%20Key%20Workflow.md), with executable-specific
contracts in the two guides above.

## Safety classes

| Class | Tools | Expected effects |
| --- | --- | --- |
| Read-only | Script help and dry-run tools, including `xWalkJiraImport` | Reports information only |
| Host generated output | `run-host-coverage.sh run` | Updates build output |
| Destructive host maintenance | `clean-build.sh --yes` | Deletes discovered CMake and Python output |
| Target configuration | `provision-hardware.sh` | Replaces one selected writable configuration atomically |
| Privileged target setup | `setup-rpi.sh --apply` | Changes target provisioning state |
| Package and v5 boot setup | `xHal_Rpi5CarDependencyInstaller` | Host skips boot; verified v5 mode is privileged |
| Manual target administration | `shell-agent/env-tool/dtoverlays/` | Required for unsupported assets |

None of these tools should start an actuator. This does not make privileged setup or manual overlay changes
risk-free. Hardware tests remain opt-in and require an explicitly approved, secured Raspberry Pi and Robot HAT.

## Safe host checks

```sh
find xWalkTool/shell-agent -type f -name '*.sh' -print0 | xargs -0 bash -n
xWalkTool/shell-agent/repo-tool/clean-build.sh --dry-run
xWalkTool/shell-agent/quality-tool/run-host-coverage.sh --help
xWalkTool/shell-agent/deploy-tool/setup-rpi.sh --help
xWalkTool/shell-agent/deploy-tool/provision-hardware.sh --help
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --device host --check
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarIwGenerator --help
xWalkTool/py-agent/dev-tool/xWalkLicenseTool --help
python3 -m unittest xWalkTool/py-agent/dev-tool/test/test_xWalkLicenseTool.py
bash xWalkTool/shell-agent/deploy-tool/test/setup-rpi-test.sh
```

Do not run `setup-rpi.sh --apply`, select `xHal_Rpi5CarDependencyInstaller --device rpi`, install an overlay, or
execute hardware-labelled tests during host checks.
