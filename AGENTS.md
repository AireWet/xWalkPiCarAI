# xWalk repository instructions

## Required project knowledge

Before analyzing, editing, reviewing, or generating code in this repository,
read and follow both knowledge-base documents:

- [`.agents/gudlines/CODING_GUIDELINES.md`](.agents/gudlines/CODING_GUIDELINES.md)
- [`.agents/gudlines/DOCUMENTATION_GUIDELINES.md`](.agents/gudlines/DOCUMENTATION_GUIDELINES.md)

Treat these files as the coding, architecture, and documentation knowledge base
for the complete `MyPiCarX` workspace, including `xWalk-rpi5-hw/xWalkLibrary/common`, `xWalkHal`,
`xWalkAgent`, and `xWalkController`.

Apply the guide to every future implementation. Preserve intentional existing
architecture, naming, dependency boundaries, validation behavior, test safety,
and CMake patterns unless the user explicitly requests a change.

After an implementation, compare the result with the guide. If the work
deliberately introduces or changes a reusable project-wide convention, update
the guide in the same change. Do not update it for a local exception, generated
output, accidental inconsistency, or unapproved redesign.

## Mandatory C++ formatting

Apply these rules to `.cpp`, `.cc`, `.cxx`, `.h`, `.hpp`, and `.hxx` files:

- use four spaces for every indentation level and never use tabs;
- indent nested namespaces, types, functions, methods, conditions, loops,
  switches, and all other blocks;
- indent `public:`, `protected:`, and `private:` inside their class, then indent
  declarations four additional spaces below the access specifier;
- use Allman braces, with every opening brace on the next line aligned with its
  block declaration and every closing brace aligned with its opening brace;
- always use braces for control-flow and other blocks, including a block whose
  body contains only one statement; and
- keep every C++ source or header line at or below 120 characters, wrapping
  declarations, calls, conditions, expressions, strings, and comments with
  continued indentation when necessary.

Use the repository-root `.clang-format` for mechanical formatting. Do not
format generated sources under an `auto-gen` or `generated` directory, and do
not reformat vendored or third-party code, including dependency-prefix headers
below `xWalkLibrary/x86_64` and `xWalkLibrary/aarch64`.

Before completing any C++ change, format all project-owned C++ files with:

```bash
xWalk-rpi5-tool/py-agent/dev-tool/styler-tool/xWalkStyler format
```

Before submitting or merging a change, validate formatting without modifying
the checkout:

```bash
xWalk-rpi5-tool/py-agent/dev-tool/styler-tool/xWalkStyler check
```

## Header-owned type definitions

Define every project class and structure in a header. Do not define a `class`
or `struct` in a `.cpp`, `.cc`, or `.cxx` file. Source files may contain
member-function implementations and ordinary object declarations such as
`struct stat pathStatus`, but they must not own a type definition.

Place public and reusable types in the owning component's `include` directory.
For an implementation-private type, use a narrowly scoped companion type
header with a named namespace; do not use an anonymous namespace in a header.
The repository-wide test-support layout below takes precedence for test fake
state, fixtures, mappings, callbacks, and factories.

## Repository-wide test support layout

For tests in `xWalkHal`, `xWalkAgent`, and `xWalkController`, move reusable
callback state, fake-backend structures, mapping records, callback declarations,
and callback-table factories into a dedicated `<Component>TestSupport.h` under
the owning test `include` directory. Put non-trivial implementations in the
matching `<Component>TestSupport.cpp` under the test `src` directory, and list
that source explicitly in every standalone or aggregate target that uses it.

Use a named, component-specific test namespace such as
`xwalk::hal::test::gpio`; never declare an anonymous namespace in a header,
because it creates different entities in every translation unit. Apply this
layout whenever test code is added or modified anywhere in the repository.

## Markdown command formatting

In `.md` files only, keep every fenced shell-command example on one physical
line. Do not use continuation backslashes to wrap CMake, build, test, Python,
or other shell commands. A complete shell-command line may exceed the normal
115-character documentation limit.

## Verification safety

Prefer host tests. Hardware tests are opt-in and must not be run unless the user
explicitly requests them and confirms that the correct Raspberry Pi and Robot
HAT setup is connected and safe.

## Gerrit-authoritative, submit-gated publication

Gerrit is the authoritative development and review system. Commit component
changes in their independent repository and upload them only to Gerrit
`master` with:

```bash
git push origin HEAD:refs/for/master
```

An active upload triggers Gerrit CI automatically. To defer CI, upload the
change as WIP:

```bash
git push origin HEAD:refs/for/master%wip
```

For a WIP change, Gerrit's **Mark As Active** button is the Activate action.
Clearing WIP through that button triggers CI for the current patch set. Moving
an active change into WIP must not trigger CI.

GitHub repositories under `AireWet` are read-only, submit-gated mirrors.
Pending patch sets, review refs, drafts, edits, user refs, and temporary refs
must never be synchronized. Only an exact commit on a submitted Gerrit branch
may be replicated to the matching GitHub branch. Developers must not push a
component or integration branch directly to GitHub, and GitHub pull requests
must not replace Gerrit review.

An optional local `github` remote is fetch-only. Its push URL must remain the
non-routable policy URL installed by `xWalk-git-env.sh`; `origin` remains
Gerrit. Never force-push, rewrite published history, delete a mirrored branch,
or use a wildcard, `--mirror`, `--all`, or working-tree publication push.

The integration repository pins exact submitted component commits. Every
submodule-pointer change requires its own Gerrit review, complete CI, approval,
and Submit before Gerrit replication can publish it to
`https://github.com/AireWet/xWalkPiCarAI`.
