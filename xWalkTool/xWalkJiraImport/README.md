# xWalk Jira historical importer

## Purpose

This host-only Python tool reconstructs Jira backlog work items from meaningful commits on
`jochuuu/xWalkPiCarAI`. It supports thesis planning and traceability for YOLO, vulnerable road-user detection,
Raspberry Pi 5 integration, camera and hardware control, testing, deployment, and documentation work.

The tool reads commit history through the GitHub REST API. In explicit apply mode, it creates missing issues in
the Jira `TARS` project, retains their initial To Do status, and verifies that they appear in the configured board
backlog. It never modifies Git history, repository branches, tags, files, or remotes.

Commits are processed in branch-history order from oldest to newest. Jira therefore assigns `TARS-1` to the
earliest created import, `TARS-2` to the next created import, and so on when importing into an empty project.

Dry-run mode is the default. No Jira mutation method is enabled unless `--apply` is supplied and complete Jira
credentials are loaded from a safe credential source.

## Package layout

```text
xWalkJiraImport/
├── pyproject.toml
├── README.md
├── requirements.txt
├── xWalkJiraImport/
│   ├── __init__.py
│   ├── __main__.py
│   ├── xWalkJiraImportApplication.py
│   ├── xWalkJiraImportCommitAnalyser.py
│   ├── xWalkJiraImportConfig.py
│   ├── xWalkJiraImportCredentials.py
│   ├── xWalkJiraImportDateCalculator.py
│   ├── xWalkJiraImportEstimator.py
│   ├── xWalkJiraImportGitHubClient.py
│   ├── xWalkJiraImportJiraClient.py
│   ├── xWalkJiraImportModels.py
│   └── xWalkJiraImportReport.py
└── test/
```

The directory, distribution, import package, console command, source modules, and test files use the repository's
`xWalk<Module>` naming convention. The standard Python packaging files retain their conventional names.
The installable package lives directly under `xWalkTool/xWalkJiraImport`; the `xWalkTool/python` directory remains
reserved for standalone repository Python executables and their shared host tests.

## Limitations

- One accepted Git commit becomes at most one Jira work item. Broad or unrelated commits require manual review.
- Generated files, binaries, model weights, raw datasets, lock files, build output, and third-party files do not
  increase implementation effort.
- Jira Cloud assigns the real issue creation, update, resolution, and workflow-history timestamps at import time.
  The tool does not overwrite protected system timestamps.
- Historical start dates are estimates based on 09:00-17:00 weekdays in `Europe/Stockholm`.
- GitHub patches can be unavailable for binary or very large files; affected estimates use lower confidence.
- A Story is requested only for clear user-facing capability commits and falls back to Task when TARS does not
  expose Story as a creatable issue type.
- Board membership is verified against board 3's existing saved filter after creation. The tool never changes the
  board filter, column mappings, backlog, or workflow.
- Dry runs without Jira credentials can analyse GitHub history but cannot discover TARS fields, duplicates,
  permissions, workflow transitions, or the board filter.

## Installation

Create and activate the repository's Python environment, then install the package and its dependency:

```sh
python3 -m venv xWalkTool/.xWalkPyEnv
source xWalkTool/.xWalkPyEnv/bin/activate
python -m pip install ./xWalkTool/xWalkJiraImport
```

Use an editable installation while developing the importer:

```sh
python -m pip install --editable ./xWalkTool/xWalkJiraImport
```

The package installs the `xWalkJiraImport` console command. The equivalent module form is
`python -m xWalkJiraImport`. `requirements.txt` remains available for environments that manage the runtime
dependency separately, while `pyproject.toml` is the authoritative package metadata.

The root `.gitignore` excludes `xWalkTool/.xWalkPyEnv`, Python package metadata, build reports, Python caches,
`.netrc`, `*.netrc`, `.env` files, and common secret-file patterns.

## Local credentials with `.netrc`

The importer automatically loads Python's standard per-user credential file at `Path.home() / ".netrc"` on every
supported operating system. Users do not need to pass `--netrc-file` during normal execution. The filename is
`.netrc`, not `.netric`. Each developer creates and protects their own file outside the repository:

```text
machine api.github.com
    login jochuuu
    password YOUR_GITHUB_TOKEN

machine student-team-xwalk-rpi5.atlassian.net
    login YOUR_ATLASSIAN_EMAIL
    password YOUR_JIRA_API_TOKEN
```

Protect it before running the importer:

```sh
chmod 600 ~/.netrc
```

Never commit, upload, paste, or copy `.netrc` into this repository. The importer reads only the entries matching
`api.github.com` and the hostname derived from the selected Jira URL. It never prints the file contents, token
values, full Jira login, Basic authorization value, or complete request headers.

