# gAgent - FIPA Compliant Multi-Agent Platform

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)]()
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)]()

A C++ implementation of a FIPA-compliant multi-agent platform with Qt visualization and distributed communication capabilities.

## 📋 Table of Contents

- [Features](#features)
- [Architecture](#architecture)
- [Requirements](#requirements)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Documentation](#documentation)
- [Examples](#examples)
- [Contributing](#contributing)

## ✨ Features

- **FIPA Compliance**: Full implementation of FIPA standards for agent communication
- **Distributed Architecture**: UDP-based communication using Boost.Asio
- **Behavior System**: Flexible behavior composition for agent logic
- **Qt Visualization**: Real-time GUI for environment monitoring
- **Agent Lifecycle**: Complete lifecycle management (init, suspend, resume, delete)
- **Directory Facilitator**: Service discovery and registration
- **ACL Messaging**: FIPA-ACL compliant message parsing and handling
- **Multi-threading**: Thread-safe agent execution with behavior scheduling

## 🏗️ Architecture

```
gAgent/
├── src_agent/          # Core agent library
│   ├── Agent.cpp/hpp   # Main agent class
│   ├── Behaviour.cpp/hpp # Behavior system
│   ├── ACLMessage.cpp/hpp # FIPA-ACL messages
│   ├── Environnement.cpp/hpp # Environment abstraction
│   └── ...
├── src_manager/        # Platform management tools
│   ├── AgentPlatform   # Platform controller
│   ├── AgentManager    # Agent registry
│   └── AgentMonitor    # Monitoring service
├── test_agent/         # Example implementations
├── corba/              # CORBA/IDL interface
└── doc/                # Documentation

```

### Key Components

- **Agent**: Base class for all agents, manages lifecycle and behaviors
- **Behaviour**: Abstract class for defining agent actions (SimpleBehaviour, TickerBehaviour, CyclicBehaviour)
- **ACLMessage**: FIPA-ACL message structure and parsing
- **Environnement**: Environment abstraction with Qt visualization
- **AgentPlatform**: Central platform for agent registration and discovery

## 📦 Requirements

### System Dependencies

- **C++ Compiler**: GCC 7+ or Clang 5+ with C++17 support
- **CMake**: 3.10 or higher
- **Qt5**: Core, Widgets, Gui modules
- **Boost**: 1.65+ (system, thread, filesystem, asio)
- **libconfig++**: 1.5+

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    qtbase5-dev \
    libboost-all-dev \
    libconfig++-dev
```

### Fedora/RHEL

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    qt5-qtbase-devel \
    boost-devel \
    libconfig-devel
```

## 🚀 Installation

### Building the Agent Library

```bash
cd src_agent
mkdir -p build && cd build
cmake ..
make -j$(nproc)
sudo make install  # Optional: install system-wide
```

### Building the Platform Manager

```bash
cd src_manager
make
```

### Building Test Examples

```bash
cd test_agent
mkdir -p build && cd build
cmake ..
make
```

## 🎯 Quick Start

### 1. Start the Platform Services

```bash
# Terminal 1: Start the agent platform
cd src_manager
./agentplatform

# Terminal 2: Start the monitoring service
./agentmonitor
```

### 2. Create Your First Agent

```cpp
#include "Agent.hpp"
#include "Behaviour.hpp"
#include "Environnement.hpp"

using namespace gagent;

class MyBehaviour : public TickerBehaviour {
public:
    MyBehaviour(Agent* ag) : TickerBehaviour(ag, 1000) {} // 1 second tick
    
    void onTick() override {
        std::cout << "Agent tick!" << std::endl;
        this_agent->setAttribut("status", "active");
    }
};

class MyAgent : public Agent {
public:
    void setup() override {
        std::cout << "Agent setup" << std::endl;
        addBehaviour(new MyBehaviour(this));
    }
    
    void takeDown() override {
        std::cout << "Agent shutdown" << std::endl;
    }
};

int main() {
    AgentCore::initAgentSystem();
    
    MyAgent agent;
    agent.init();
    
    AgentCore::syncAgentSystem();
    return 0;
}
```

### 3. Compile and Run

```bash
g++ -std=c++17 my_agent.cpp -o my_agent \
    -I../src_agent \
    -L../src_agent/build \
    -lgagent \
    -lboost_system -lboost_thread -lpthread \
    -lQt5Core -lQt5Widgets

./my_agent
```

## 📚 Documentation

### Agent Lifecycle

```cpp
// Agent states
AGENT_UNKNOWN    // Initial state
AGENT_INITED     // After init()
AGENT_ACTIVE     // Running
AGENT_SUSPENDED  // Temporarily paused
AGENT_WAITING    // Waiting for event
AGENT_DELETED    // Marked for deletion
```

### Behavior Types

1. **SimpleBehaviour**: Executes once then stops
2. **CyclicBehaviour**: Runs continuously in a loop
3. **TickerBehaviour**: Executes at fixed time intervals

```cpp
class MyTicker : public TickerBehaviour {
public:
    MyTicker(Agent* ag) : TickerBehaviour(ag, 500) {} // 500ms interval
    
    void onTick() override {
        // Your periodic logic here
    }
};
```

### ACL Message Communication

```cpp
// Create a message
ACLMessage msg("INFORM");
msg.addReceiver(AgentID("receiver_agent"));
msg.setContent("Hello from sender");
msg.setLanguage("English");
msg.setOntology("conversation");

// Send via agent
this_agent->sendMessage(msg);
```

### Environment Visualization

```cpp
class MyEnv : public Environnement {
public:
    void init_env() override {
        std::cout << "Environment initialized" << std::endl;
    }
    
    void link_attribut() override {
        link_id("id");
        link_name("name");
        link_pos_x("x");
        link_pos_y("y");
    }
    
    void event_loop() override {
        // Refresh logic
    }
};

// Initialize with GUI
MyEnv env;
AgentCore::initEnvironnementSystem(env, true, 500);
```

## 📖 Examples

See the `test_agent/` directory for complete working examples:

- **Simple Agent**: Basic agent with ticker behavior
- **Communication**: Inter-agent messaging
- **Environment**: Agents with GUI visualization
- **Behaviors**: Different behavior patterns

## 🔧 Configuration

The platform uses `config.cfg` for configuration:

```ini
# Platform Configuration
plt_address = "127.0.0.1"
plt_port = 40010

# Manager Configuration
mng_address = "127.0.0.1"
mng_port = 40011

# Monitor Configuration
mon_address = "127.0.0.1"
mon_port = 40013
```

## 🐛 Known Issues

- Signal handling needs SIGTERM/SIGINT improvement (see TODO in AgentCore.cpp)
- Thread synchronization in high-load scenarios may need optimization
- Memory management should migrate to smart pointers (work in progress)

## 🤝 Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Development Guidelines

- Follow C++17 best practices
- Use smart pointers for dynamic allocation
- Add unit tests for new features
- Document public APIs with Doxygen comments
- Run tests before submitting PRs

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 👥 Authors

- **HD** - *Initial work* - <hdd@ai.univ-paris8.fr>

## 🙏 Acknowledgments

- FIPA (Foundation for Intelligent Physical Agents) for the agent communication standards
- Qt Project for the GUI framework
- Boost community for excellent C++ libraries

## 📧 Contact

For questions and support, please open an issue on the project repository.

---

**Note**: This is an academic research project for multi-agent systems simulation and distributed AI applications.
