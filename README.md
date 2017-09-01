## What is JabberHive?
JabberHive is a modular ChatBot system. All "modules" are in fact separate
programs linked together using the JabberHive Protocol. Please refer to the
protocol for more information.

## Component Description
* Filter for a JabberHive network.
* Stores passing STRINGs from ?RLR and ?RL requests.

## JabberHive Protocol Compatibility
* **Protocol Version(s):** 1.
* **Inbound Connections:** Multiple.
* **Outbound Connections:** Multiple.
* **Pipelining:** No.
* **Behavior:** Filter.

## Notes
* Does not correctly reply to Pipelining & Protocol Version requests.
* The jabberhive-cli Gateway can be used to feed a storage file to a JabberHive
network.

## Dependencies
- POSIX compliant OS.
- C compiler (with C99 support).
- (GNU) make.
