# gAgent — Démarrage rapide

## Prérequis

```bash
sudo apt install build-essential cmake libboost-all-dev libconfig++-dev libzmq3-dev flex bison
```

## Build

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
ctest --output-on-failure   # 9/9 tests doivent passer
```

---

## 1. Premier agent (Hello World)

```cpp
// hello.cpp
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
using namespace gagent;

class Hello : public OneShotBehaviour {
public:
    Hello(Agent* ag) : OneShotBehaviour(ag) {}
    void action() override {
        std::cout << "Hello depuis l'agent PID " << getpid() << "\n";
    }
};

class HelloAgent : public Agent {
public:
    void setup() override { addBehaviour(new Hello(this)); }
};

int main() {
    AgentCore::initAgentSystem();
    HelloAgent ag;
    ag.init();
    AgentCore::syncAgentSystem();
}
```

```bash
g++ -std=c++17 hello.cpp -o hello \
    -I./include -L./build/src -lgagent \
    -lboost_system -lboost_thread -lpthread -lzmq -lconfig++
./hello
# Hello depuis l'agent PID 12345
```

---

## 2. Deux agents qui communiquent (protocole Request)

Le protocole **FIPA Request** permet à un agent de demander un service à un autre.

```cpp
// request_demo.cpp
#include <gagent/core/Agent.hpp>
#include <gagent/core/AgentCore.hpp>
#include <gagent/protocols/Request.hpp>
#include <gagent/messaging/AclMQ.hpp>
using namespace gagent;
using namespace gagent::protocols;
using namespace gagent::messaging;

// ── Serveur de calcul ──────────────────────────────────────────────────────

class CalcServer : public RequestParticipant {
    int count_ = 0;
public:
    CalcServer(Agent* ag) : RequestParticipant(ag, "server") {}

    ACLMessage handleRequest(const ACLMessage& req) override {
        count_++;
        // Exemple : "add(3,4)" → "7"
        auto reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setContent("7");
        return reply;
    }
    bool done() override { return count_ >= 1; }
};

class ServerAgent : public Agent {
public:
    void setup()    override { addBehaviour(new CalcServer(this)); }
    void takeDown() override { acl_unlink("server"); }
};

// ── Client ─────────────────────────────────────────────────────────────────

class CalcClient : public RequestInitiator {
public:
    CalcClient(Agent* ag)
        : RequestInitiator(ag, "client", "server", "add(3,4)", "math") {}

    void handleInform(const ACLMessage& msg) override {
        std::cout << "Résultat : " << msg.getContent() << "\n";
    }
    void handleTimeout() override {
        std::cerr << "Timeout !\n";
    }
};

class ClientAgent : public Agent {
public:
    void setup()    override { addBehaviour(new CalcClient(this)); }
    void takeDown() override { acl_unlink("client"); }
};

// ── main ───────────────────────────────────────────────────────────────────

int main() {
    AgentCore::initAgentSystem();

    ServerAgent server;
    server.init();

    // Laisser le serveur se lier avant que le client envoie
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    ClientAgent client;
    client.init();

    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();
}
```

---

## 3. Traitement long avec AGREE (FIPA SC00026H §3.4)

Pour un serveur dont le traitement prend du temps, envoyer `AGREE` d'abord
notifie le client que la requête est acceptée avant que le résultat soit prêt.

```cpp
class SlowServer : public RequestParticipant {
public:
    SlowServer(Agent* ag) : RequestParticipant(ag, "slow-server") {}

    // Retourner true → framework envoie AGREE automatiquement
    bool prepareAgree(const ACLMessage&) override { return true; }

    ACLMessage handleRequest(const ACLMessage& req) override {
        // Ce bloc peut bloquer — le client a déjà reçu AGREE
        std::this_thread::sleep_for(std::chrono::seconds(5));
        auto reply = req.createReply(ACLMessage::Performative::INFORM);
        reply.setContent("résultat après traitement long");
        return reply;
    }
};

// Côté client : handleAgree() est appelé à la réception du AGREE
class MyClient : public RequestInitiator {
public:
    MyClient(Agent* ag)
        : RequestInitiator(ag, "client", "slow-server", "long-task", "", 30000) {}

    void handleAgree (const ACLMessage&) override {
        std::cout << "Serveur a accepté, traitement en cours...\n";
    }
    void handleInform(const ACLMessage& msg) override {
        std::cout << "Résultat final : " << msg.getContent() << "\n";
    }
};
```

---

## 4. Contract Net (délégation par enchère)

Le protocole **FIPA Contract Net** permet à un agent manager de trouver le
meilleur exécutant pour une tâche via un système d'appel d'offres.

```cpp
#include <gagent/protocols/ContractNet.hpp>
using namespace gagent::protocols;

// ── Initiateur (manager) ───────────────────────────────────────────────────

class TaskManager : public ContractNetInitiator {
public:
    TaskManager(Agent* ag, std::vector<AgentIdentifier> workers)
        : ContractNetInitiator(ag, "manager",
                               make_cfp(), workers,
                               3000,   // timeout collecte offres
                               5000)   // timeout résultat
    {}

