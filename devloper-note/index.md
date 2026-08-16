# xWalk Developer Notes

This wiki collects architecture, development, deployment, Raspberry Pi, PiCar-X, Python simulator, Gerrit, and
continuous-integration documentation for the xWalk workspace.

[Open the published GitHub Pages wiki](https://jochuuu.github.io/xWalkPiCarAI/)

## Documentation collections

- [xWalk C++ architecture and deployment](xwalk-rpi5-note/index.md)
- [xWalk Python 3 simulator and PiCar-X backend](xwalk-rpi5-py3-note/index.md)
- [Gerrit administration and continuous integration](gerrit-note/index.md)

## Access profiles

Run these commands from the repository root:

```bash
xWalkTool/doc-tool/wiki.sh local
```

```bash
xWalkTool/doc-tool/wiki.sh server --site-url https://docs.example.edu/xwalk/
```

```bash
xWalkTool/doc-tool/wiki.sh github
```

Use `local` on a workstation, `server` on the college host, and `github` for the Pages artifact. See the
[wiki tool guide](README.md) for hosting requirements, commands, and publication boundaries.