The GitHub token is optional because `jochuuu/xWalkPiCarAI` is public. Without it, the importer continues with the
lower unauthenticated GitHub API rate limit. Jira login and API token are required for `--apply` and
authenticated Jira discovery. Use the Atlassian account email as `login` and an API token as `password`; never
store the Atlassian account password there.

`--netrc-file` remains an optional override for CI or an exceptional protected location:

```sh
xWalkJiraImport --netrc-file /secure/path/xwalk.netrc --dry-run
```

On POSIX systems, group or other read/write permissions stop apply mode. Dry-run mode warns and disables all
authenticated requests until the permissions are corrected. Windows skips the POSIX permission-bit check.

## Non-secret environment variables

Non-secret project defaults match the thesis services, but every value can be supplied through the documented
environment variables:

```sh
export JIRA_URL="https://student-team-xwalk-rpi5.atlassian.net"
export JIRA_PROJECT_KEY="TARS"
export GITHUB_REPOSITORY="jochuuu/xWalkPiCarAI"
export GITHUB_BRANCH="master"
```

The equivalent command-line options `--jira-url`, `--jira-project-key`, `--github-repository`, and `--branch`
override these environment values.

## Command-line options

`--dry-run` and `--apply` are mutually exclusive. Omitting both selects dry-run mode. Command-line project
settings override non-secret environment variables; the listed built-in defaults apply when neither source is
provided.

| Option | Default | Description |
| --- | --- | --- |
| `-h`, `--help` | Not applicable | Show command usage and exit without running an import. |
| `--dry-run` | Selected implicitly | Analyse commits and write reports without creating or changing Jira items. |
| `--apply` | Disabled | Create missing Jira items in the board backlog with To Do status. |
| `--since YYYY-MM-DD` | No lower limit | Include commits on or after this ISO calendar date. |
| `--until YYYY-MM-DD` | No upper limit | Include commits on or before this ISO calendar date. |
| `--author NAME_OR_EMAIL` | All authors | Match author name, email, or GitHub login case-insensitively. |
| `--branch BRANCH` | `GITHUB_BRANCH` or `master` | Read commits from this GitHub branch. |
| `--jira-url URL` | `JIRA_URL` or thesis Jira URL | Select the HTTPS Jira site and its netrc machine. |
| `--jira-project-key KEY` | `JIRA_PROJECT_KEY` or `TARS` | Select the Jira project for searches and creation. |
| `--github-repository OWNER/REPOSITORY` | Environment or thesis repository | Select the GitHub repository. |
| `--netrc-file PATH` | `~/.netrc` | Use a protected credential file at an exceptional custom location. |
| `--max-commits NUMBER` | Unlimited | Select at most this many oldest matching commits; must be positive. |
| `--include-merges` | Disabled | Include merge commits, which are otherwise excluded. |
| `--include-insignificant` | Disabled | Include formatting-only and insignificant typo-only commits. |
| `--apply-manual-review` | Disabled | Create broad low-confidence commits; requires `--apply`. |
| `--update-existing-estimates` | Disabled | Update Original estimate on exact-SHA imports; requires `--apply`. |
| `--update-existing-planning-after ISSUE_KEY` | Disabled | Sequence imports after an anchor; apply only. |
| `--update-existing-planning-start YYYY-MM-DD` | Disabled | Sequence imports from an explicit date; apply only. |
| `--update-existing-sprints` | Disabled | Assign imports to board sprints by Due date; apply only. |
| `--update-existing-epic ISSUE_KEY` | Disabled | Attach imports to a validated Epic parent; apply only. |
| `--output-report PATH` | `build/jira-import-preview` | Set the JSON and CSV report base path. |
| `--verbose` | Disabled | Print additional progress diagnostics without credentials or authorization headers. |

The date range is inclusive, and `--since` must not be later than `--until`. Supplying `PATH`, `PATH.json`, or
`PATH.csv` to `--output-report` still creates both report formats using the resolved base path.

## CI credential fallback

CI environments that cannot mount a protected netrc file may use the compatibility fallback variables:

```sh
export GITHUB_TOKEN="<optional GitHub API token>"
export JIRA_EMAIL="<Atlassian account email>"
export JIRA_API_TOKEN="<Atlassian API token>"
```

Safe netrc values take precedence over matching fallback variables. A complete netrc therefore reports only:

```text
Credential source: netrc
```

Never place real values in this repository, tracked shell scripts, command arguments, reports, screenshots, or
support messages. Environment secrets are intended for CI compatibility, not normal local execution.

## Create and protect an Atlassian API token

