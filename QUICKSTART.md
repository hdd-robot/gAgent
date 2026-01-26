# Quick Start Guide

Get started with gAgent in 5 minutes!

## Prerequisites

```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake qtbase5-dev libboost-all-dev libconfig++-dev

# Fedora/RHEL
sudo dnf install gcc-c++ cmake qt5-qtbase-devel boost-devel libconfig-devel
```

## Build

```bash
# Simple build
./scripts/build.sh

# Debug build with sanitizers
./scripts/build.sh Debug
```

## Run First Example

```bash
# Console mode
./build/examples/improved_agent

# With GUI (if Qt available)
./build/examples/improved_agent --gui
```

## View Logs

```bash
cat improved_agent.log
```

## Create Your Own Agent

```cpp
#include "Agent.hpp"
#include "Behaviour.hpp"
#include "Logger.hpp"

using namespace gagent;

class MyBehaviour : public TickerBehaviour {
public:
    MyBehaviour(Agent* ag) : TickerBehaviour(ag, 1000) {}
    
    void onTick() override {
        LOG_INFO("My agent is working!");
    }
};

class MyAgent : public Agent {
public:
    void setup() override {
        addBehaviour(new MyBehaviour(this));
    }
};

int main() {
    Logger::getInstance().setLogLevel(LogLevel::INFO);
    AgentCore::initAgentSystem();
    
    MyAgent agent;
    agent.init();
    
    AgentCore::syncAgentSystem();
    return 0;
}
```

## Compile Your Agent

```bash
g++ -std=c++17 my_agent.cpp -o my_agent \
    -I./src_agent \
    -L./build/src_agent \
    -lgagent \
    -lboost_system -lboost_thread -lpthread \
    -lQt5Core -lQt5Widgets

export LD_LIBRARY_PATH=./build/src_agent:$LD_LIBRARY_PATH
./my_agent
```

## Next Steps

- Read [README.md](README.md) for full documentation
- Check [examples/](examples/) for more examples
- Review [CONTRIBUTING.md](CONTRIBUTING.md) to contribute
- Generate API docs: `doxygen Doxyfile`

## Troubleshooting

### Library not found
```bash
export LD_LIBRARY_PATH=/path/to/gAgent/build/src_agent:$LD_LIBRARY_PATH
```

### Qt not found
Install Qt5 development packages or build without GUI.

### Build fails
Check that all dependencies are installed and CMake version is 3.10+.

## Docker Alternative

```bash
docker build -t gagent .
docker run -it gagent bash
```

## Support

- Documentation: [README.md](README.md)
- Issues: Open on GitHub
- Examples: [examples/](examples/)
