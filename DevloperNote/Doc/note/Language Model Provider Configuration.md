# Language-Model Provider Configuration

The three voice Agents remain provider-neutral:

- `xWalkLocalVoiceChatbot` coordinates listening, prompting, and speech.
- `xWalkVoiceActiveCar` adds sensors, camera input, and bounded actions.
- `xWalkVoiceActiveCarGpt` supplies the Buddy English behavior profile.

`xWalkBootRpi` selects the HTTP backend from the layered deployment
configuration. The tracked `xWalkController/xWalkConfig/picar-x.conf` manifest
includes functional fragments below `picar-x.d`. An installed deployment
normally uses `/var/lib/xwalk/picar-x.conf` and `/var/lib/xwalk/picar-x.d`;
`/etc/xwalk` contains the administrator-controlled templates.

Generic provider profiles are separate files below `picar-x.d/ai/providers`:

- `ollama.conf`;
- `openai.conf`;
- `gemini.conf`;
- `anthropic.conf`;
- `openai-compatible.conf`.

Enable exactly one provider include in the manifest. Ollama is enabled by
default. The adjacent `ai/features.conf` retains settings for commands with a
fixed backend, including local voice chat, Rolly, GPT car, and text vision.

## Configuration keys

| Key | Purpose |
| --- | --- |
| `voice_language_model_provider` | Selects the HTTP dialect and validation policy |
| `voice_language_model_endpoint` | Complete chat endpoint URL |
| `voice_language_model_model_environment` | Environment variable containing the model identifier |
| `voice_language_model_api_key_environment` | Environment variable containing the bearer credential |
| `voice_language_model_maximum_output_tokens` | Non-zero requested response bound |

The credential and generic model selection are not stored in controller
configuration. Provider files contain only their environment-variable names.
Neither value is added to conversation history or Doctor output. Calibration
writes remain in the primary file; included defaults are never rewritten by
`XWalkConfigStore`.

`xWalkTool/shell/xWalkEnv.sh` is the reviewed environment-loader boundary. It
uses `xWalkTool/python/xWalkLicenseTool` to authenticate and decrypt the
fixed `xWalkLibrary/X_WALK_LICENSE.KEY` model settings, then reads API
credentials from the developer's mode-`0600` `~/.netrc`. It validates every
supported model and credential name before exporting anything, never evaluates
values as shell syntax, and removes its mode-`0600` temporary JSON file. Source
the loader so the variables remain in the calling shell:

```sh
source xWalkTool/shell/xWalkEnv.sh
```

Create the encrypted model file from the committed empty template or repeated
model-only `--env` arguments, and configure actual provider hosts by following
the [licence-key workflow](License%20Key%20Workflow.md). Keep the generated
decryption key outside the repository. The loader requests it interactively and
rejects an unprovisioned, modified, incomplete, or group/world-readable file.

For development, create the already-ignored local file and restrict its mode:

```sh
cp xWalkController/xWalkConfig/picar-x.conf xWalkController/xWalkConfig/picar-x.local.conf
chmod 0600 xWalkController/xWalkConfig/picar-x.local.conf
```

Select a different provider include in `picar-x.local.conf`. Provision the
complete model template and private netrc credentials, then export the combined
variables by sourcing `xWalkEnv.sh`.

Run a voice command with that explicit absolute path:

```sh
/usr/bin/xwalk-picarx-control --deployment-config /absolute/path/to/xWalkController/xWalkConfig/picar-x.local.conf voice-chat start
```

Do not add `.netrc` or a real API key to a tracked provider file, `picar-x.conf`,
documentation, tests, logs, shell history, systemd environment file, or service
unit. Rotate a key immediately if exposed.

## Provider values

| Provider | Model environment | Credential environment |
| --- | --- | --- |
| `ollama` | `OLLAMA_MODEL` | Empty; no credential |
| `openai` or `chatgpt` | `OPENAI_MODEL` | `OPENAI_API_KEY` |
| `gemini` | `GEMINI_MODEL` | `GEMINI_API_KEY` |
| `claude` or `anthropic` | `ANTHROPIC_MODEL` | `ANTHROPIC_API_KEY` |
| `openai_compatible` | `XWALK_AI_MODEL` | `XWALK_AI_API_KEY` |

Ollama uses an HTTP or HTTPS endpoint ending in `/api/chat`. Cloud providers
use their configured OpenAI-compatible HTTPS chat endpoints.

`xWalkEnv.sh` loads only credentials whose fixed provider hostname is documented
in the licence-key workflow. It does not load `XWALK_AI_API_KEY` for the generic
`openai_compatible` profile until that deployment selects and documents its
actual endpoint hostname. The same restriction applies to generic
`LLM_API_KEY` and `OTHERS_API_KEY` example credentials.

The firmware does not embed default cloud model names. Provider model catalogs
change independently of this repository, so configure an exact model supported
by the selected account and endpoint.

## Obtain provider API keys

Create keys only for the models and providers the deployment plans to use. Most
paid providers require billing details, prepaid credit, or an active balance
before a new key can make successful requests.