1. Sign in to the Atlassian account that has Browse, Create Issues, and Transition Issues permissions for TARS.
2. Open [Atlassian API token management](https://id.atlassian.com/manage-profile/security/api-tokens).
3. Create a dedicated token with the narrowest suitable lifetime and scope available for the account.
4. Copy it once into a protected password manager and the mode-`0600` `.netrc` entry.
5. Replace an expired token in the password manager and `.netrc`; an HTTP 401 reports an invalid, expired, or
   revoked token without echoing it.
6. Revoke unused or compromised tokens from the same Atlassian token-management page and remove the old entry.

The direct `student-team-xwalk-rpi5.atlassian.net` implementation expects a regular Atlassian API token. A scoped
Atlassian token may require the `https://api.atlassian.com/ex/jira/{cloudId}` gateway, which this importer does not
implement. Use a regular token for the direct Jira-site URL unless gateway support is added separately.

The importer does not print credential values, request authorization headers, or remote response bodies.

## Test authentication safely

Test the GitHub entry without displaying its token:

```sh
curl --netrc --header "Accept: application/vnd.github+json" "https://api.github.com/repos/jochuuu/xWalkPiCarAI/commits?per_page=1"
```

Test the Jira entry without displaying its API token:

```sh
curl --netrc --header "Accept: application/json" "https://student-team-xwalk-rpi5.atlassian.net/rest/api/3/myself"
```

An HTTP 401 normally means the token is invalid, expired, or revoked. An HTTP 403 normally means the authenticated
account lacks permission. Revoke and replace questionable tokens rather than sharing them for diagnosis.

## Dry-run preview

Dry-run is implicit:

```sh
xWalkJiraImport --max-commits 20 --output-report build/jira-import-preview
```

The explicit form is useful in review records:

```sh
xWalkJiraImport --since 2026-01-01 --until 2026-08-07 --dry-run --output-report build/jira-import-preview
```

This preview may run without Jira credentials. In that case, Jira metadata and duplicate checks are skipped and
reported as limitations. A missing GitHub token also remains non-fatal because the repository is public.

Filter by Git author name, email, or GitHub login:

```sh
xWalkJiraImport --author "Joxy John" --branch master --dry-run --output-report build/jira-import-author-preview
```

Merge commits are excluded by default. Use `--include-merges` only when merge commits represent independent work.
Formatting-only and insignificant typo-only commits are excluded unless `--include-insignificant` is supplied.

## Review before apply

Review both generated reports. Confirm at least the following for every accepted record:

- component, summary, and Bug, Task, or Story classification;
- important source and test files represented by the diff;
- ignored generated and third-party files;
- estimated time, story points, confidence, and historical dates;
- manual-review records, especially broad cross-module commits;
- duplicate results and any Jira field or board limitations.

The JSON report contains final counters, discovered field names, limitations, and full records. The CSV report
contains the same per-commit fields for spreadsheet review. Supplying `PATH`, `PATH.json`, or `PATH.csv` always
produces both `PATH.json` and `PATH.csv`.

## Human development estimates

Effort values represent the expected work of a human developer, not AI generation speed or the elapsed time of an
automated editing tool. The estimator considers repository and requirements review, investigation or design,
implementation, hardware or cross-module integration, local verification, regression testing, code review, and
normal correction work. Changed-line count is supporting evidence only.

Tiny documentation or configuration corrections can remain Trivial. Semantic source changes include normal human
engineering overhead, Bugs include investigation, user-facing Stories include design and validation, and complex
hardware, camera, computer-vision, or AI work receives an integration-risk adjustment. Tested implementation work
includes the effort to design, implement, and run the tests. Broad or ambiguous commits remain manual-review items
and receive a conservative 24, 40, or 80 man-hour scope-band estimate with Low confidence.

Apply mode normally skips those records. After reviewing them, an operator may explicitly retain their
low-confidence classification and create them with `--apply --apply-manual-review`. The override accepts the
scope-band planning value without presenting it as precise elapsed time.

Existing exact-SHA imports remain unchanged by default. After reviewing recalculated estimates, use
`--apply --update-existing-estimates` to update only Jira's time-tracking `Original estimate` field. Combine it
with `--apply-manual-review` when the selected history includes broad records.

Use `--apply --update-existing-planning-after ISSUE_KEY` to align existing exact-SHA imports with the established
short summary tags and schedule them after an anchor ticket. The anchor must have a Due date. The first selected
commit starts on the next weekday; each Due date rounds Original estimate up to whole eight-hour Jira workdays,
and the next selected commit starts on the following weekday. This prevents overlapping date-only schedules.
Use `--update-existing-planning-start YYYY-MM-DD` instead when the first selected import has an explicit Start
date. The explicit-date and issue-anchor options are mutually exclusive.

Use `--apply --update-existing-sprints` to select the configured board sprint whose date range contains each
existing import's Due date. The importer assigns the issue through Jira Software, then verifies both sprint and
board membership. Future-sprint issues are represented in Jira's sprint-planning backlog; completed issues remain
Done and may be hidden by Jira backlog views that exclude completed statuses.

Jira dashboards do not directly contain issues. Dashboard gadgets display issues through project filters,
assignees, sprint membership, and status. The importer verifies the configured board filter and sprint membership;
it does not rewrite dashboard layouts or reopen completed historical work to bypass backlog status filtering.

Use `--apply --update-existing-epic ISSUE_KEY` to attach exact-SHA existing imports to the same Epic hierarchy as
the earlier Jira history. The importer verifies that the requested parent is an Epic before any child mutation,
sets Jira's standard `parent` field, and confirms the saved relationship for every updated issue.

## Apply after approval

Apply mode is intentionally a separate command and requires Jira credentials:

```sh
xWalkJiraImport --since 2026-01-01 --until 2026-08-07 --apply --output-report build/jira-import-result
```

Apply mode performs these actions for each accepted non-manual commit, plus reviewed broad commits when
`--apply-manual-review` is supplied:

1. Search TARS for the exact full-SHA marker.
2. Skip an existing marker as `already imported`.
3. Create one issue using fields supported by the selected issue type.
4. Verify that the initial workflow status is To Do.
5. Confirm the issue matches board 3's saved filter and appears in its backlog.
6. Record the issue key, URL, final status, and result in JSON and CSV.

It edits an existing exact-SHA issue only when an explicit existing-item update option is supplied, and it never
deletes an issue. If creation succeeds but a later status, board, or backlog check fails, the report retains the
new Jira key. A repeat run finds the permanent full-SHA marker and does not create a duplicate.

## Expected Jira fields

The importer discovers issue types and create fields through Jira API v3. It does not hard-code custom-field IDs.
Core fields are project, issue type, summary, and an Atlassian Document Format description. When the selected issue
type supports them, it also sets:

- labels;
- time tracking `Original estimate`;
- `Story point estimate` or `Story points`;
- `Start date`;
- `Due date`;
- `Historical created date`;
- `Historical done date`.

The detailed ADF description uses headings, explanatory paragraphs, and bullet lists. Its sections cover the
overview, engineering context, completed work, human development estimate, validation and testing, historical
timeline, Git traceability, original commit message, and import provenance. It records the full SHA, clickable
GitHub URL, author, semantic file statistics, historical timestamps, effort evidence, confidence, test evidence,
import notice, and permanent marker:

```text
xwalk-git-import:<FULL_COMMIT_SHA>
```

## Duplicate prevention

The tool searches Jira enhanced JQL using the full SHA and then verifies the exact marker in the returned ADF
description. It never relies on a short SHA or generated summary. This makes preview and apply runs safely
repeatable even if a summary or classification changes later.

## Testing

All requests are mocked. Tests cannot contact GitHub or Jira and cannot create work items:

```sh
cd xWalkTool/xWalkJiraImport && python3 -m unittest discover -s test -p 'test_xWalkJiraImport*.py'
```

The suite covers netrc paths, parsing, precedence, permissions, redaction, session isolation, classification, Jira
types, generated-file exclusion, estimates, working dates, ADF, field discovery, duplicates, backlog verification,
pagination, retries, authentication failures, and dry-run mutation protection. Every credential fixture is a fake
temporary file; tests never read the developer's real `~/.netrc`.

## Troubleshooting

- Missing `.netrc`: create `~/.netrc`, protect it with mode `0600`, or configure the CI environment fallback.
- Malformed `.netrc`: check the exact `machine`, `login`, and `password` structure without sharing the file.
- Unsafe permissions: run `chmod 600 ~/.netrc`; the importer never changes permissions automatically.
- Missing Jira entry: ensure the machine name exactly matches the hostname in `JIRA_URL`.
- `HTTP 401`: replace an invalid, expired, or revoked token; never substitute an Atlassian account password.
- `HTTP 403`: confirm TARS Browse, Create Issues, and Transition Issues permissions and board visibility.
- `HTTP 404`: confirm the Jira URL, project key, board ID, repository identifier, and branch.
- GitHub rate limit or `HTTP 429`: wait for reset or configure the optional GitHub token; retries are bounded.
- No Story type: the importer safely falls back to Task.
- Missing estimates or date fields: ensure those fields are on the create screen for the selected issue type.
- Missing To Do backlog item: inspect the initial workflow status, board filter, and sprint assignment.
- Board mismatch: inspect board 3's saved filter and status-column mapping; the tool does not alter either.
- Manual review: split the historical work conceptually or approve a conservative Jira record by hand before
  importing; broad records use a Low-confidence man-hour scope band rather than a precise elapsed-time claim.
