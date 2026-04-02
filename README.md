# gAgent — Plateforme Multi-Agent FIPA C++17

[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![FIPA](https://img.shields.io/badge/FIPA-ACL-orange.svg)](http://www.fipa.org)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)]()

**gAgent** est une plateforme multi-agent C++17 conforme au standard **FIPA ACL**,
conçue pour la **planification neuro-symbolique** : combiner des agents symboliques
(PDDL, HTN) avec des agents génératifs (LLM via `PythonBehaviour`).

---

## Fonctionnalités

- **Agents FIPA** — cycle de vie complet (init, active, suspend, wait, transit, delete)
- **Messagerie ACL** — parser Flex/Bison, transport ZeroMQ PUSH/PULL, performatives FIPA
- **Transport abstrait** — interface `ITransport` pluggable (ZMQ par défaut, Email, Mock…)
- **AMS** — registre central d'agents (Unix socket)
- **DF** — annuaire de services avec recherche par type/ontologie
- **Protocoles FIPA** — Contract Net (SC00029H), Subscribe-Notify (SC00035H), Request (SC00026H)
- **PythonBehaviour** — bridge C++ ↔ Python pour agents LLM (OpenAI, Ollama…)
- **Logging structuré** — JSON Lines via `GAGENT_LOG`
- **Visualisation web** — `agentview` HTTP+SVG, zéro dépendance externe
- **CLI** — `agentmanager` pour lister, suspendre, tuer, chercher des services
- **Tests unitaires** — `MockTransport` pour tester les protocoles sans fork ni ZMQ

---

## Architecture

```
gAgent/
├── include/gagent/
│   ├── core/              # Agent, Behaviour (Simple/Cyclic/FSM/Composite), AgentCore
│   ├── messaging/
│   │   ├── ACLMessage.hpp        # Messages FIPA ACL
│   │   ├── ITransport.hpp        # Interface abstraite de transport
│   │   ├── ZmqTransport.hpp      # Implémentation ZeroMQ (défaut)
│   │   ├── MockTransport.hpp     # Transport en mémoire (tests unitaires)
│   │   └── AclMQ.hpp             # Fonctions acl_bind/send/receive/unlink
│   ├── protocols/
│   │   ├── ContractNet.hpp       # FIPA SC00029H
│   │   ├── Request.hpp           # FIPA SC00026H (avec AGREE+INFORM différé)
│   │   └── SubscribeNotify.hpp   # FIPA SC00035H
│   ├── platform/          # AMSClient, DFClient, PlatformConfig
│   ├── python/            # PythonBehaviour (bridge LLM)
│   └── utils/             # Logger (texte + JSON Lines), ErrorHandler
├── src/                   # Implémentations
├── platform/              # Démons AMS, DF, agentmanager, agentview
├── python/gagent_py/      # Bibliothèque Python pour agents LLM
├── examples/              # Exemples complets
└── tests/                 # Tests CTest (intégration + unitaires MockTransport)
```

Chaque agent tourne dans son **propre processus** (`fork`). La communication
inter-agent utilise **ZeroMQ PUSH/PULL** (IPC local ou TCP cluster).

---

## Dépendances

```bash
sudo apt install build-essential cmake libboost-all-dev libconfig++-dev libzmq3-dev
```

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure
```

Lancer la plateforme (AMS + DF, optionnels en mode local) :

```bash
./build/platform/agentplatform &
```

---

## Démarrage rapide

```cpp
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
using namespace gagent;

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
    void setup()    override { addBehaviour(new Hello(this)); }
    void takeDown() override { /* acl_unlink si messagerie utilisée */ }
};

int main() {
    AgentCore::initAgentSystem();
    MyAgent ag;
    ag.init();
    AgentCore::syncAgentSystem();
}
```

---

## Protocoles FIPA disponibles

| Protocole | Classes | Flux |
|-----------|---------|------|
| Contract Net (SC00029H) | `ContractNetInitiator` / `ContractNetParticipant` | CFP → PROPOSE → ACCEPT/REJECT → INFORM |
| Subscribe-Notify (SC00035H) | `SubscribeInitiator` / `SubscribeParticipant` | SUBSCRIBE → AGREE → INFORM* |
| Request (SC00026H) | `RequestInitiator` / `RequestParticipant` | REQUEST → [AGREE →] INFORM/REFUSE/FAILURE |

### Exemple — Protocole Request

```cpp
#include <gagent/protocols/Request.hpp>
using namespace gagent::protocols;

// Côté client
class MyRequester : public RequestInitiator {
public:
    MyRequester(Agent* ag)
        : RequestInitiator(ag, "alice", "bob", "add(3,4)", "math") {}

    void handleInform(const ACLMessage& msg) override {
        std::cout << "Résultat : " << msg.getContent() << "\n";
    }
};

// Côté serveur — réponse directe
class MyServer : public RequestParticipant {
public:
    MyServer(Agent* ag) : RequestParticipant(ag, "bob") {}

    ACLMessage handleRequest(const ACLMessage& req) override {
        auto reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setContent("7");
        return reply;
    }
};

// Côté serveur — traitement long (FIPA SC00026H §3.4)
class SlowServer : public RequestParticipant {
public:
    SlowServer(Agent* ag) : RequestParticipant(ag, "bob") {}

    // Retourner true → AGREE envoyé immédiatement, INFORM après traitement
    bool prepareAgree(const ACLMessage&) override { return true; }

    ACLMessage handleRequest(const ACLMessage& req) override {
        std::this_thread::sleep_for(std::chrono::seconds(5)); // traitement long
        auto reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setContent("done");
        return reply;
    }
};
```

### Exemple — Contract Net

```cpp
#include <gagent/protocols/ContractNet.hpp>
using namespace gagent::protocols;

class MyInitiator : public ContractNetInitiator {
public:
    MyInitiator(Agent* ag, std::vector<AgentIdentifier> parts)
        : ContractNetInitiator(ag, "manager", cfp, parts) {}

    std::vector<std::string> selectProposals(
            const std::vector<ACLMessage>& proposals) override {
        // Accepter la meilleure offre
        return { proposals[0].getSender().name };
    }
    void handleInform(const ACLMessage& msg) override {
        std::cout << "Tâche accomplie : " << msg.getContent() << "\n";
    }
};

class MyWorker : public ContractNetParticipant {
public:
    MyWorker(Agent* ag, const std::string& name)
        : ContractNetParticipant(ag, name) {}

    ACLMessage prepareProposal(const ACLMessage&) override {
        ACLMessage p(ACLMessage::Performative::PROPOSE);
        p.setContent("coût:42");
        return p;
    }
    ACLMessage executeTask(const ACLMessage&) override {
        ACLMessage r(ACLMessage::Performative::INFORM);
        r.setContent("tâche exécutée");
        return r;
    }
};
```

---

## Transport de messagerie

Les protocoles FIPA n'ont aucune dépendance directe sur ZeroMQ. Ils passent
par l'interface `ITransport` injectée dans l'agent.

### Transport par défaut (ZeroMQ)

```
Agent::transport() → ZmqTransport → AclMQ (ZMQ PUSH/PULL IPC/TCP)
```

Aucune configuration nécessaire — `ZmqTransport` est instancié automatiquement.

### Injecter un transport custom

```cpp
class EmailTransport : public gagent::messaging::ITransport {
public:
    EmailTransport(const SmtpConfig& cfg) : smtp_(cfg) {}

    std::string bind(const std::string& name) override {
        return "mailto:" + name + "@myplatform.org";
    }
    bool send(const std::string& to, const ACLMessage& msg) override {
        return smtp_.sendMail(to + "@myplatform.org", msg.toString());
    }
    std::optional<ACLMessage> receive(const std::string& name,
                                      int timeout_ms) override {
        auto raw = imap_.poll(name, timeout_ms);
        if (!raw) return std::nullopt;
        return ACLMessage::parse(*raw);
    }
    void unlink(const std::string&) override {}
    void flush()                    override {}
private:
    SmtpClient smtp_;
    ImapClient imap_;
};

// Dans main(), avant agent.init() :
MyAgent agent;
agent.setTransport(std::make_shared<EmailTransport>(smtpConfig));
agent.init();
```

Les protocoles ContractNet, Request et SubscribeNotify fonctionnent sans aucune
modification quel que soit le transport injecté.

---

## Tests unitaires avec MockTransport

`MockTransport` permet de tester les protocoles FIPA sans fork, sans ZeroMQ
et sans délais réseau. Les state machines sont pilotées manuellement.

```cpp
#include <gagent/messaging/MockTransport.hpp>
#include <gagent/protocols/Request.hpp>

auto bus = std::make_shared<MockBus>();

// Agents de test (pas de fork, pas d'init())
class StubAgent : public Agent {
public:
    explicit StubAgent(std::shared_ptr<ITransport> t) { setTransport(std::move(t)); }
    void setup() override {}
};

StubAgent alice(std::make_shared<MockTransport>(bus));
StubAgent bob  (std::make_shared<MockTransport>(bus));

// Behaviours instanciés directement
MyRequester req(&alice, ...);
MyServer    srv(&bob,   ...);

req.onStart();  srv.onStart();
req.action();   // envoie REQUEST dans le bus mémoire
srv.action();   // reçoit REQUEST → envoie INFORM
req.action();   // reçoit INFORM → handleInform()

assert(req.done());
```

---

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

---

## Logging structuré

```bash
GAGENT_LOG=gagent.jsonl ./build/examples/demo_visualization
tail -f gagent.jsonl | jq .
```

```json
{"ts":"2026-04-01T10:00:00.123Z","event":"acl_send","from":"alice","to":"bob","perf":"request","conv":"req-alice-123","content":"add(3,4)"}
{"ts":"2026-04-01T10:00:00.145Z","event":"acl_recv","to":"bob","from":"alice","perf":"request","conv":"req-alice-123","content":"add(3,4)"}
{"ts":"2026-04-01T10:00:00.150Z","event":"agent_start","agent":"bob","pid":"12345","status":"active"}
```

Événements disponibles : `acl_send`, `acl_recv`, `acl_error`, `agent_start`, `agent_migrate`.

---

## Tests

```bash
cd build && ctest --output-on-failure
```

| Test | Type | Durée |
|------|------|-------|
| `two_agents_acl` | Intégration ZMQ | ~0.1s |
| `test_nsap` | Intégration AMS | ~0.6s |
| `test_acl` | Unitaire ACL parser | ~0.01s |
| `test_platform` | Intégration plateforme | ~0.01s |
| `test_contract_net` | Intégration ZMQ | ~0.5s |
| `test_request` | Intégration ZMQ | ~1.2s |
| `test_subscribe_notify` | Intégration ZMQ | ~16s |
| `test_concurrent_send` | Intégration ZMQ | ~0.1s |
| `test_mock_transport` | **Unitaire MockTransport** | **~0.01s** |

---

## Licence

MIT — voir `LICENSE`.
