# xWalk Python Tools

This directory contains the repository's executable Python maintenance tools
and their host-only tests. Run commands from the MyPiCarX repository root so
the documented relative paths remain valid.

## Create the local Python environment

On Debian-family systems, install Python's virtual-environment support when it
is not already available:

```sh
sudo apt-get install python3-venv
```

Create the project-local virtual environment once:

```sh
python3 -m venv .xWalkPyEnv
```

Activate it and install the licence-tool dependency:

```sh
source .xWalkPyEnv/bin/activate
python -m pip install --upgrade pip
python -m pip install PyNaCl
```

The root `.gitignore` excludes `/.xWalkPyEnv/`. Do not place licence JSON,
decrypted data, decryption keys, or other secrets inside the environment.

Leave the environment when finished:

```sh
deactivate
```

## Licence tool

`xWalkLicenseTool` encrypts environment values into the fixed
`xWalkLibrary/X_WALK_LICENSE.KEY` path. Copy the empty template outside the
repository, restrict the copy, and fill only that external file:

```sh
install -m 0600 xWalkTool/environment/xWalkLicense.json /secure/location/xWalkLicense.json
```

Encrypt the protected JSON:

```sh
xWalkTool/python/xWalkLicenseTool encrypt --json /secure/location/xWalkLicense.json
```

Alternatively, provide repeated values manually. These values may be visible
in shell history or process listings, so protected JSON is preferred:

```sh
xWalkTool/python/xWalkLicenseTool encrypt --env OPENAI_API_KEY='your-OpenAI-API-key' --env GEMINI_API_KEY='your-Gemini-API-key'
```

Replace the quoted example text with the real values. Do not type angle-bracket
placeholders because the shell interprets `<` and `>` as redirection operators.

Decrypt to an explicit temporary path:

```sh
xWalkTool/python/xWalkLicenseTool decrypt --output /tmp/xWalkLicense.decrypted.json
```

The tool requests the decryption key interactively. It never accepts the key as
a command-line option. Delete temporary plaintext after use. See the full
[licence-key workflow](../../DevloperNote/Doc/note/License%20Key%20Workflow.md)
for commit policy, deployment, and security limitations. The dedicated
[licence tool guide](../../DevloperNote/Doc/note/xWalk%20Licence%20Tool%20Guide.md)
documents serial-number output, invocation forms, and exit behavior.

Run the host-only licence tests:

```sh
python xWalkTool/python/test/test_xWalkLicenseTool.py
```

## Other Python tools

Check project dependencies without changing the system:

```sh
xWalkTool/python/xHal_Rpi5CarDependencyInstaller --check
```

Inspect generator options before regenerating checked-in xWalkIW files:

```sh
xWalkTool/python/xHal_Rpi5CarIwGenerator --help
```