| Provider | Official key page | Setup action |
| --- | --- | --- |
| OpenAI | [API keys][openai-keys] | Create a secret key in the selected project and copy it once |
| Anthropic/Claude | [Console keys][anthropic-keys] | Open API Keys and create a key |
| Google Gemini | [AI Studio keys][gemini-keys] | Create a key in a new or existing Cloud project |
| DeepSeek | [Platform keys][deepseek-keys] | Register, add balance when required, and create a key |
| Grok/xAI | [xAI Console][xai-keys] | Create a team, configure billing or credits, and create a key |
| Qwen | [Model Studio][qwen-keys] | Select the required region and workspace, then create a key |
| Doubao | [Ark Console][doubao-keys] | Activate Ark, enable a model, and create a key |

Provider-specific considerations:

- An OpenAI ChatGPT Plus or Pro subscription does not automatically include
  OpenAI API credit; API billing is managed separately.
- Grok is an xAI model family. One xAI key supplies both `GROK_API_KEY` and
  `XAI_API_KEY` in the current environment loader.
- Qwen API keys and hostnames are region-specific. A Swedish deployment should
  select an international or European region when the account offers one, then
  use the hostname assigned to that region or workspace.
- Volcengine Ark can require regional account verification. International users
  may need the corresponding BytePlus ModelArk offering.
- Google AI Studio can offer a limited Gemini free quota. Availability, models,
  and regional limits are controlled by Google and can change independently.

### Custom credential variables

The following variables do not identify external providers and therefore have
no official key-download page:

```text
LLM_API_KEY
OTHERS_API_KEY
XWALK_AI_API_KEY
```

Use `LLM_API_KEY` only as an explicitly configured default-provider credential.
Use `OTHERS_API_KEY` only after naming and configuring another provider. Use
`XWALK_AI_API_KEY` only when a deployment-owned xWalk-compatible service
authenticates clients. Generate a strong credential for such a service with:

```sh
openssl rand -hex 32
```

These generic variables remain unsupported by `xWalkEnv.sh` until the deployment
selects and documents their actual endpoint hostnames.

### Store a created key

After creating an OpenAI key, for example, update the existing host entry in
the developer's private `~/.netrc`:

```netrc
machine api.openai.com
    login OPENAI_API_KEY
    password sk-your-real-openai-key
```

Protect the file after editing it:

```sh
chmod 600 ~/.netrc
```

API secrets are commonly displayed only once. If a key is lost, revoke it and
create a replacement. Never paste a real key into an AI conversation,
screenshot, GitHub issue, source file, tracked configuration, documentation,
test fixture, or publicly shared terminal output.

[openai-keys]: https://platform.openai.com/api-keys
[anthropic-keys]: https://console.anthropic.com/settings/keys
[gemini-keys]: https://aistudio.google.com/app/apikey
[deepseek-keys]: https://platform.deepseek.com/api_keys
[xai-keys]: https://console.x.ai/
[qwen-keys]: https://bailian.console.aliyun.com/?apiKey=1
[doubao-keys]: https://console.volcengine.com/ark/

Claude's OpenAI compatibility interface is useful for compatibility testing,
but Anthropic does not recommend it as the long-term production path for most
use cases. A native Claude dialect should be added before relying on
Claude-specific features.

Kiro is not accepted as a provider. Its documented non-interactive CLI does not
define a model-selectable HTTP inference protocol suitable for this HAL. Adding
Kiro requires a documented API contract, authentication scheme, bounded request
and response formats, and deterministic host tests.

## Example values

Ollama:

```ini
voice_language_model_provider = ollama
voice_language_model_endpoint = http://127.0.0.1:11434/api/chat
voice_language_model_model_environment = OLLAMA_MODEL
voice_language_model_api_key_environment =
```

ChatGPT through OpenAI:

```ini
voice_language_model_provider = openai
voice_language_model_endpoint = https://api.openai.com/v1/chat/completions
voice_language_model_model_environment = OPENAI_MODEL
voice_language_model_api_key_environment = OPENAI_API_KEY
```

Gemini compatibility endpoint:

```ini
voice_language_model_provider = gemini
voice_language_model_endpoint = https://generativelanguage.googleapis.com/v1beta/openai/chat/completions
voice_language_model_model_environment = GEMINI_MODEL
voice_language_model_api_key_environment = GEMINI_API_KEY
```

Claude compatibility endpoint:

```ini
voice_language_model_provider = claude
voice_language_model_endpoint = https://api.anthropic.com/v1/chat/completions
voice_language_model_model_environment = ANTHROPIC_MODEL
voice_language_model_api_key_environment = ANTHROPIC_API_KEY
```

## Validation

Doctor validates configuration without contacting a model service. It verifies
the native Ollama executable and configured model manifest, or validates that a
cloud provider has HTTPS and non-empty values in the configured model and
credential environment variables. It never prints the credential value.

```sh
/usr/bin/xwalk-picarx-control --deployment-config /var/lib/xwalk/picar-x.conf doctor
```

Host tests inject an HTTP callback. They verify dialect mapping, endpoint and
credential validation, compatible JSON, authorization-header separation,
response parsing, and explicit Kiro rejection without network or hardware use.

Raspberry Pi acceptance still requires a deployment-owned account, network and
TLS policy, an allowed model, audio hardware, and a physically secured vehicle.
