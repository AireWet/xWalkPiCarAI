# xWalk licence installation

The Controller loads AI model selections from an authenticated encrypted licence. Provider credentials remain
separate in the runtime user's mode-`0600` `$HOME/.netrc` file.

Run every non-privileged command below as the same user that starts `xwalk-picarx-control`. From the integration
checkout:

```bash
cd /repo/joxjoh24/xWalkPiCarAI
```

Never commit the encrypted licence, a filled model configuration, `.netrc`, an API key, or the licence decryption
key.

## Install the encryption dependency

On Raspberry Pi OS, Debian, or Ubuntu:

```bash
sudo apt-get install python3-nacl
```

## Prepare the model configuration

Copy the repository's empty template to an owner-only file outside the repository:

```bash
install -m 0600 xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg "$HOME/xWalkLicense.cfg"
nano "$HOME/xWalkLicense.cfg"
```

For the optional Gemini Jarvis provider, populate every model field:

```ini
[models]
ANTHROPIC_MODEL = unused
GEMINI_MODEL = gemini-3.6-flash
OLLAMA_MODEL = llama3.2:3b
OPENAI_MODEL = unused
XWALK_AI_MODEL = gemini-3.6-flash
```

The configuration must contain exactly the `[models]` section and all names from the committed template. Do not
place API keys in this file.

## Store the Gemini credential

Create or update `$HOME/.netrc` for the runtime user:

```text
machine generativelanguage.googleapis.com
    login apikey
    password REPLACE_WITH_THE_GEMINI_API_KEY
```

Protect the file before loading the environment:

```bash
chmod 600 "$HOME/.netrc"
```

See [Gemini Jarvis configuration](GEMINI_CONFIGURATION.md) for the voice model and Controller configuration.

## Generate the encrypted licence

Encrypt the protected model configuration:

```bash
xWalkTool/py-agent/dev-tool/xWalkLicenseTool encrypt --config "$HOME/xWalkLicense.cfg"
```

The command creates `xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY` and prints its decryption key exactly once. Save
the decryption key in a password manager or approved secret-management service. Do not paste it into chat or
store it in the repository.

After safely storing the decryption key, remove the plaintext configuration:

```bash
rm "$HOME/xWalkLicense.cfg"
```

Confirm that the encrypted file exists, belongs to the runtime user, and has owner-only permissions:

```bash
stat -c 'mode=%a owner=%U:%G path=%n' xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY
```

The expected mode is `600`. If necessary, correct only this file's permissions:

```bash
chmod 600 xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY
```

## Load and verify the environment

Source the environment loader and enter the saved decryption key when prompted:

```bash
source xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
```

Confirm that the Gemini credential and model were loaded without printing either secret:

```bash
test -n "${GEMINI_API_KEY:-}" && echo "GEMINI_API_KEY loaded"
test "${GEMINI_MODEL:-}" = "gemini-3.6-flash" && echo "Gemini model loaded"
```

The variables exist only in the current shell. Source `xWalkEnv.sh` again in each new shell before starting an
AI-backed Controller command.

## Build and validate the Controller

Configure and build the Raspberry Pi release:

```bash
cmake --fresh -S xWalk-rpi5 --preset rpi-release
cmake --build build-rpi/cmake --parallel
```

Validate the deployed configuration, then run the bounded hardware preflight:

```bash
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control --deployment-config="$PWD/build-rpi/runtime/picar-x.conf" --validate-config
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control doctor
```

Confirm that the correct Raspberry Pi and Robot HAT are connected before running Doctor.

## Start Jarvis safely

Before starting, place the car securely with its wheels raised. Confirm that the microphone, camera, speaker, and
intended Robot HAT are connected.

```bash
source xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
build-rpi/cmake/xWalkController/xWalkApp/xwalk-picarx-control voice-active-car-gpt start
```

Say `Hey Jarvis`, wait for the acknowledgement, and then speak the request. Press `Ctrl+C` to stop.

## Troubleshooting

### Encrypted licence file is unreadable

This message means the loader could not find or read the deployment-specific licence:

```text
xWalk environment: encrypted licence file is unreadable: .../xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY
```

Generate the file with the encryption command above. If it already exists, inspect its ownership and mode with
`stat`; the runtime user must own or be able to read it, and its mode must be `0600`.

### Encrypted licence file must have mode 0600

Restrict the existing file:

```bash
chmod 600 xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY
```

### Licence-key decryption failed

Retry with the exact key printed when that licence file was generated. If the key is unavailable, create a new
licence and securely retain its newly generated key.

### Credential file is unavailable

Create `$HOME/.netrc` for the runtime user, add the Gemini machine entry, and apply mode `0600`. The loader checks
the encrypted licence before it reads `.netrc`, so resolve licence errors first.
