# Community C++ adaptations

External tutorials can target different Robot HAT revisions, operating systems,
or APIs. Before adapting one to xWalk Firmware:

1. Replace implicit global hardware with objects created in `main()`.
2. Replace platform calls with injected backends or callbacks.
3. Validate pin mappings and board revision using current xWalk constants.
4. Apply the ownership, range, timing, and safety contracts from public headers.
5. Add host tests before enabling physical outputs.

TODO: Add reviewed C++ community references after their code and hardware
assumptions have been verified against the current xWalk HAL.
