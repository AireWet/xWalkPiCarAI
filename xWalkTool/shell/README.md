# xWalk Shell Tools

The scripts in this directory provide explicit repository maintenance, host
verification, Raspberry Pi provisioning, and licence-environment loading.
Run commands from the `MyPiCarX` repository root unless a section says to
source the script.

Hardware operations are opt-in. Do not apply Raspberry Pi or Robot HAT
changes until the exact board, HAT revision, wiring, and safe physical state
have been confirmed.

## Prerequisites

Use Bash and install the dependencies required by the selected operation.
CMake and CTest are needed for host builds. The complete sanitizer, coverage,
Valgrind, static-analysis, and ShellCheck prerequisites and commands are in the
[host quality guide](../quality/README.md).

The licence environment loader additionally requires Python 3 and PyNaCl.
The recommended Python environment is documented in the
[Python tools guide](../python/README.md).

## Script inventory

| Script | Purpose | Safety boundary |
|---|---|---|
| `clean-build.sh` | Finds and removes generated CMake and Python output | Use `--dry-run` first |
| `run-host-coverage.sh` | Builds, tests, and creates the host coverage report | Host-only |
| `check-host-quality-dependencies.sh` | Reports analysis tool paths and versions | Read-only |
| `run-host-sanitizer.sh` | Runs isolated ASan/UBSan, LSan, or TSan verification | Host-only |
| `run-host-valgrind.sh` | Runs focused CTest MemCheck cases | Host-only |
| `validate-valgrind-descriptors.sh` | Rejects child-owned descriptors reported by Valgrind | Read-only |
| `run-clang-static-analyzer.sh` | Runs a clean Clang Static Analyzer build | Host-only |
| `run-host-shellcheck.sh` | Checks all repository-owned shell scripts | Read-only |
| `run-host-quality.sh` | Runs the complete host-quality workflow | Host-only |
| `validate-integration-metadata.sh` | Validates eight exact Gerrit submodule gitlinks | Read-only |
| `checkout-gerrit-submodules.sh` | Initializes private Gerrit submodules with a temporary SSH identity | CI-only |
| `provision-hardware.sh` | Writes verified hardware identities to one configuration file | Hardware-specific |
| `setup-rpi.sh` | Checks, previews, or applies Raspberry Pi provisioning | `--apply` is privileged |
| `xWalkEnv.sh` | Combines authenticated models with private netrc credentials | Must be sourced |

## Host maintenance

Preview cleanup without changing the workspace:

```sh
xWalkTool/shell/clean-build.sh --dry-run
```

Interactive cleanup asks for confirmation. Automation must opt in explicitly:

```sh
xWalkTool/shell/clean-build.sh --yes
```

Run the root host coverage preset and generate the configured report:

```sh
xWalkTool/shell/run-host-coverage.sh run
```

Each script that provides option help can be queried directly, for example:

```sh
xWalkTool/shell/clean-build.sh --help
xWalkTool/shell/run-host-coverage.sh --help
```

## Raspberry Pi provisioning

`setup-rpi.sh` requires an explicit Robot HAT profile, runtime user, and GPIO
controller. Its default mode is a dry run. Review that plan before using
`--check` or the privileged `--apply` mode:

```sh
xWalkTool/shell/setup-rpi.sh --profile robot_hat_v5 --runtime-user <user> --gpio-device /dev/gpiochip0 --dry-run
```

Use `provision-hardware.sh` only on a verified target. It validates the
selected profile and detected GPIO identity, then updates the specified
writable controller configuration:

```sh
xWalkTool/shell/provision-hardware.sh --profile robot_hat_v5 --config /var/lib/xwalk/picar-x.conf --gpio-device /dev/gpiochip0 --i2c-device /dev/i2c-1 --spi-device /dev/spidev0.0
```

Show the complete supported options before provisioning:

```sh
xWalkTool/shell/setup-rpi.sh --help
xWalkTool/shell/provision-hardware.sh --help
```

## GitHub integration metadata

GitHub Actions uses `validate-integration-metadata.sh` for metadata-only checks
and `checkout-gerrit-submodules.sh` when a read-only Gerrit identity is
available. The checkout helper validates metadata before it configures the
temporary SSH command and initializes the exact recorded submodule commits.

Neither helper pushes refs or configures GitHub remotes in component
submodules. Metadata-only validation can be exercised without credentials:

```sh
XWALK_GITHUB_METADATA_ONLY=true xWalkTool/shell/checkout-gerrit-submodules.sh
```

## Licence environment loader

`xWalkEnv.sh` must be sourced so that it can export values into the current
shell:

```sh
source xWalkTool/shell/xWalkEnv.sh
```

The loader authenticates `xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY` with
`xWalkTool/python/xWalkLicenseTool`. It privately requests the decryption key,
validates the decrypted model names against
`xWalkTool/environment/xWalkLicense.cfg`, loads API credentials from the
developer's mode-`0600` `~/.netrc`, and exports the combined values.
Missing hosts and empty passwords are skipped so unused providers do not need
keys. It does not export `X_WALK_LICENSE_SERIAL` or print API-key values.

Decryption uses a temporary mode-`0600` JSON file, which the loader removes
before returning. The encrypted licence and netrc files must also have mode
`0600`. Executing the script instead of sourcing it fails because a child
process cannot update its parent shell environment.

See the [licence-key workflow](../../xWalk-rpi5/devloper-note/Doc/note/License%20Key%20Workflow.md)
for encryption, decryption, virtual-environment setup, and key-storage rules.
The dedicated
[environment loader guide](../../xWalk-rpi5/devloper-note/Doc/note/xWalk%20Environment%20Loader%20Guide.md)
documents the validation order, failure behavior, and shell-environment lifetime.

## Verification

Check every shell script for Bash syntax errors without executing its work:

```sh
bash -n xWalkTool/shell/*.sh
```

Run the host-only licence environment integration test:

```sh
bash xWalkTool/deployment/test/environment-loader-test.sh
```

Additional script-specific detail is available from the
[xWalkTool overview](../README.md) and the linked guides in its **Detailed
guides** section.
