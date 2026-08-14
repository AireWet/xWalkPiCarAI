# Produce speech

Create the platform synthesis backend, `XWalkBoardControl`, and
`XWalkTextToSpeech` in `main()`. Enable speaker power only after target audio
configuration is valid, then pass bounded text to the injected synthesis
callback.

The application owns model selection, process or network access, credentials,
audio files, and cleanup. The HAL does not invoke external tools implicitly.
