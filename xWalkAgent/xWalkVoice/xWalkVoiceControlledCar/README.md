# xWalkVoiceControlledCar

`xWalkVoiceControlledCar` ports upstream `example/16.voice_controlled_car.py`
through caller-owned `XWalkPicarx` and `XWalkSpeechToText` objects. It waits for
“hey robot”, accepts repeated forward, backward, left, and right commands at
30-percent requested speed for one second, and returns to wake-word listening
when it hears “sleep”.

The source's centred straight movement and minus/plus 25-degree turns are
preserved. Empty and unknown transcripts do not move the car. Normal exit,
cancellation, and exceptions stop recognition and leave the motors stopped with
centred steering. Movement timing uses 20-millisecond cancellation slices while
preserving each source one-second interval. The Agent owns no motor, microphone, ALSA endpoint, Vosk
model, or application callback.
