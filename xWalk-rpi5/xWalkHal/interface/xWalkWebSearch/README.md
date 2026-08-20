# xWalkWebSearch

`xWalkWebSearch` is a bounded, provider-neutral client for a locally controlled
SearXNG JSON endpoint. It accepts only a loopback `/search` endpoint, never
follows result URLs, rejects credential-bearing and local-network result URLs,
strips HTML-like tags and control bytes, and delimits returned text as untrusted
reference material. Search remains optional; local model conversation does not
depend on SearXNG.
