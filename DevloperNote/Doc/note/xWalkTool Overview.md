# xWalkTool Overview

`xWalkTool` contains repository-maintenance, host-verification, Raspberry Pi provisioning, interface-generation,
and Device Tree assets. It is tooling around the xWalk firmware workspace, not a HAL or Agent runtime module.

Normal host builds do not run these tools automatically. Select a tool explicitly for the intended task and
review whether it is read-only, writes generated output, changes configuration, or modifies a Raspberry Pi.

## Directory contents

- `clean-build.sh` finds and removes generated CMake output. See the
  [clean build guide](Clean%20Build%20Script%20Guide.md).
- `eclipse-build.sh` builds or cleans the Eclipse-oriented CLI host tree. See the
  [Eclipse build guide](Eclipse%20Build%20Script%20Guide.md).
- `run-host-coverage.sh` runs host coverage in the foreground. See the
  [host coverage guide](Host%20Coverage%20Script%20Guide.md).
- `setup-rpi.sh` plans, validates, or applies Raspberry Pi setup. See the
  [Raspberry Pi setup guide](Raspberry%20Pi%20Setup%20Script%20Guide.md).
- `provision-hardware.sh` persists exact target hardware identity. See the
  [hardware provisioning guide](Hardware%20Provisioning%20Script%20Guide.md).
- `xHal_Rpi5CarIwGenerator` validates or generates the inactive xWalkIW schema outputs. It is not separately
  documented.
- `dtoverlays/` holds two compiled SunFounder Device Tree overlay assets. See the
  [overlay assets guide](Device%20Tree%20Overlay%20Assets%20Guide.md).

The authoritative source-level inventory remains [`xWalkTool/README.md`](../../../xWalkTool/README.md).

## Safety classes

| Class | Tools | Expected effects |
| --- | --- | --- |
| Read-only | Script help, `clean-build.sh --dry-run`, `setup-rpi.sh --dry-run` | Reports information only |
| Host generated output | `eclipse-build.sh`, `run-host-coverage.sh run` | Creates or updates build output |
| Destructive host maintenance | `clean-build.sh --yes` | Deletes discovered generated build output |
| Target configuration | `provision-hardware.sh` | Replaces one selected writable configuration atomically |
| Privileged target setup | `setup-rpi.sh --apply` | Changes target provisioning state |
| Manual target administration | `dtoverlays/` | Requires separate physical verification and boot configuration |

None of these tools should start an actuator. This does not make privileged setup or manual overlay changes
risk-free. Hardware tests remain opt-in and require an explicitly approved, secured Raspberry Pi and Robot HAT.

## Safe host checks

```sh
bash -n xWalkTool/*.sh
xWalkTool/clean-build.sh --dry-run
xWalkTool/run-host-coverage.sh --help
xWalkTool/setup-rpi.sh --help
xWalkTool/provision-hardware.sh --help
xWalkTool/xHal_Rpi5CarIwGenerator --help
bash deployment/test/setup-rpi-test.sh
```

Do not run `setup-rpi.sh --apply`, install an overlay, or execute hardware-labelled tests during host checks.
