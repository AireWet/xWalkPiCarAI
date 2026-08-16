# C++ speech interfaces

`XWalkTextToSpeech` coordinates text synthesis through an injected backend.
`XWalkSpeechToText` coordinates recognition through an injected backend. Both
classes belong to [xWalkGPT](../../../../xWalk-rpi5/xWalkHal/layer1/xWalkGPT/README.md).

The C++ core owns no model, microphone, audio device, network client, process,
credential, or generated media file. The application supplies and owns those
facilities and keeps callback contexts valid for the object lifetime.

The Linux hardware layer supplies two offline providers. `XWalkSpeechRecognizerVosk`
loads `libvosk.so` and one configured model at runtime, while
`XWalkTextToSpeechEspeak` executes Espeak without a shell and converts its WAV
output to signed sixteen-bit PCM. ALSA adapters perform microphone capture and
shared speaker playback. `xWalkBootRpi` retains Robot HAT speaker power for the
complete synthesis and playback lifetime.
