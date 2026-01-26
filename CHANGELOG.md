# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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

