# Configure I2S for the speaker

I2S and ALSA configuration belongs to target deployment, not to xWalk HAL
production code. Configure the operating-system audio overlay and routing using
the deployment procedure approved for the target image.

## Installation-screen image

TODO: Add a C++ target-deployment screenshot showing I2S configuration.

## Verification-screen image

TODO: Add a C++ target-deployment screenshot showing audio-device verification.

## Restart-screen image

TODO: Add a C++ target-deployment screenshot showing the approved restart step.

The application creates the audio backend, enables speaker power through
`XWalkBoardControl`, and injects playback or speech callbacks. HAL components do
not run privileged commands, change system files, or install packages.
