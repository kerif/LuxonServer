### Read Before Contributing

Before submitting issues, pull requests, code, or design input that affects the project's **core compatibility logic**, you must verify that you meet the following legal requirements:

- **No Exit Games Agreements**: You must **never** have accepted, signed, or otherwise agreed to the Exit Games / Photon Engine Terms of Service, End User License Agreement (EULA), Non-Disclosure Agreement (NDA), or any other binding agreement with Exit Games in any capacity.

- **No White-Box Access**: You must not have decompiled, reverse-engineered using white-box methods, or viewed the source code of any proprietary Exit Games / Photon binaries or SDKs. Discovering functionality through black-box testing (interacting with the software externally to observe its behavior) is acceptable.

For the purposes of this policy, **core compatibility logic** means code, tests, specifications, or behavioral descriptions whose purpose is to reproduce or define Photon Realtime server behavior, protocol semantics, or client-visible compatibility behavior.

Examples of **core compatibility logic** include:

- Authentication and encryption negotiation behavior
- Operation, event, and response semantics and behavior
- Room, actor, join/leave, matchmaking, and event broadcasting rules
- Serialization formats, parameter meanings and ordering rules
- Other behavioral details implemented for protocol compatibility
- Tests or fixtures that define or lock in any of the above behavior

Examples that are **generally not** core compatibility logic include:

- Build scripts, CI configuration, packaging and release automation
- FFI layers, language bindings, ABI shims, or interop code that do not implement protocol semantics
- Logging, metrics, tracing, configuration loading, and admin/debug tooling
- Formatting, editor config, and linting setup
- Generic utilities or platform abstraction layers unrelated to Photon-compatible behavior

**Why is this necessary?**

If you have ever agreed to the Exit Games terms, you may be bound by restrictions against reverse engineering or creating derivative works. Accepting core compatibility contributions from people subject to those restrictions could expose this project to breach-of-contract or copyright claims.

If you do not meet these criteria, you are considered legally "tainted" for the purposes of this project's **core compatibility logic** and **cannot contribute to those parts**. You may still contribute to non-core areas, provided your contributions do not include proprietary code, confidential information, or behavior derived from prohibited white-box access.

If you are unsure whether a contribution touches core compatibility logic, *please ask before contributing*.
