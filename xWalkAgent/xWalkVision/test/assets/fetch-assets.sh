#!/usr/bin/env bash
# Reproducibly fetch and derive the small recorded-scenario fixtures.

set -euo pipefail

script_directory="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cache_directory="${XWALK_ASSET_CACHE:-${TMPDIR:-/tmp}/xwalk-vision-asset-cache}"
opencv_revision="71d3237a093b60a27601c20e9ee6c3e52154e8b1"
opencv_sha256="45cddc9490be69345cbdab64ca583be65987e864ca408038e648db99e10516cf"
bicycle_sha256="1612ecaf8d0ee94327c5aacb9b13bb19b6a9e46624a4937b922b3130aaaf8e58"
opencv_source="${cache_directory}/opencv-vtest.avi"
bicycle_source="${cache_directory}/bicyclist-crossing.jpg"

mkdir -p "${cache_directory}"
if ! printf '%s  %s\n' "${opencv_sha256}" "${opencv_source}" | sha256sum --check --status 2>/dev/null; then
    curl --fail --location --retry 3 --user-agent 'xWalk-test-fixture-fetch/1.0' \
        --output "${opencv_source}" \
        "https://raw.githubusercontent.com/opencv/opencv/${opencv_revision}/samples/data/vtest.avi"
fi
if ! printf '%s  %s\n' "${bicycle_sha256}" "${bicycle_source}" | sha256sum --check --status 2>/dev/null; then
    curl --fail --location --retry 3 --user-agent 'xWalk-test-fixture-fetch/1.0' \
        --output "${bicycle_source}" \
        'https://upload.wikimedia.org/wikipedia/commons/d/d1/Bicyclist_Crossing_the_Street.jpg'
fi
printf '%s  %s\n' "${opencv_sha256}" "${opencv_source}" | sha256sum --check --status
printf '%s  %s\n' "${bicycle_sha256}" "${bicycle_source}" | sha256sum --check --status

derive_video() {
    local scenario="$1"
    local start="$2"
    local filter="$3"
    local destination="${script_directory}/recorded_scenarios/${scenario}/scenario.avi"
    mkdir -p "$(dirname "${destination}")"
    ffmpeg -nostdin -loglevel error -y -ss "${start}" -i "${opencv_source}" -t 1.0 \
        -an -vf "fps=5,scale=320:240:flags=area,${filter}" -c:v mjpeg -q:v 8 "${destination}"
}

derive_video pedestrian_entering_crosswalk 20.0 'null'
derive_video pedestrian_standing_safely 28.0 'null'
derive_video vehicle_approaching_pedestrian 45.0 'null'
derive_video multiple_road_users 35.0 'null'
derive_video partial_occlusion 39.0 'drawbox=x=120:y=0:w=80:h=240:color=black@1:t=fill'
derive_video poor_lighting 28.0 'eq=brightness=-0.45:contrast=1.15'
derive_video motion_blur 20.0 'avgblur=sizeX=9:sizeY=3'
derive_video empty_road 1.0 'null'
derive_video false_positive_challenge 8.0 'null'
derive_video camera_interruption 20.0 'null'
derive_video end_of_video 20.0 'null'

bicycle_destination="${script_directory}/recorded_scenarios/bicycle_crossing/scenario.avi"
mkdir -p "$(dirname "${bicycle_destination}")"
ffmpeg -nostdin -loglevel error -y -loop 1 -i "${bicycle_source}" -t 1.0 -an \
    -vf 'fps=5,scale=320:240:force_original_aspect_ratio=increase,crop=320:240' \
    -c:v mjpeg -q:v 8 "${bicycle_destination}"

malformed_directory="${script_directory}/malformed_images"
mkdir -p "${malformed_directory}"
printf '\377\330\377\340truncated' > "${malformed_directory}/truncated.jpg"
printf 'not-an-image\n' > "${malformed_directory}/text-as-image.jpg"

"${script_directory}/verify-assets.sh"
