#!/usr/bin/env python3
"""Validate xWalk recorded-scenario metadata and committed checksums."""

import hashlib
import json
import pathlib
import sys


REQUIRED_SCENARIOS = {
    "pedestrian_entering_crosswalk",
    "pedestrian_standing_safely",
    "vehicle_approaching_pedestrian",
    "bicycle_crossing",
    "multiple_road_users",
    "partial_occlusion",
    "poor_lighting",
    "motion_blur",
    "empty_road",
    "false_positive_challenge",
    "camera_interruption",
    "end_of_video",
}


def fail(message: str) -> None:
    raise SystemExit(f"asset validation failed: {message}")


def main() -> None:
    root = pathlib.Path(sys.argv[1] if len(sys.argv) == 2 else __file__).resolve()
    if root.is_file():
        root = root.parent
    manifest = json.loads((root / "manifests/manifest.json").read_text(encoding="utf-8"))
    annotations = json.loads(
        (root / "manifests/annotations.json").read_text(encoding="utf-8")
    )
    source_ids = {source["identifier"] for source in manifest["sources"]}
    scenarios = set()
    for asset in manifest["assets"]:
        required = {
            "repository_filename", "sha256", "source", "scenario", "transformation",
            "expected_duration", "expected_frame_count",
        }
        if not required.issubset(asset):
            fail(f"manifest entry is missing fields: {asset}")
        if asset["source"] not in source_ids:
            fail(f"unknown source {asset['source']}")
        path = root / asset["repository_filename"]
        if not path.is_file():
            fail(f"missing {asset['repository_filename']}")
        digest = hashlib.sha256(path.read_bytes()).hexdigest()
        if digest != asset["sha256"]:
            fail(f"checksum mismatch for {asset['repository_filename']}")
        scenarios.add(asset["scenario"])
    if scenarios != REQUIRED_SCENARIOS:
        fail(f"scenario set differs: {sorted(scenarios ^ REQUIRED_SCENARIOS)}")
    annotation_ids = {scenario["scenario_identifier"] for scenario in annotations["scenarios"]}
    if annotation_ids != REQUIRED_SCENARIOS:
        fail(f"annotation set differs: {sorted(annotation_ids ^ REQUIRED_SCENARIOS)}")
    for scenario in annotations["scenarios"]:
        if scenario["media_sha256"] not in {asset["sha256"] for asset in manifest["assets"]}:
            fail(f"unreferenced media checksum for {scenario['scenario_identifier']}")
        if not scenario["frames"]:
            fail(f"no frame expectations for {scenario['scenario_identifier']}")
        previous_index = -1
        previous_timestamp = -1.0
        for frame in scenario["frames"]:
            if frame["frame_index"] <= previous_index:
                fail(f"non-monotonic frame index in {scenario['scenario_identifier']}")
            if frame["timestamp_seconds"] <= previous_timestamp:
                fail(f"non-monotonic timestamp in {scenario['scenario_identifier']}")
            previous_index = frame["frame_index"]
            previous_timestamp = frame["timestamp_seconds"]
    print(f"validated {len(manifest['assets'])} assets and {len(annotations['scenarios'])} annotations")


if __name__ == "__main__":
    main()

