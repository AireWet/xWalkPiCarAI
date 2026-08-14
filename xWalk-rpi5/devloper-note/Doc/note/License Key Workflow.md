# Licence-Key Workflow

xWalk stores selected model names in one authenticated encrypted file. Each
developer stores paid-provider API credentials separately in the standard
mode-`0600` `~/.netrc`. The repository never stores, derives, or embeds those
credentials or the licence decryption key.

## Directory structure

```text
MyPiCarX/
├── xWalk-rpi5/xWalkLibrary/
│   └── X_WALK_LICENSE.KEY
└── xWalkTool/
    ├── environment/
    │   └── xWalkLicense.cfg
    └── python/
        └── xWalkLicenseTool

~/.netrc                         per-developer API credentials, never committed
```

`xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg` is the committed empty model template.
Every key is a model environment-variable name and every value is an empty string.
Never fill this tracked file in place. `xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY` is the
only encrypted licence path and is ignored by Git because every generated file
is deployment-specific. The generated `X_WALK_LICENSE_SERIAL` metadata is not
an input-template field.

The configuration uses one case-sensitive INI-style section:

```ini
[models]
ANTHROPIC_MODEL =
GEMINI_MODEL =
OLLAMA_MODEL =
OPENAI_MODEL =
XWALK_AI_MODEL =
```

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

## Prepare model input

Copy the empty template to a secure location outside the repository, restrict
it to its owner, and fill every model value required by the environment loader:

```sh
install -m 0600 xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg /secure/location/xWalkLicense.cfg
```

Encryption requires exactly one case-sensitive `[models]` section. It rejects
an empty section, additional sections, invalid environment names, empty values,
and every `*_API_KEY` credential. Running it against the committed template
therefore fails safely and names the empty model variables without printing
their values.

## Prepare API credentials

Create the standard per-user `~/.netrc` outside the repository. The filename is
`.netrc`, not `.netric`. Each `machine` is the actual hostname contacted by the
corresponding fixed provider:

```text
machine api.anthropic.com
    login ""
    password ""
machine api.deepseek.com
    login ""
    password ""
machine ark.cn-beijing.volces.com
    login ""
    password ""
machine generativelanguage.googleapis.com
    login ""
    password ""
machine api.x.ai
    login ""
    password ""
machine api.openai.com
    login ""
    password ""
machine dashscope-intl.aliyuncs.com
    login ""
    password ""
```

Replace an empty password only for a provider the deployment plans to use.
Missing hosts and empty passwords are skipped, so unused providers do not need
accounts, billing, or API keys. The loader does not use or export the login
field, so it may remain empty unless the developer's credential policy requires
an account identifier. The one `api.x.ai` password supplies both
`GROK_API_KEY` and `XAI_API_KEY`.

`LLM_API_KEY`, `OTHERS_API_KEY`, and `XWALK_AI_API_KEY` are not loaded because
their providers have deployment-selected endpoints rather than fixed hosts.
Add support only after selecting and documenting their actual hostnames. These
provider entries may coexist with `api.github.com` and the Jira site.

See [provider-key setup](Language%20Model%20Provider%20Configuration.md#obtain-provider-api-keys)
for official key pages, billing and regional considerations, and key-rotation guidance.

Protect the file before loading the environment:

```sh
chmod 600 ~/.netrc
```

`xWalkEnv.sh` uses `Path.home() / ".netrc"` automatically. The
`XWALK_NETRC_FILE` path override exists for isolated tests, CI, or an exceptional
protected location; it must never contain credential values itself.

## Encrypt

The preferred command reads the protected external configuration file:

```sh
python3 xWalkTool/py-agent/dev-tool/xWalkLicenseTool encrypt --config /secure/location/xWalkLicense.cfg
```

For a small manual model selection, repeat `--env`:

```sh
python3 xWalkTool/py-agent/dev-tool/xWalkLicenseTool encrypt --env OPENAI_MODEL=<model-name> --env GEMINI_MODEL=<model-name>
```

Values supplied through `--env` can appear in shell history and process
listings. A protected configuration outside the repository is preferred. API keys
are rejected from both input methods. The two model-input methods are mutually
exclusive, and duplicate `--env` names are rejected.

Successful encryption writes only `xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY`. It uses a
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
python3 xWalkTool/py-agent/dev-tool/xWalkLicenseTool decrypt --output /tmp/xWalkLicense.decrypted.json
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
source xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh
```

The loader requires every model name in the committed template and every
credential machine listed above. It validates netrc mode `0600`, exact machine
names, matching login names, and non-empty passwords before exporting anything.
The encryption tool can protect a smaller valid model object for other
consumers, but the complete xWalk runtime environment rejects incomplete input.
The loader authenticates and validates `X_WALK_LICENSE_SERIAL` but does not
export it as an environment variable.

## Commit and deployment policy

The following files may be committed:

- the empty model-only `xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg` template;
- the licence tool, loader, tests, and documentation.

Never commit `xWalk-rpi5/xWalkLibrary/X_WALK_LICENSE.KEY`, `.netrc`, `*.netrc`, a filled
JSON copy, decrypted JSON or environment output, the decryption key, shell
history containing plaintext, or an unencrypted credential. The root
`.gitignore` excludes the encrypted licence and common private-file patterns
without excluding the empty template. Review staged changes manually before
every commit; ignore rules do not remove a secret that is already tracked. If a
secret enters Git history, revoke or rotate it and rewrite history through the
repository's approved incident process. Do not rely on a later deletion.

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

Authenticated encryption detects accidental or malicious model-setting
modification. Netrc permission checks reduce accidental local disclosure but do
not encrypt its API keys. When xWalk is distributed to untrusted devices, keep
high-value paid API keys on a backend service and expose only a narrowly scoped
device-facing interface.

The supplied systemd service does not embed or non-interactively retrieve a
decryption key. Automated service startup requires a separate operating-system
credential or secret-agent design; do not solve that by placing the key in a
unit, environment file, script, configuration file, or command line.

See the [xWalk licence tool guide](xWalk%20Licence%20Tool%20Guide.md) for the
complete command and exit contract. See the
[xWalk environment loader guide](xWalk%20Environment%20Loader%20Guide.md) for
sourced-shell behavior, validation order, cleanup, and environment lifetime.
