#!/usr/bin/env bash
# Verify committed fixture checksums and validate the reviewed manifest.

set -euo pipefail

script_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
python3 "${script_directory}/validate-assets.py" "${script_directory}"

