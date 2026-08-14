# xWalk environment loader guide

[`xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh`](../../../../xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh) loads the
authenticated licence environment into the current interactive Bash process.
It must be sourced because an executed child process cannot modify its parent
shell environment.

## Prerequisites

The loader requires:

- Bash with indexed and associative array support;
- Python 3 and PyNaCl;
- the executable `xWalkTool/py-agent/dev-tool/xWalkLicenseTool`;
- the empty model-only `xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg` template;
- a per-user mode-`0600` `~/.netrc` containing any fixed-provider credentials
  selected by the deployment; and
- a provisioned `xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY` file with mode `0600`.

The tool and files may be in the repository layout or the matching installed
layout below `/usr/lib/xwalk`.

## Load the environment

From the repository root, source the loader:

```sh
source xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
```

It requests the decryption key through the licence tool's private prompt. On
success, model variables from the authenticated licence and paid-provider API
credentials from `~/.netrc` are exported into the current shell. The
authenticated `X_WALK_LICENSE_SERIAL` value is validated but is not exported.

Executing the file is an error:

```sh
xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
```

That form returns `2` and explains that the script must be sourced.

## Validation and cleanup

The loader performs these operations before exporting any value:

1. Resolves the repository or installed root relative to its own location.
2. Requires the encrypted licence file to be readable and have mode `0600`.
3. Creates a temporary JSON file and restricts it to mode `0600`.
4. Authenticates and decrypts the licence through `xWalkLicenseTool`.
5. Validates the serial and requires the decrypted model-name set to match the
   committed empty template exactly.
6. Loads `Path.home() / ".netrc"`, requires mode `0600` on POSIX, and reads
   supported fixed-provider hosts. Missing hosts and empty passwords are skipped;
   the login field is ignored.
7. Transfers model and credential names and values as null-delimited records
   without evaluating plaintext as shell commands.
8. Removes the temporary decrypted file before exporting validated values.

Failure returns `2`, removes the temporary file, and does not print API values.
Existing values already present in the calling shell are not cleared by a
failed load.

## Shell security boundary

After success, plaintext credentials exist in the calling process environment
and are inherited by child processes unless the caller removes them. Do not
print the variables, save the environment, enable shell tracing, or launch
untrusted child processes from that shell. Unset sensitive variables when the
session no longer needs them.

The loader does not make unattended service startup safe. A system service
needs a separate operating-system credential or secret-agent design; never
place the decryption key in a unit, script, configuration file, or command line.

## Verification

Run Bash syntax validation and the isolated fake-secret integration test:

```sh
bash -n xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
bash xWalkTool/shell-agent/deploy-tool/test/environment-loader-test.sh
```

The integration test sets `XWALK_NETRC_FILE` to a temporary fake netrc file and
does not read or expose a real API key.

See the [licence tool guide](xWalk%20Licence%20Tool%20Guide.md) for licence
creation and the [licence-key workflow](License%20Key%20Workflow.md) for the
complete deployment and commit policy.
