# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - 2026-04-02

### Added

- **`ITransport`** (`include/gagent/messaging/ITransport.hpp`) — interface abstraite
  de transport FIPA : `bind`, `send`, `receive`, `unlink`, `flush`. Permet de brancher
  n'importe quel mécanisme de transport sans toucher aux protocoles.

- **`ZmqTransport`** (`include/gagent/messaging/ZmqTransport.hpp`) — implémentation
  ZeroMQ de `ITransport`. Délègue aux fonctions `acl_*` de `AclMQ.hpp`.
  Instancié automatiquement par `Agent` si aucun transport n'est injecté.

- **`MockTransport` + `MockBus`** (`include/gagent/messaging/MockTransport.hpp`) —
  transport en mémoire partagée pour les tests unitaires. `receive()` retourne
  immédiatement sans bloquer, ce qui permet de piloter les state machines tick par tick.

- **`Agent::transport()`** / **`Agent::setTransport()`** — accès et injection du
  transport de messagerie. Appeler `setTransport()` avant `init()` pour substituer
  `ZmqTransport` par un transport custom (email, HTTP, mock…).

- **`RequestParticipant::prepareAgree()`** — callback conforme FIPA SC00026H §3.4.
  Retourner `true` pour envoyer `AGREE` immédiatement (traitement long), puis laisser
  `handleRequest()` s'exécuter avant d'envoyer `INFORM`/`FAILURE`.

- **`RequestParticipant::onStart()`** — pré-bind du socket de réception dès le
  démarrage du behaviour (évite la perte du premier message).

- **`test_mock_transport`** — 4 scénarios, 17 assertions, temps d'exécution < 0.1s :
  Request sync, Request AGREE+INFORM, Request REFUSE, ContractNet complet.

- **`zmq_log_error()`** (`AclMQ.hpp`) — helper de log JSON pour les erreurs ZMQ
  (`zmq_socket`, `zmq_bind`, `zmq_connect`).

### Changed

- **Protocoles FIPA** (`ContractNet`, `Request`, `SubscribeNotify`) — utilisent
  `this_agent->transport()` au lieu d'appeler directement les fonctions ZMQ.
  Aucun changement d'API pour le code utilisateur existant.

- **`PythonBehaviour`** — utilise `this_agent->transport()` pour `send`/`receive`.

- **`Agent::_init()`** — fork fatal : `fork() == -1` lève `InitializationException`
  au lieu de continuer silencieusement.

- **`AMSClient::write()`** — résultat de `write()` vérifié, retour d'erreur si
  écriture incomplète.

- **`AgentFactory`** — appels `::write()` sur fd de migration marqués `(void)` (réponse
  intentionnellement ignorée, connexion peut être en train de fermer).

- **`acl_receive()`** — retry sur `EINTR` (signal reçu pendant `zmq_poll`) au lieu
  de retourner `nullopt` immédiatement.

- Mode dégradé AMS : l'agent démarre sans exception si l'AMS est absent, log
  `status: degraded_no_ams` au lieu de `status: active`.

### Removed

- **`Agent::control_message()`** — thread mort (boucle infinie sur une POSIX mqueue
  que personne n'écrit). Suppression du thread, de la méthode et de sa déclaration.

- **`SIG_AGENT_INFORM_PARENT`** — macro en collision avec `SIG_AGENT_TRANSIT`
  (même valeur `__SIGRTMIN+7`), jamais utilisée.

- **`CommunicationManager` CORBA** — stubs et infrastructure inutilisés supprimés.

---

## [0.9.0] - 2025-12-13

### Added
- Comprehensive README.md with full documentation
- CMake-based unified build system
- CI/CD pipeline with GitHub Actions
- Logger system for centralized logging
- ErrorHandler with custom exception types
- BehaviourFactory for dynamic behaviour creation
- **CommunicationManager** - Unified communication protocol manager
- **UDPChannel** - Improved non-blocking UDP communication
- **MQChannel** - Enhanced message queue handling
- **CORBAChannel** - Infrastructure for CORBA integration
- CONTRIBUTING.md guidelines
- LICENSE file (MIT)
- .clang-format for code formatting
- Code quality checks in CI
- BUGFIXES.md documentation
- Docker support with multi-stage builds
- Examples with modern C++ practices
- Automated build scripts

### Changed
- Updated C++ standard requirement to C++17
- Improved project structure documentation
- Enhanced build configuration with options
- **AgentCore**: Complete signal handling (SIGINT, SIGTERM, SIGQUIT)
- **Agent**: Graceful agent deletion with proper cleanup
- **Message Queue**: Safe buffer handling without overflows

### Fixed
- **Critical**: Signal handling now properly terminates all child processes
- **Critical**: Agent deletion uses SIGTERM before SIGKILL
- **Critical**: Message queue name generation buffer overflow
- **Critical**: Zombie process prevention with proper waitpid()
- **Critical**: Message queue cleanup on agent termination
- Race conditions in signal handlers
- Memory leaks in message queue name allocation
- Resource cleanup on agent shutdown
- Process group management
- Documentation for build process
- Missing license information

### Security
- Fixed buffer overflow in get_msg_queue_name()
- Added bounds checking for string operations
- Proper null termination in all buffers
- Resource cleanup to prevent leaks

## [0.8.0] - 2023-XX-XX

### Added
- Initial FIPA-compliant agent platform
- UDP-based communication
- Qt5 visualization support
- Behaviour system (Simple, Cyclic, Ticker)
- ACL message parsing
- Directory Facilitator service
- Agent lifecycle management
- CORBA interface support

### Known Issues
- Memory management needs smart pointer migration
- Signal handling incomplete for SIGTERM/SIGINT
- Race conditions in thread synchronization
- TODO: chldpid cleanup in Agent.cpp

