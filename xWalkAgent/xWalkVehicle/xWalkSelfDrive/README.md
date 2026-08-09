# xWalkSelfDrive

`xWalkSelfDrive` is a C++17 Agent coordinator for the named
gesture, short movement, horn, engine-start, status, queue, and worker behavior while keeping platform and
hardware ownership outside the module.

## Behavior

- accepts the exact upstream movement names: `shake head`, `nod`, `wave hands`, `resist`, `act cute`,
  `rub hands`, `think`, `twist body`, `celebrate`, `depressed`, `forward`, `backward`, and `stop`;
- accepts the upstream sound names `honking` and `start engine`;
- preserves servo angles, motor commands, delay intervals, action names, and sound volumes;
- provides synchronous `doAction()` plus explicit standby, think, queued-actions, start, stop, and wait
  flow;
- injects timing and stores non-owning PiCar-X and Music pointers instead of owning hardware or audio
  resources;
- rejects unknown queued actions without executing or storing them;
- optionally checks a controlling-thread cancellation callback during delays in slices no longer than 20
  milliseconds; and
- reports a worker delay failure as a Boolean completion status and lets ordinary synchronous failures escape
  after command-scope actuator cleanup has run.

Standby is an explicit idle state. It does not select random intervals, enqueue actions, play sounds, or move
an actuator. The unused upstream completed-actions state was removed; queue completion transitions directly
to standby and wakes waiting callers.

Voice-requested `stop` is queued through the worker with every other action. It therefore waits for an active
thinking pose to finish before touching the PiCar-X motors and cannot race the worker's actuator writes.

## Sound resources

The preset sound actions resolve `car-double-horn.wav` and `car-start-engine.wav` below the constructor's
sound directory. The root-level [`xWalkAudioResources`](../../../xWalkAudioResources) directory supplies
development assets and
provenance; installed composition uses `/usr/share/xwalk/sounds` by default. A missing or unreadable regular
file rejects its action before playback or movement.

## Safety

Every gesture can move steering, camera servos, or drive motors. Configure the cancellation callback before
starting movement. Cancellation latches the shared PiCar-X emergency stop, attempts both motors independently,
and suppresses later actuator commands in the current action. The application command boundary owns the
non-throwing safety guard that performs the same cleanup during normal return and stack cleanup. The worker
delay callback returns `false` on failure; this performs emergency shutdown and makes `waitActionsDone()`
return `false` to the controlling thread. No exception-handling statement is required. Call direct action
methods and queue-control methods from one controlling context.

## Layout

| Path | Responsibility |
| --- | --- |
| `include/xAgent_Rpi5CarSelfDrive.h` | Preset-action, queue, worker, and dependency contract |
| `include/xAgent_Rpi5CarSelfDriveTypes.h` | Status enumeration and injected timing callback |
| `src/*Actions.cpp` | Exact action-name dispatch, movement actions, and sound actions |
| `src/*Gestures.cpp` | Ordered steering, camera, and motor gesture sequences |
| `src/*Lifecycle.cpp` | Dependency validation and worker ownership |
| `src/*Worker.cpp` | Status transitions and first-in, first-out action processing |
| `test/src/*Test.cpp` | In-memory gesture, sound, validation, and queue tests |

Use `XWALK_SELF_DRIVE_BUILD_HOST_TESTS=ON` for standalone host verification. Use
`XWALK_SELF_DRIVE_BUILD_HARDWARE_TESTS=ON` only to compile the module and its Linux/RPi dependencies; the
module does not register a physical preset-action test because automatically running its gesture catalog would
be unsafe.
