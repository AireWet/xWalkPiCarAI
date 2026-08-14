#!/usr/bin/env python3
"""Run the complete xWalk host-quality gate for one Gerrit patch set."""

from __future__ import annotations

import os
from concurrent.futures import ThreadPoolExecutor, as_completed
from pathlib import Path
import subprocess
from typing import Callable, TextIO


class XWalkGerritQuality:
    """Run host-quality jobs with resource isolation and retain all results."""

    def __init__(self, workspace: Path, log: TextIO) -> None:
        """Store the isolated checkout and combined verification log."""

        self.workspace = workspace
        self.product_workspace = workspace / "xWalk-rpi5"
        self.log = log

    def environment(self, overrides: dict[str, str] | None = None) -> dict[str, str]:
        """Return a clean command environment with GitHub-compatible paths."""

        environment = os.environ.copy()
        environment["GITHUB_WORKSPACE"] = str(self.workspace)
        if overrides is not None:
            environment.update(overrides)
        return environment

    def run_command(
        self,
        command: list[str],
        environment: dict[str, str] | None = None,
        working_directory: Path | None = None,
    ) -> bool:
        """Run one quality command and report whether its status is successful."""

        directory = self.workspace if working_directory is None else working_directory
        self.log.write(f"\n$ {' '.join(command)}\n")
        self.log.flush()
        result = subprocess.run(
            command,
            cwd=directory,
            env=self.environment(environment),
            stdout=self.log,
            stderr=subprocess.STDOUT,
            check=False,
            text=True,
        )
        return result.returncode == 0

    def run_steps(
        self,
        name: str,
        steps: list[tuple[list[str], dict[str, str] | None]],
        working_directory: Path | None = None,
    ) -> bool:
        """Run one job until its first failed step, matching GitHub Actions."""

        self.log.write(f"\n{'=' * 24} {name} {'=' * 24}\n")
        self.log.flush()
        passed = True
        for command, environment in steps:
            if passed:
                passed = self.run_command(command, environment, working_directory)
        result_text = "PASSED" if passed else "FAILED"
        self.log.write(f"\n[{result_text}] {name}\n")
        self.log.flush()
        return passed

    def compiler_steps(
        self, compiler: str, build_type: str,
    ) -> list[tuple[list[str], dict[str, str]]]:
        """Build one compiler matrix job without changing its commands."""

        build_directory = f"build-host/ci-{compiler}-{build_type}"
        environment = {
            "CC": "clang" if compiler == "clang" else "gcc",
            "CXX": "clang++" if compiler == "clang" else "g++",
        }
        executable_directory = self.workspace / build_directory
        configure = [
            "cmake", "--fresh", "-S", str(self.product_workspace),
            "-B", build_directory, "-G", "Ninja",
            f"-DCMAKE_BUILD_TYPE={build_type}", "-DXWALK_ENABLE_STRICT_WARNINGS=ON",
        ]
        ctest = ["ctest", "--test-dir", build_directory, "--output-on-failure", "--no-tests=error"]
        return [
            (configure, environment),
            (["cmake", "--build", build_directory, "--parallel"], environment),
            (ctest, environment), (ctest + ["-L", "agent-aggregate"], environment),
            (ctest + ["-L", "agent-group"], environment),
            (
                [str(executable_directory / "xGoogleTest"),
                 "TEST_SUITE_XWALK_SEQUENCE:1", "--gtest_brief=1"],
                environment,
            ),
            ([str(executable_directory / "xCliGoogleTest"), "--gtest_brief=1"], environment),
            ([str(executable_directory / "xCliSequenceTest"), "--gtest_brief=1"], environment),
        ]

    def compiler_job(self, compiler: str, build_type: str) -> bool:
        """Run one complete compiler and build-type matrix job."""

        return self.run_steps(f"{compiler} {build_type}", self.compiler_steps(compiler, build_type))

    def preset_job(
        self,
        name: str,
        preset: str,
        test_preset: str,
        environment: dict[str, str] | None = None,
        timeout: str | None = None,
        disable_aslr: bool = False,
    ) -> bool:
        """Configure, build and test one CMake preset-based job."""

        test_command = ["ctest", "--preset", test_preset]
        if timeout is not None:
            test_command.extend(["--timeout", timeout])
        if disable_aslr:
            test_command = ["setarch", "x86_64", "-R", *test_command]
        return self.run_steps(
            name,
            [
                (["cmake", "--fresh", "--preset", preset], environment),
                (["cmake", "--build", "--preset", preset, "--parallel"], environment),
                (test_command, environment),
            ],
            self.product_workspace,
        )

    def static_analysis_job(self) -> bool:
        """Run Clang-Tidy compilation and the configured Cppcheck target."""

        return self.run_steps(
            "static-analysis",
            [
                (["cmake", "--fresh", "--preset", "clang-tidy"], None),
                (["cmake", "--build", "--preset", "clang-tidy", "--parallel"], None),
                (["cmake", "--build", "../build-host/clang-tidy", "--target", "cppcheck"],
                 None),
            ],
            self.product_workspace,
        )

    def coverage_job(self) -> bool:
        """Run the enforced host coverage build, tests and report generation."""

        return self.run_steps(
            "coverage",
            [(["bash", "xWalkTool/shell-agent/quality-tool/run-host-coverage.sh", "run"], None)],
        )

    def deployment_scripts_job(self) -> bool:
        """Validate every deployment shell script and provisioning behavior."""

        shell_scripts = sorted(self.workspace.glob("xWalkTool/shell-agent/**/*.sh"))
        deployment_tests = sorted(self.workspace.glob("xWalkTool/shell-agent/deploy-tool/test/*.sh"))
        shellcheck_command = [
            "shellcheck",
            *(str(path.relative_to(self.workspace)) for path in shell_scripts),
            *(str(path.relative_to(self.workspace)) for path in deployment_tests),
        ]
        return self.run_steps(
            "deployment-scripts",
            [
                (shellcheck_command, None),
                (["bash", "xWalkTool/shell-agent/deploy-tool/test/setup-rpi-test.sh"], None),
            ],
        )

    @staticmethod
    def staged_required_paths() -> list[tuple[str, str]]:
        """Return every path required from the staged release installation."""

        return [
            ("-x", "usr/bin/xwalk-picarx-control"),
            ("-x", "usr/lib/xwalk/xWalkTool/shell-agent/env-tool/license/xWalkEnv.sh"),
            ("-x", "usr/lib/xwalk/xWalkTool/py-agent/dev-tool/xWalkLicenseTool"),
            ("-r", "usr/lib/xwalk/xWalkTool/shell-agent/env-tool/license/xWalkLicense.cfg"),
            ("!", "usr/lib/xwalk/xWalkLibrary/X_WALK_LICENSE.KEY"),
            ("-r", "etc/xwalk/picar-x.conf"),
            ("-r", "etc/xwalk/picar-x.d/hardware.conf"),
            ("-r", "etc/xwalk/picar-x.d/ai/providers/gemini.conf"),
            ("-r", "usr/lib/xwalk/libvosk.so"),
            ("-r", "usr/share/xwalk/models/vosk/vosk-model-small-en-us-0.15/am/final.mdl"),
            ("-r", "usr/share/xwalk/sounds/car-double-horn.wav"),
            ("-r", "usr/share/xwalk/sounds/car-start-engine.wav"),
            ("-r", "usr/share/xwalk/music/slow-trail-Ahjay_Stelino.mp3"),
        ]

    def staged_install_steps(self, deploy_directory: Path) -> list[tuple[list[str], dict[str, str] | None]]:
        """Build release installation, path, linkage, and integrity steps."""

        steps: list[tuple[list[str], dict[str, str] | None]] = [
            (
                ["cmake", "--fresh", "-S", str(self.product_workspace),
                 "-B", "build-host/install",
                 "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
                 "-DCMAKE_INSTALL_PREFIX=/usr"],
                None,
            ),
            (["cmake", "--build", "build-host/install", "--parallel"], None),
            (["cmake", "--install", "build-host/install"],
             {"DESTDIR": str(deploy_directory)}),
        ]
        for test_flag, relative_path in self.staged_required_paths():
            deployed_path = str(deploy_directory / relative_path)
            command = (
                ["test", "!", "-e", deployed_path]
                if test_flag == "!" else ["test", test_flag, deployed_path]
            )
            steps.append((command, None))
        return steps

    def staged_validation_steps(self, executable: Path) -> list[tuple[list[str], None]]:
        """Build runtime, relocation, permission, and checksum validation steps."""

        help_command = "cd /tmp && \"$GITHUB_WORKSPACE/build-host/deploy/usr/bin/xwalk-picarx-control\" --help"
        relocation = (
            "if grep -R --fixed-strings \"$GITHUB_WORKSPACE\" build-host/deploy/etc "
            "build-host/deploy/usr/share/xwalk/config; then exit 1; fi"
        )
        permissions = "test -z \"$(find build-host/deploy -type f -perm /022 -print -quit)\""
        checksum = (
            "find build-host/deploy -type f -print0 | sort -z | "
            "xargs -0 sha256sum > build-host/deploy.sha256"
        )
        return [
            (["bash", "-c", help_command], None), (["bash", "-c", relocation], None),
            (["ldd", str(executable)], None), (["bash", "-c", permissions], None),
            (["bash", "-c", checksum], None),
        ]

    def staged_install_job(self) -> bool:
        """Build, stage and validate the exact release installation layout."""

        deploy_directory = self.workspace / "build-host/deploy"
        steps = self.staged_install_steps(deploy_directory)
        executable = deploy_directory / "usr/bin/xwalk-picarx-control"
        steps.extend(self.staged_validation_steps(executable))
        return self.run_steps("staged-install", steps)

    def current_gerrit_job(self) -> bool:
        """Retain the original strict Gerrit Debug build and complete CTest run."""

        return self.run_steps(
            "gerrit-host",
            [
                (
                    ["cmake", "--fresh", "-S", str(self.product_workspace),
                     "-B", "build-host/gerrit",
                     "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Debug",
                     "-DXWALK_ENABLE_STRICT_WARNINGS=ON"],
                    None,
                ),
                (["cmake", "--build", "build-host/gerrit", "--parallel"], None),
                (["ctest", "--test-dir", "build-host/gerrit", "--output-on-failure",
                  "--no-tests=error"], None),
            ],
        )

    def thread_sanitizer_job(self) -> bool:
        """Run TSan in a non-root namespace with a private loopback interface."""

        command = (
            "ip link set lo up && "
            "exec bash xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh tsan"
        )
        return self.run_steps(
            "thread-sanitizer",
            [
                (
                    [
                        "unshare", "--user", "--map-root-user", "--net", "--pid",
                        "--fork", "--mount-proc", "sh", "-c", command,
                    ],
                    None,
                ),
            ],
        )

    def quality_jobs(self) -> list[tuple[str, Callable[[], bool]]]:
        """Return the unchanged complete host-quality job matrix."""

        return [
            ("gerrit-host", self.current_gerrit_job),
            ("gcc Debug", lambda: self.compiler_job("gcc", "Debug")),
            ("gcc Release", lambda: self.compiler_job("gcc", "Release")),
            ("clang Debug", lambda: self.compiler_job("clang", "Debug")),
            ("clang Release", lambda: self.compiler_job("clang", "Release")),
            ("sanitizers", lambda: self.run_steps(
                "sanitizers",
                [(["bash", "xWalkTool/shell-agent/quality-tool/run-host-sanitizer.sh", "asan"], None)],
            )),
            ("thread-sanitizer", self.thread_sanitizer_job),
            ("stress-tests", lambda: self.preset_job(
                "stress-tests", "sanity", "host-stress")),
            ("static-analysis", self.static_analysis_job),
            ("coverage", self.coverage_job),
            ("deployment-scripts", self.deployment_scripts_job),
            ("staged-install", self.staged_install_job),
        ]

    @staticmethod
    def worker_count(job_count: int) -> int:
        """Return the bounded configured parallel quality-job count."""

        try:
            return max(1, min(job_count, int(os.environ.get("XWALK_CI_MAX_WORKERS", "4"))))
        except ValueError:
            return min(job_count, 4)

    def execute_jobs(
        self, jobs: list[tuple[str, Callable[[], bool]]],
    ) -> dict[str, bool]:
        """Execute independent jobs concurrently and retain every result."""

        results: dict[str, bool] = {}
        with ThreadPoolExecutor(
            max_workers=self.worker_count(len(jobs)), thread_name_prefix="xwalk-ci"
        ) as executor:
            pending = {executor.submit(job): name for name, job in jobs}
            for future in as_completed(pending):
                name = pending[future]
                try:
                    results[name] = future.result()
                except Exception as error:  # pragma: no cover - defensive worker boundary
                    self.log.write(f"\n[FAILED] {name}: {error}\n")
                    self.log.flush()
                    results[name] = False
        return {name: results.get(name, False) for name, unused_job in jobs}

    def write_summary(self, results: dict[str, bool]) -> None:
        """Write the quality summary in deterministic job order."""

        self.log.write("\n======================== QUALITY SUMMARY ========================\n")
        for name, passed in results.items():
            result_text = "PASSED" if passed else "FAILED"
            self.log.write(f"[{result_text}] {name}\n")
        self.log.flush()

    @staticmethod
    def partition_jobs(
        jobs: list[tuple[str, Callable[[], bool]]],
    ) -> tuple[list[tuple[str, Callable[[], bool]]], list[tuple[str, Callable[[], bool]]]]:
        """Separate TSan from jobs whose parallel builds can starve its runtime."""

        isolated = [job for job in jobs if job[0] == "thread-sanitizer"]
        parallel = [job for job in jobs if job[0] != "thread-sanitizer"]
        return isolated, parallel

    def run_all(self) -> dict[str, bool]:
        """Run all isolated quality jobs with bounded host concurrency."""

        jobs = self.quality_jobs()
        isolated, parallel = self.partition_jobs(jobs)
        results = self.execute_jobs(isolated)
        results.update(self.execute_jobs(parallel))
        ordered_results = {name: results.get(name, False) for name, unused_job in jobs}
        self.write_summary(ordered_results)
        return ordered_results
