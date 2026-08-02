#!/usr/bin/env bash

set -eu

repository_root="$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)"
configuration_file="$repository_root/xWalkCLI/xWalkController/config/picar-x.conf"

test -r "$configuration_file"
grep -q '^voice_language_model_provider = ollama$' "$configuration_file"
grep -q '^voice_language_model_endpoint = http://127.0.0.1:11434/api/chat$' \
    "$configuration_file"
grep -q '^voice_language_model_model = qwen2.5:0.5b$' "$configuration_file"
grep -q '^voice_language_model_api_key =$' "$configuration_file"
grep -q '^voice_language_model_maximum_output_tokens = 1024$' \
    "$configuration_file"

if grep -Eq '^voice_language_model_api_key = .+' "$configuration_file"; then
    echo "Tracked language-model configuration contains a credential." >&2
    exit 1
fi

echo "Language-model configuration template tests passed."
