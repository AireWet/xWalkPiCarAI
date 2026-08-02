# `XWalkConfigStore` and `XWalkConfig`

`XWalkConfigStore` provides flat key-value persistence. `XWalkConfig` provides
section-aware configuration persistence. Both are defined by the
[`xWalkConfig`](../../../xWalkHal/xWalkConfig/README.md) module.

Configuration objects own their paths and in-memory values. Filesystem access
passes through common file wrappers. Deployment owns directory creation,
permissions, and recovery policy.

Applications should load and validate configuration before constructing motor,
servo, or robot components. Hardware classes do not open configuration files.
