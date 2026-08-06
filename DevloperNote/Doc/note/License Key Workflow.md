# Licence-Key Workflow

xWalk stores selected model names and paid-provider API credentials in one
authenticated encrypted file. The decryption key is separate from the project;
the repository never stores, derives, or embeds it.

## Directory structure

```text
MyPiCarX/
├── xWalkLibrary/
│   └── X_WALK_LICENSE.KEY
└── xWalkTool/
    ├── environment/
    │   └── xWalkLicense.json
    └── python/
        └── xWalkLicenseTool
```

`xWalkTool/environment/xWalkLicense.json` is the committed empty template.
Every key is an environment-variable name and every value is an empty string.
Never fill this tracked file in place. `xWalkLibrary/X_WALK_LICENSE.KEY` is the
only encrypted licence path; its repository version is an unprovisioned `XWL1`
marker until an operator performs encryption. The generated
`X_WALK_LICENSE_SERIAL` metadata is not an input-template field.

## Dependency

The tool uses PyNaCl `SecretBox` authenticated encryption. On Debian, Ubuntu,
or Raspberry Pi OS, install the repository-managed package:

```sh
sudo apt-get install python3-nacl
```

In an isolated Python environment, the equivalent package is:

```sh
python3 -m pip install PyNaCl
```

The xWalk dependency manifest, Raspberry Pi setup script, Debian package
metadata, and host CI all declare `python3-nacl`.

## Prepare protected input

Copy the empty template to a secure location outside the repository, restrict
it to its owner, and fill every value required by the environment loader:

```sh
install -m 0600 xWalkTool/environment/xWalkLicense.json \
    /secure/location/xWalkLicense.json
```

Encryption rejects an empty object, non-string values, invalid environment
names, and any empty value. Running it against the committed template therefore
fails safely and names the empty variables without printing their values.

## Encrypt

The preferred command reads the protected external JSON file:

```sh
python3 xWalkTool/python/xWalkLicenseTool encrypt \
    --json /secure/location/xWalkLicense.json
```

For a small manual selection, repeat `--env`:

```sh
python3 xWalkTool/python/xWalkLicenseTool encrypt \
    --env OPENAI_API_KEY=<value> \
    --env GEMINI_API_KEY=<value>
```

Values supplied through `--env` can appear in shell history and process
listings. A protected JSON file outside the repository is preferred. The two
input methods are mutually exclusive, and duplicate `--env` names are rejected.

Successful encryption writes only `xWalkLibrary/X_WALK_LICENSE.KEY`. It uses a
new random 256-bit key, a fresh random nonce, the `XWL1` version header, and a
SecretBox authenticator. It also generates one licence identifier using the
current UTC year and four cryptographically secure random bytes, stores that
same value as `X_WALK_LICENSE_SERIAL` in the authenticated payload, and prints
it only after the encrypted file has been written successfully.

Successful output has this structure:

```text
xWalk licence created successfully.

Encrypted licence file:
<project-root>/xWalkLibrary/X_WALK_LICENSE.KEY

Licence serial number:
XWALK-2026-A7F3C92D

Decryption key:
<generated-256-bit-key>
```

The random serial suffix is eight uppercase hexadecimal characters. The serial
identifies the licence; it is public metadata, is not a decryption key, and is
never used to derive the encryption key. One encryption generates only one
serial, so the terminal and authenticated payload always match. The decryption
key is printed exactly once. Store it in a separate password manager or
secret-management service and do not put it anywhere in this repository. A
validation or write failure prints neither the serial nor the key.

## Decrypt and load

Decrypt to an explicitly selected temporary path outside the source tree:

```sh
python3 xWalkTool/python/xWalkLicenseTool decrypt \
    --output /tmp/xWalkLicense.decrypted.json
```

The tool requests the key with `getpass`; it has no command-line key option.
The output is atomically written with owner-only mode `0600`. Delete it as soon
as the consumer has loaded it. An incorrect key, changed ciphertext, or changed
authentication tag fails without creating usable plaintext.

For an interactive xWalk process, source the reviewed loader. It decrypts to a
mode-`0600` temporary file, validates the complete supported name set, exports
the variables without evaluating their values as shell code, and removes the
temporary file:

```sh
source xWalkTool/shell/xWalkEnv.sh
```

The loader requires every name in the committed template. The encryption tool
can protect a smaller valid object for other consumers, but the complete xWalk
runtime environment will reject an incomplete object. The loader authenticates
and validates `X_WALK_LICENSE_SERIAL` but does not export it as an environment
variable.

## Commit and deployment policy

The following files may be committed:

- the empty `xWalkTool/environment/xWalkLicense.json` template;
- `xWalkLibrary/X_WALK_LICENSE.KEY` after it contains authenticated ciphertext;
- the licence tool, loader, tests, and documentation.

Never commit a filled JSON copy, decrypted JSON or environment file, private
key, decryption key, shell history containing values, or plaintext secret.
Generated licence copies and common private-file suffixes are ignored by Git,
while the empty template and encrypted output path remain trackable.

Installation always includes the tool, loader, and empty template under
`lib/xwalk` with the same `xWalkTool` subdirectories. The encrypted licence is
deployment-specific and is omitted by default. A package intended for one
controlled deployment can explicitly configure
`-DXWALK_INSTALL_ENCRYPTED_LICENSE=ON`; CMake then requires a provisioned `XWL1`
file and installs it under `lib/xwalk/xWalkLibrary` with mode `0600`.

## Security boundary

Base64 changes representation but provides no confidentiality. XOR schemes,
fixed nonces, hardcoded keys, and disguising data as `.obj` files are also not
secure encryption. The build never compiles a licence file into a C or C++
object and never generates source code containing a decryption key.

Authenticated encryption detects accidental or malicious modification, but it
cannot protect a device after both the encrypted file and its decryption key
are obtained. Anyone possessing both can recover every stored API key. When
xWalk is distributed to untrusted devices, keep high-value paid API keys on a
backend service and expose only a narrowly scoped device-facing interface.

The supplied systemd service does not embed or non-interactively retrieve a
decryption key. Automated service startup requires a separate operating-system
credential or secret-agent design; do not solve that by placing the key in a
unit, environment file, script, configuration file, or command line.

See the [xWalk licence tool guide](xWalk%20Licence%20Tool%20Guide.md) for the
complete command and exit contract. See the
[xWalk environment loader guide](xWalk%20Environment%20Loader%20Guide.md) for
sourced-shell behavior, validation order, cleanup, and environment lifetime.
