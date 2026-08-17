# xWalk Developer Notes Wiki

`xWalkTool/doc-tool/wiki.sh` builds these Markdown collections as a searchable Material for MkDocs website. It
provides local-development, college-server, and GitHub Pages profiles.

Run the examples from the repository root. Generated dependencies and HTML remain under the ignored
`build-devloper-note-wiki` directory.

## Local profile

Build the wiki, bind it only to the current computer, and open the default browser:

```bash
xWalkTool/doc-tool/wiki.sh local
```

The default URL is `http://127.0.0.1:8000/`. Press Ctrl-C to stop the local server.

Serve locally without opening a browser:

```bash
xWalkTool/doc-tool/wiki.sh local --no-open
```

Choose another loopback port:

```bash
xWalkTool/doc-tool/wiki.sh local --port 8001
```

The local profile rejects non-loopback bind addresses.

## College-server profile

Run this command on the college server:

```bash
xWalkTool/doc-tool/wiki.sh server
```

It builds production-style static HTML under `build-devloper-note-wiki/server-site`, binds to
`0.0.0.0:8080`, and serves until Ctrl-C is pressed.

When the college provides a public HTTPS address, include it in the build:

```bash
xWalkTool/doc-tool/wiki.sh server --site-url https://docs.example.edu/xwalk/
```

Use another approved port when required:

```bash
xWalkTool/doc-tool/wiki.sh server --port 9000 --site-url https://docs.example.edu/xwalk/
```

Binding to `0.0.0.0` listens on every server interface. Worldwide access additionally requires:

- a public college DNS name or public IP address;
- firewall and NAT rules that permit the selected port;
- a TLS certificate and HTTPS reverse proxy, normally Nginx or Apache;
- college authorization for public hosting; and
- a persistent service manager if the wiki must survive logout or reboot.

The built-in Python static server is appropriate for initial verification. Use the college's managed HTTPS web
server for production access and point its document root at the generated `server-site` content.

## GitHub Pages profile

Build a GitHub Pages-compatible artifact locally:

```bash
xWalkTool/doc-tool/wiki.sh github
```

The default public URL is:

```text
https://jochuuu.github.io/xWalkPiCarAI/
```

Open the currently deployed public URL after building:

```bash
xWalkTool/doc-tool/wiki.sh github --open
```

Override the URL when the repository or custom domain changes:

```bash
xWalkTool/doc-tool/wiki.sh github --site-url https://docs.example.org/xwalk/
```

The `github` profile only creates `build-devloper-note-wiki/github-site`; it never pushes. The
`developer-note-pages.yml` workflow publishes that artifact after an approved Gerrit integration commit is
synchronized to `jochuuu/xWalkPiCarAI/master`. In GitHub, enable **Settings → Pages → Source: GitHub Actions**
once before the first deployment.

The workflow validates the committed integration metadata without initializing private Gerrit submodules. The
developer-note sources are already part of the exact submitted integration commit synchronized to GitHub, so
the Pages build requires no Gerrit SSH key, server address, or account variables.

## CI verification

Validate wiki-owned local links, strictly build the GitHub Pages artifact, and validate its homepage, sitemap,
search-index JSON, and public URL:

```bash
xWalkTool/doc-tool/wiki.sh verify
```

This non-serving command is used by Gerrit component CI, integrated Host Quality, and GitHub Host Quality. It
does not open a browser, bind a port, or publish the generated artifact.

## Dependency setup

Every profile creates an isolated Python environment when needed. Prepare it without starting a site:

```bash
xWalkTool/doc-tool/wiki.sh setup
```

The package pin is in `xWalkTool/doc-tool/requirements-wiki.txt`. First-time installation requires Python 3,
the Python `venv` module, and package-index access.

## Remove generated files

```bash
xWalkTool/doc-tool/wiki.sh clean
```

This removes only the validated `build-devloper-note-wiki` directory. It does not remove source notes.

## Build behavior

- `mkdocs.yml` enables Material navigation, tables, heading anchors, and local full-text search.
- Every profile stages a fresh copy of the Markdown source before building or serving it.
- Links to workspace files outside `devloper-note` are converted in the staged artifact to GitHub source links
  for the exact deployed revision. Source Markdown retains its checkout-relative links.
- Every build cleans stale HTML from its profile-specific site directory.
- Generated environments and HTML must not be committed.
