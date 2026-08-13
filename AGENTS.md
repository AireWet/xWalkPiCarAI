# xWalk repository instructions

## Required project knowledge

Before analyzing, editing, reviewing, or generating code in this repository,
read and follow both knowledge-base documents:

- [`.agents/gudlines/CODING_GUIDELINES.md`](.agents/gudlines/CODING_GUIDELINES.md)
- [`.agents/gudlines/DOCUMENTATION_GUIDELINES.md`](.agents/gudlines/DOCUMENTATION_GUIDELINES.md)

Treat these files as the coding, architecture, and documentation knowledge base
for the complete `MyPiCarX` workspace, including `xWalkLibrary/common`, `xWalkHal`,
`xWalkAgent`, and `xWalkController`.

Apply the guide to every future implementation. Preserve intentional existing
architecture, naming, dependency boundaries, validation behavior, test safety,
and CMake patterns unless the user explicitly requests a change.

After an implementation, compare the result with the guide. If the work
deliberately introduces or changes a reusable project-wide convention, update
the guide in the same change. Do not update it for a local exception, generated
output, accidental inconsistency, or unapproved redesign.

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

## Gerrit-only publication

Never push repository changes directly to GitHub, including through the
`origin` or `github-actions` remotes. Commit changes locally, then upload them
only to Gerrit for review with:

```bash
git push gerrit HEAD:refs/for/master
```

An active upload triggers Gerrit CI automatically. To defer CI, upload the
change as WIP:

```bash
git push gerrit HEAD:refs/for/master%wip
```

For a WIP change, Gerrit's **Mark As Active** button is the Activate action.
Clearing WIP through that button triggers CI for the current patch set. Moving
an active change into WIP must not trigger CI.

Only Joxy (`joxjoh24@student.hh.se`) may merge into GitHub `master`. After
Gerrit's **Submit** action, the CI service may mirror a directly applicable
Joxy-owned change to GitHub `master`. A submitted change owned by anyone else,
or a change stacked on GitHub work still awaiting review, must be pushed to the
dedicated GitHub review branch and presented to Joxy as a pull request. Do not
use a direct GitHub push as a preliminary, backup, or alternate publication
path.
