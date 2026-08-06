#!/usr/bin/env bash

# Loads authenticated xWalk AI environment values into the calling shell.
# Source this file; executing it cannot update its parent process environment.

_xwalk_load_environment() {
    local script_directory repository_root license_file license_mode license_tool
    local template_file decrypted_file name value record_index sentinel
    local -a decoded_records=()
    local -A decoded_values=()

    script_directory="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
    repository_root="$(CDPATH='' cd -- "$script_directory/../.." && pwd)"
    license_file="$repository_root/xWalkLibrary/X_WALK_LICENSE.KEY"
    license_tool="$repository_root/xWalkTool/python/xWalkLicenseTool"
    template_file="$repository_root/xWalkTool/environment/xWalkLicense.json"

    if [ ! -f "$license_file" ] || [ ! -r "$license_file" ]; then
        echo "xWalk environment: encrypted licence file is unreadable: $license_file" >&2
        return 2
    fi
    license_mode="$(stat -c '%a' "$license_file")" || return 2
    if (( (8#$license_mode & 077) != 0 )); then
        echo "xWalk environment: encrypted licence file must have mode 0600" >&2
        return 2
    fi
    if [ ! -r "$license_tool" ]; then
        echo "xWalk environment: licence tool is unavailable: $license_tool" >&2
        return 2
    fi
    if [ ! -r "$template_file" ]; then
        echo "xWalk environment: licence template is unavailable: $template_file" >&2
        return 2
    fi

    decrypted_file="$(mktemp "${TMPDIR:-/tmp}/xwalk-license.XXXXXX.json")" || return 2
    chmod 0600 "$decrypted_file" || {
        rm -f "$decrypted_file"
        return 2
    }
    if ! python3 "$license_tool" decrypt --output "$decrypted_file" >/dev/null; then
        rm -f "$decrypted_file"
        echo "xWalk environment: licence-key decryption failed" >&2
        return 2
    fi

    mapfile -d '' -t decoded_records < <(
        python3 -c 'import json, os, re, sys
with open(sys.argv[1], encoding="utf-8") as input_file:
    values = json.load(input_file)
with open(sys.argv[2], encoding="utf-8") as input_file:
    template = json.load(input_file)
valid_name = re.compile(r"^[A-Za-z_][A-Za-z0-9_]*$").fullmatch
if not isinstance(template, dict) or not template:
    raise SystemExit("licence template root must be a non-empty object")
if any(not isinstance(name, str) or not valid_name(name) for name in template):
    raise SystemExit("licence template contains an invalid variable name")
if any(not isinstance(value, str) or value for value in template.values()):
    raise SystemExit("licence template values must remain empty strings")
serial_name = "X_WALK_LICENSE_SERIAL"
serial_pattern = re.compile(r"^XWALK-[0-9]{4}-[0-9A-F]{8}$").fullmatch
if not isinstance(values.get(serial_name), str) or not serial_pattern(values[serial_name]):
    raise SystemExit("decrypted licence contains an invalid serial number")
if set(values) - {serial_name} != set(template):
    raise SystemExit("decrypted licence does not match the template variable names")
for name in sorted(template):
    os.write(1, name.encode("utf-8") + b"\0" + values[name].encode("utf-8") + b"\0")
os.write(1, b"XWALK_LICENSE_RECORDS_COMPLETE\0")' \
            "$decrypted_file" "$template_file"
    )
    rm -f "$decrypted_file"
    sentinel="XWALK_LICENSE_RECORDS_COMPLETE"
    if [ "${#decoded_records[@]}" -lt 1 ] ||
        [ "${decoded_records[-1]}" != "$sentinel" ] ||
        (( (${#decoded_records[@]} - 1) % 2 != 0 )); then
        echo "xWalk environment: decrypted licence does not match the template" >&2
        return 2
    fi
    unset 'decoded_records[-1]'

    for ((record_index = 0; record_index < ${#decoded_records[@]}; record_index += 2)); do
        name="${decoded_records[$record_index]}"
        value="${decoded_records[$((record_index + 1))]}"
        decoded_values[$name]="$value"
    done
    for name in "${!decoded_values[@]}"; do
        printf -v "$name" '%s' "${decoded_values[$name]}"
        export "$name"
    done
    return 0
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
    echo "Source xWalkEnv.sh so it can update the calling shell environment." >&2
    exit 2
fi

_xwalk_load_environment
_xwalk_environment_status=$?
unset -f _xwalk_load_environment
if [ "$_xwalk_environment_status" -eq 0 ]; then
    unset _xwalk_environment_status
    return 0
fi
unset _xwalk_environment_status
return 2
