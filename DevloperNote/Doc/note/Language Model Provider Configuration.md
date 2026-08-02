# Language-Model Provider Configuration

The three voice Agents remain provider-neutral:

- `xWalkLocalVoiceChatbot` coordinates listening, prompting, and speech.
- `xWalkVoiceActiveCar` adds sensors, camera input, and bounded actions.
- `xWalkVoiceActiveCarGpt` supplies the Buddy English behavior profile.

`xWalkBootRpi` selects the HTTP backend from the deployment configuration. The
tracked template is
`xWalkCLI/xWalkController/config/picar-x.conf`. An installed deployment normally
uses `/var/lib/xwalk/picar-x.conf`; `/etc/xwalk/picar-x.conf` is the immutable
administrator-controlled template.

## Configuration keys

| Key | Purpose |
| --- | --- |
| `voice_language_model_provider` | Selects the HTTP dialect and validation policy |
| `voice_language_model_endpoint` | Complete chat endpoint URL |
| `voice_language_model_model` | Exact deployment-selected model identifier |
| `voice_language_model_api_key` | Bearer credential; empty only for Ollama |
| `voice_language_model_maximum_output_tokens` | Non-zero requested response bound |

The key is never included in model JSON, conversation history, or Doctor output.
The configuration file itself contains the credential, so keep a populated copy
out of source control and restrict it to the runtime user and administrator.

For development, create the already-ignored local file and restrict its mode:

```sh
cp xWalkCLI/xWalkController/config/picar-x.conf xWalkCLI/xWalkController/config/picar-x.local.conf
chmod 0600 xWalkCLI/xWalkController/config/picar-x.local.conf
```

Run a voice command with that explicit absolute path:

```sh
/usr/bin/xwalk-picarx-control --deployment-config /absolute/path/to/xWalkCLI/xWalkController/config/picar-x.local.conf voice-chat start
```

Do not add a real API key to `picar-x.conf`, documentation, tests, logs, shell
history, or a service unit. Rotate a key immediately if it is exposed.

## Provider values

| Provider | Endpoint contract | API key |
| --- | --- | --- |
| `ollama` | HTTP or HTTPS URL ending in `/api/chat` | Must be empty |
| `openai` or `chatgpt` | HTTPS URL ending in `/chat/completions` | Required |
| `gemini` | Gemini OpenAI-compatible HTTPS chat endpoint | Required |
| `claude` or `anthropic` | Claude OpenAI-compatible HTTPS chat endpoint | Required |
| `openai_compatible` | Deployment-verified compatible HTTPS endpoint | Required |

The firmware does not embed default cloud model names. Provider model catalogs
change independently of this repository, so configure an exact model supported
by the selected account and endpoint.

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
voice_language_model_model = qwen2.5:0.5b
voice_language_model_api_key =
```

ChatGPT through OpenAI:

```ini
voice_language_model_provider = openai
voice_language_model_endpoint = https://api.openai.com/v1/chat/completions
voice_language_model_model = DEPLOYMENT_SELECTED_MODEL
voice_language_model_api_key = DEPLOYMENT_SECRET
```

Gemini compatibility endpoint:

```ini
voice_language_model_provider = gemini
voice_language_model_endpoint = https://generativelanguage.googleapis.com/v1beta/openai/chat/completions
voice_language_model_model = DEPLOYMENT_SELECTED_MODEL
voice_language_model_api_key = DEPLOYMENT_SECRET
```

Claude compatibility endpoint:

```ini
voice_language_model_provider = claude
voice_language_model_endpoint = https://api.anthropic.com/v1/chat/completions
voice_language_model_model = DEPLOYMENT_SELECTED_MODEL
voice_language_model_api_key = DEPLOYMENT_SECRET
```

## Validation

Doctor validates configuration without contacting a model service. It verifies
the native Ollama executable and configured model manifest, or validates that a
cloud provider has HTTPS, a model, and a non-empty credential. It never prints
the credential.

```sh
/usr/bin/xwalk-picarx-control --deployment-config /var/lib/xwalk/picar-x.conf doctor
```

Host tests inject an HTTP callback. They verify dialect mapping, endpoint and
credential validation, compatible JSON, authorization-header separation,
response parsing, and explicit Kiro rejection without network or hardware use.

Raspberry Pi acceptance still requires a deployment-owned account, network and
TLS policy, an allowed model, audio hardware, and a physically secured vehicle.
