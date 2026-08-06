# xWalk licence tool guide

[`xWalkTool/python/xWalkLicenseTool`](../../../xWalkTool/python/xWalkLicenseTool)
creates and opens the authenticated xWalk licence file. It is an executable
Python script without a `.py` filename suffix.

## Responsibility

The tool:

- reads environment values from one protected JSON file or repeated `--env` arguments;
- validates names, string types, duplicates, and empty values before encryption;
- generates a random 256-bit SecretBox key and a fresh authenticated nonce;
- generates one `XWALK-<UTC_YEAR>-<HEX>` serial and stores it in the encrypted payload;
- writes only `xWalkLibrary/X_WALK_LICENSE.KEY` during encryption;
- authenticates encrypted data before writing decrypted JSON with mode `0600`; and
- reports variable names and counts without printing plaintext values.

It never writes values into the committed template, embeds the decryption key,
or accepts a decryption key as a normal command-line argument.

## Python environment

Create and activate the ignored project environment from the repository root:

```sh
python3 -m venv .xWalkPyEnv
source .xWalkPyEnv/bin/activate
python -m pip install --upgrade pip
python -m pip install PyNaCl
```

The system-package alternative on Debian-family systems is
`sudo apt-get install python3-nacl`.

## Prepare JSON input

The committed `xWalkTool/environment/xWalkLicense.json` file is an empty
template. Copy it outside the repository, restrict the copy, and fill only the
external file:

```sh
install -m 0600 xWalkTool/environment/xWalkLicense.json /secure/location/xWalkLicense.json
```

Running encryption against the empty committed template fails intentionally.

## Encrypt

The executable can be invoked directly because it has a Python 3 shebang and
executable permissions:

```sh
xWalkTool/python/xWalkLicenseTool encrypt --json /secure/location/xWalkLicense.json
```

Calling it explicitly through Python 3 is equivalent:

```sh
python3 xWalkTool/python/xWalkLicenseTool encrypt --json /secure/location/xWalkLicense.json
```

Repeated manual values are also supported:

```sh
xWalkTool/python/xWalkLicenseTool encrypt --env OPENAI_API_KEY='value' --env GEMINI_API_KEY='value'
```

Command-line values can be exposed through shell history or process listings.
Prefer the protected JSON method. Never type angle-bracket placeholders as
literal values because shells interpret `<` and `>` as redirection operators.

After the encrypted file has been written successfully, the tool prints the
file path, one licence serial, and the decryption key exactly once. Store the
key outside the repository in a password manager or secret service.

## Decrypt

Choose an explicit temporary output outside the source tree:

```sh
xWalkTool/python/xWalkLicenseTool decrypt --output /tmp/xWalkLicense.decrypted.json
```

The tool requests the key privately with `getpass`. An incorrect key or changed
ciphertext fails authentication and returns a nonzero status. A successful
output file has mode `0600`; remove it after its consumer has loaded the values.

## Exit and output contract

Successful commands return zero. Validation, file, authentication, or key
errors return `2` and write a value-free diagnostic to standard error. Failed
encryption does not print a serial number or decryption key.

## Verification

Run the host-only fake-secret test suite:

```sh
python3 xWalkTool/python/test/test_xWalkLicenseTool.py
```

The tests use temporary directories and do not use paid-provider credentials.
See the [licence-key workflow](License%20Key%20Workflow.md) for commit,
deployment, and device-trust limitations.
