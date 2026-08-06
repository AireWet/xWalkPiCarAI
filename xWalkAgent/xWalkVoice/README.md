# xWalk Voice Agents

`xWalkVoice` groups the local chatbot, spoken movement, storytelling, voice-control, vision-language,
online-language-model, voice-active-car, and GPT-car coordinators.

Consumers can link `xWalk::AgentVoice`. The child modules remain distinct because they have different wake
profiles, model providers, prompt flows, and hardware dependencies.

Set `XWALK_AGENT_VOICE_BUILD_HOST_TESTS=ON` to run one GoogleTest case per child module independently.
Set `XWALK_AGENT_VOICE_BUILD_HARDWARE_TESTS=ON` to compile the matching hardware-profile cases.