    // Sélectionner l'offre avec le coût le plus bas
    std::vector<std::string> selectProposals(
            const std::vector<ACLMessage>& proposals) override
    {
        if (proposals.empty()) return {};
        auto best = std::min_element(proposals.begin(), proposals.end(),
            [](const ACLMessage& a, const ACLMessage& b) {
                return std::stoi(a.getContent()) < std::stoi(b.getContent());
            });
        std::cout << "Sélectionné : " << best->getSender().name << "\n";
        return { best->getSender().name };
    }

    void handleInform(const ACLMessage& msg) override {
        std::cout << "Tâche accomplie : " << msg.getContent() << "\n";
    }

private:
    static ACLMessage make_cfp() {
        ACLMessage cfp(ACLMessage::Performative::CFP);
        cfp.setContent("do-task");
        return cfp;
    }
};

// ── Participant (worker) ───────────────────────────────────────────────────

class Worker : public ContractNetParticipant {
    int cost_;
public:
    Worker(Agent* ag, const std::string& name, int cost)
        : ContractNetParticipant(ag, name), cost_(cost) {}

    ACLMessage prepareProposal(const ACLMessage&) override {
        ACLMessage p(ACLMessage::Performative::PROPOSE);
        p.setContent(std::to_string(cost_));
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

## 5. Brancher un transport custom

Par défaut gAgent utilise **ZeroMQ**. L'interface `ITransport` permet de
remplacer le transport sans toucher aux protocoles.

```cpp
#include <gagent/messaging/ITransport.hpp>

class MyTransport : public gagent::messaging::ITransport {
public:
    std::string bind(const std::string& name) override {
        // Enregistrer la queue de réception pour 'name'
        return "custom://" + name;
    }
    bool send(const std::string& to, const ACLMessage& msg) override {
        // Envoyer msg.toString() à 'to'
        return true;
    }
    std::optional<ACLMessage> receive(const std::string& name,
                                      int timeout_ms) override {
        // Attendre un message pour 'name' pendant timeout_ms
        return std::nullopt;  // nullopt = timeout
    }
    void unlink(const std::string&) override {}
    void flush()                    override {}
};

// Injection avant init()
MyAgent agent;
agent.setTransport(std::make_shared<MyTransport>());
agent.init();
```

---

## 6. Tests unitaires avec MockTransport

`MockTransport` est un transport en mémoire pour tester les protocoles
sans fork, sans réseau, sans délais.

```cpp
#include <gagent/messaging/MockTransport.hpp>
#include <gagent/protocols/Request.hpp>
using namespace gagent::messaging;
using namespace gagent::protocols;

// Agent minimal de test (pas de fork)
class StubAgent : public Agent {
public:
    explicit StubAgent(std::shared_ptr<ITransport> t) { setTransport(std::move(t)); }
    void setup() override {}
};

// ── Test ───────────────────────────────────────────────────────────────────

auto bus = std::make_shared<MockBus>();
StubAgent alice(std::make_shared<MockTransport>(bus));
StubAgent bob  (std::make_shared<MockTransport>(bus));

bool inform_received = false;

struct Requester : RequestInitiator {
    bool& flag_;
    Requester(Agent* ag, bool& f)
        : RequestInitiator(ag, "alice", "bob", "hello"), flag_(f) {}
    void handleInform(const ACLMessage&) override { flag_ = true; }
};

struct Server : RequestParticipant {
    Server(Agent* ag) : RequestParticipant(ag, "bob") {}
    ACLMessage handleRequest(const ACLMessage& req) override {
        auto r = req.createReply(ACLMessage::Performative::INFORM);
        r.setContent("world");
        return r;
    }
};

Requester req(&alice, inform_received);
Server    srv(&bob);

req.onStart();  srv.onStart();
req.action();   // envoie REQUEST
srv.action();   // reçoit REQUEST → envoie INFORM
req.action();   // reçoit INFORM → handleInform()

assert(inform_received);  // ✓
```

---

## Logging

```bash
# Activer le log JSON Lines
GAGENT_LOG=gagent.jsonl ./mon_agent

# Suivre en temps réel
tail -f gagent.jsonl | jq '{event: .event, from: .from, to: .to}'
```

---

## Dépannage

**`libgagent.so` introuvable :**
```bash
export LD_LIBRARY_PATH=/chemin/vers/gAgent/build/src:$LD_LIBRARY_PATH
```

**Agents qui ne se voient pas :**
- Vérifier que les noms de queue correspondent (`acl_bind("bob")` ↔ `acl_send("bob", ...)`)
- En mode cluster, lancer `agentplatform` pour que l'AMS soit disponible

**Timeout systématique :**
- S'assurer que le serveur appelle `acl_bind()` (via `onStart()`) avant que le client envoie
- Ajouter un `sleep_for(100ms)` entre `server.init()` et `client.init()`

---

## Prochaines étapes

- [README.md](README.md) — documentation complète
- [examples/](examples/) — exemples concrets
- [tests/](tests/) — lire les tests pour voir des patterns d'usage
