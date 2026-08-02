# Onboard MCU

The Robot HAT uses an onboard microcontroller for ADC and PWM operations. xWalk
communicates through `XWalkI2c`; the normal 7-bit MCU address is `0x14`.

## ADC protocol

- Channels: 0 through 7.
- Command byte: `0x10`.
- Response: two bytes assembled as one unsigned 12-bit sample.
- Valid raw range: 0 through 4095.
- Voltage reference: 3.3 volts.

`XWalkAdc` validates the channel and requires exactly two response bytes.

## PWM protocol

- Output registers begin at `0x20` for channels 0 through 19.
- Prescaler registers begin at `0x40` and `0x50`.
- Period registers begin at `0x44` and `0x54`.
- The PWM clock constant is 72 megahertz.

`XWalkPwmTimerState` records the shared timer configuration. `XWalkPwm` rejects
incompatible frequency changes instead of silently changing sibling channels.

## PWM timer mapping

| Timer | Channels |
|---|---|
| 0 | 0, 1, 2, 3 |
| 1 | 4, 5, 6, 7 |
| 2 | 8, 9, 10, 11 |
| 3 | 12, 13, 14, 15 |
| 4 | 16, 17 |
| 5 | 18 |
| 6 | 19 |

## MCU reset

`XWalkBoardControl::resetMcu()` drives Raspberry Pi GPIO5 low for 10
milliseconds and restores it high. Stop active outputs and serialize I2C access
before resetting the MCU.
