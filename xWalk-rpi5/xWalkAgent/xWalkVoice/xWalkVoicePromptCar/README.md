# xWalkVoicePromptCar

`xWalkVoicePromptCar` ports upstream `example/14.voice_promt_car.py` through
caller-owned `XWalkPicarx` and `XWalkTextToSpeech` objects. It speaks the source
greeting, then announces and performs forward, backward, left, and right
movements at 30-percent requested speed for two seconds each.

The left and right movements preserve the source's minus/plus 20-degree
steering. Normal completion, cancellation between movements, and exceptions
all stop the motors and restore centred steering. The Agent owns no physical
motor, speaker, ALSA endpoint, Espeak process, or application callback.
