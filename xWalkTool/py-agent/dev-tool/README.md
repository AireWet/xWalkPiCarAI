# xWalk Developer Tools

This directory contains executable Python utilities used for xWalk development,
dependency preparation, interface generation, and licence configuration.

Run all commands from the MyPiCarX repository root:

```sh
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarDependencyInstaller --check
xWalkTool/py-agent/dev-tool/xHal_Rpi5CarIwGenerator --help
xWalkTool/py-agent/dev-tool/xWalkLicenseTool --help
python3 xWalkTool/py-agent/dev-tool/xWalkZuulValidator .zuul.yaml
python3 xWalkTool/py-agent/dev-tool/test/test_xWalkLicenseTool.py
```

The dependency installer's Raspberry Pi apply modes can modify the host and
must be used only on an explicitly approved device. The interface generator,
licence tool, and all repository tests remain host-safe. The `test` directory
contains the licence tool's host-only test suite.
