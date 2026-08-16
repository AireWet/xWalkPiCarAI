# Documentation tool

`doc-tool` owns the executable tooling for the developer-note wiki. Documentation content and navigation remain
under `devloper-note`; generated environments and HTML remain under the ignored
`build-devloper-note-wiki` directory.

Run commands from the repository root:

```bash
xWalkTool/doc-tool/wiki.sh local
```

```bash
xWalkTool/doc-tool/wiki.sh server --site-url https://docs.example.edu/xwalk/
```

```bash
xWalkTool/doc-tool/wiki.sh github
```

```bash
xWalkTool/doc-tool/wiki.sh verify
```

The launcher provides `local`, `server`, and `github` access profiles plus `verify`, `setup`, and `clean`
maintenance operations. It never publishes directly or selects hardware tests.
