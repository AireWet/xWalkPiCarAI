# Runtime logs

Run xWalk applications from the repository root to place generated diagnostics
in this directory. `XWalkTrace` appends accepted records to `xWalkTrace.log`.
Records contain a UTC timestamp, monotonic elapsed time, component, category or
priority, optional UID, sanitized source location, and formatted message.

Generated `*.log` files are ignored by Git.
