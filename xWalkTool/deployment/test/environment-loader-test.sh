#!/usr/bin/env bash

set -eu

repository_root="$(CDPATH='' cd -- "$(dirname -- "$0")/../../.." && pwd)"
fixture_root="$(mktemp -d)"
trap 'rm -rf "$fixture_root"' EXIT

mkdir -p "$fixture_root/xWalkLibrary" "$fixture_root/xWalkTool/python" \
    "$fixture_root/xWalkTool/shell" "$fixture_root/xWalkTool/environment"
cp "$repository_root/xWalkTool/python/xWalkLicenseTool" \
    "$fixture_root/xWalkTool/python/xWalkLicenseTool"
cp "$repository_root/xWalkTool/shell/xWalkEnv.sh" \
    "$fixture_root/xWalkTool/shell/xWalkEnv.sh"
cp "$repository_root/xWalkTool/environment/xWalkLicense.json" \
    "$fixture_root/xWalkTool/environment/xWalkLicense.json"

fixture_json="$fixture_root/private-license.json"
cat > "$fixture_json" <<'EOF'
{
    "ANTHROPIC_API_KEY": "anthropic-fake-value",
    "ANTHROPIC_MODEL": "claude-fake-model",
    "DEEPSEEK_API_KEY": "deepseek-fake-value",
    "DOUBAO_API_KEY": "doubao-fake-value",
    "GEMINI_API_KEY": "gemini-fake-value",
    "GEMINI_MODEL": "gemini-fake-model",
    "GROK_API_KEY": "grok-fake-value",
    "LLM_API_KEY": "generic-fake-value",
    "OLLAMA_MODEL": "ollama-fake-model",
    "OPENAI_API_KEY": "openai-fake-value",
    "OPENAI_MODEL": "openai-fake-model",
    "OTHERS_API_KEY": "others-fake-value",
    "QWEN_API_KEY": "qwen-fake-value",
    "XAI_API_KEY": "xai-fake-value",
    "XWALK_AI_API_KEY": "compatible-fake-value",
    "XWALK_AI_MODEL": "$(touch should-not-run)"
}
EOF
chmod 0600 "$fixture_json"

encryption_output="$(
    python3 "$fixture_root/xWalkTool/python/xWalkLicenseTool" encrypt \
        --json "$fixture_json"
)"
decryption_key="$(
    printf '%s\n' "$encryption_output" | sed -n '/^Decryption key:$/ {n;p;}'
)"
test -n "$decryption_key"
license_file="$fixture_root/xWalkLibrary/X_WALK_LICENSE.KEY"
test "$(stat -c '%a' "$license_file")" = 600

printf '%s\n' "$decryption_key" | PYTHONWARNINGS=ignore bash -c '
    source "$1"
    test "$OPENAI_API_KEY" = openai-fake-value
    test "$OPENAI_MODEL" = openai-fake-model
    test "$GEMINI_API_KEY" = gemini-fake-value
    test "$GEMINI_MODEL" = gemini-fake-model
    test "$ANTHROPIC_API_KEY" = anthropic-fake-value
    test "$ANTHROPIC_MODEL" = claude-fake-model
    test "$XWALK_AI_API_KEY" = compatible-fake-value
    test "$XWALK_AI_MODEL" = "\$(touch should-not-run)"
    test "$OLLAMA_MODEL" = ollama-fake-model
    test -z "${X_WALK_LICENSE_SERIAL+x}"
' _ "$fixture_root/xWalkTool/shell/xWalkEnv.sh" 2> "$fixture_root/loader.err"
if grep -Eq '(openai|gemini|anthropic|compatible)-fake-value' \
    "$fixture_root/loader.err"; then
    echo "xWalkEnv.sh exposed a fake credential in diagnostics." >&2
    exit 1
fi
test ! -e "$fixture_root/should-not-run"

chmod 0644 "$license_file"
if printf '%s\n' "$decryption_key" | PYTHONWARNINGS=ignore \
    bash -c 'source "$1"' _ "$fixture_root/xWalkTool/shell/xWalkEnv.sh" \
    > /dev/null 2>&1; then
    echo "xWalkEnv.sh accepted a group/world-readable encrypted licence." >&2
    exit 1
fi
chmod 0600 "$license_file"

incomplete_output="$(
    python3 "$fixture_root/xWalkTool/python/xWalkLicenseTool" encrypt \
        --env OPENAI_API_KEY=incomplete-fake-value
)"
incomplete_key="$(
    printf '%s\n' "$incomplete_output" | sed -n '/^Decryption key:$/ {n;p;}'
)"
printf '%s\n' "$incomplete_key" | PYTHONWARNINGS=ignore bash -c '
    export OPENAI_API_KEY=original-value
    if source "$1" > /dev/null 2>&1; then
        exit 1
    fi
    test "$OPENAI_API_KEY" = original-value
' _ "$fixture_root/xWalkTool/shell/xWalkEnv.sh"

if "$fixture_root/xWalkTool/shell/xWalkEnv.sh" > /dev/null 2>&1; then
    echo "Executing xWalkEnv.sh did not require sourcing." >&2
    exit 1
fi

echo "xWalk environment-loader host tests passed."
