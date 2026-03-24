# gAgent — generative Agent Platform

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![FIPA](https://img.shields.io/badge/FIPA-ACL-orange.svg)](http://www.fipa.org)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)]()

**gAgent** est une plateforme multi-agent C++17 conforme au standard **FIPA ACL**,
conçue pour la **planification neuro-symbolique** : combiner des agents symboliques
(PDDL, HTN) avec des agents génératifs (LLM via `PythonBehaviour`).

---

## Fonctionnalités

- **Agents FIPA** — cycle de vie complet (init, active, suspend, wait, delete)
- **Messagerie ACL** — parser Flex/Bison, queues POSIX MQ, performatives FIPA
- **AMS** — registre central d'agents (Unix socket)
- **DF** — annuaire de services avec recherche par type/ontologie
- **Protocoles FIPA** — Contract Net, Subscribe-Notify, Request
- **PythonBehaviour** — bridge C++ ↔ Python pour agents LLM (OpenAI, Ollama…)
- **Logging structuré** — JSON Lines via `GAGENT_LOG`
- **Visualisation web** — `agentview` HTTP+SVG, zéro dépendance externe
- **CLI** — `agentmanager` pour lister, suspendre, tuer, chercher des services

## Architecture

```
gAgent/
├── include/gagent/        # Headers publics
│   ├── core/              # Agent, Behaviour, AgentCore
│   ├── messaging/         # ACLMessage, AclMQ, AgentIdentifier
│   ├── protocols/         # ContractNet, SubscribeNotify, Request
│   ├── platform/          # AMSClient, DFClient
│   ├── python/            # PythonBehaviour (bridge LLM)
│   └── utils/             # Logger (texte + JSON Lines)
├── src/                   # Implémentations
├── platform/              # Démons AMS, DF, agentmanager, agentview
├── python/gagent_py/      # Bibliothèque Python pour agents LLM
├── examples/              # Exemples complets
└── tests/                 # Tests CTest
```

Chaque agent tourne dans son **propre processus** (`fork`). La communication
inter-agent utilise des queues **POSIX MQ** nommées `/acl_<agent>`.

## Dépendances

```bash
sudo apt install build-essential cmake libboost-all-dev libconfig++-dev
```

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

Lancer la plateforme (AMS + DF requis pour l'enregistrement des agents) :

```bash
./platform/agentplatform &
```

## Démarrage rapide

```cpp
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/messaging/AclMQ.hpp>
using namespace gagent;
using namespace gagent::messaging;

class Hello : public Behaviour {
public:
    Hello(Agent* ag) : Behaviour(ag) {}
    void action() override {
        std::cout << "Hello from agent!\n";
        done_ = true;
    }
    bool done() override { return done_; }
private:
    bool done_ = false;
};

class MyAgent : public Agent {
public:
    void setup() override { addBehaviour(new Hello(this)); }
    void takeDown() override { acl_unlink("my-agent"); }
};

int main() {
    AgentCore::initAgentSystem();
    MyAgent ag;
    ag.init();
    AgentCore::syncAgentSystem();
}
```

## Protocoles FIPA disponibles

| Protocole | Classe | Usage |
|---|---|---|
| Contract Net (SC00029H) | `ContractNetInitiator` / `ContractNetParticipant` | Délégation par enchère |
| Subscribe-Notify (SC00035H) | `SubscribeInitiator` / `SubscribeParticipant` | Pub/sub événementiel |
| Request (SC00026H) | `RequestInitiator` / `RequestParticipant` | Requête/réponse simple |

## Agent LLM (PythonBehaviour)

```cpp
#include <gagent/python/PythonBehaviour.hpp>

class MyLLMAgent : public Agent {
    void setup() override {
        addBehaviour(new PythonBehaviour(
            this, "llm-agent", "path/to/script.py",
            "Tu es un assistant expert en planification.",
            "gpt-4o-mini", 200, 10
        ));
    }
};
```

```bash
OPENAI_API_KEY=sk-... ./build/examples/llm_agent
```

## Logging structuré

```bash
GAGENT_LOG=gagent.jsonl ./build/examples/demo_visualization
tail -f gagent.jsonl | jq .
```

```json
{"ts":"2026-03-24T10:00:00.123Z","event":"acl_send","from":"alice","to":"bob","perf":"request","conv":"req-alice-123","content":"add(3,4)"}
```

## Tests

```bash
cd build && ctest --output-on-failure
```

## Documentation

```bash
cd doc && make html   # Sphinx
# → doc/generated/html/index.html
```

## Licence

MIT — voir `LICENSE`.
