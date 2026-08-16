# C++ utility interfaces

The [xWalkUtils](../../../../xWalk-rpi5/xWalkHal/interface/xWalkUtils/README.md) module contains:

- `XWalkUtils` for injected platform utility and output operations;
- `XWalkLazyReader<ValueType>` for bounded-rate callback value caching;
- `XWalkStderrGuard` for scope-bound stderr suppression and restoration.

Generic utilities do not create hardware or platform services. Callers provide
callbacks and contexts explicitly. Common non-member functions remain in the
`xwalk::hal::common` namespace under `xWalkLibraryCommon`.
