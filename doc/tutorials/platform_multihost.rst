20 — Déploiement multi-machine
================================

gAgent supporte nativement le déploiement sur plusieurs machines. Le
principe est simple : **une plateforme master** héberge l'AMS et le DF
pour l'ensemble du cluster ; les autres machines démarrent une
**plateforme esclave** qui se connecte au master. Les agents ne savent
pas où se trouvent leurs interlocuteurs — ils appellent ``acl_send``
exactement comme en local.

Architecture
------------

.. code-block:: text

   Machine A — master (192.168.1.10)
   ┌──────────────────────────────────────────┐
   │  agentplatform --master --ip 192.168.1.10 │
   │  AMS TCP:40011   DF TCP:40012             │
   │  SlaveRegistry (watchdog heartbeat)       │
   │                                           │
   │  AgentAlice  ←──────────────────────────┐ │
   └─────────────────────────────────────────┼─┘
            ↑ AMS lookup (tcp)               │ ZMQ direct
   Machine B — esclave (192.168.1.20)        │
   ┌─────────────────────────────────────────┼─┐
   │  agentplatform --slave 192.168.1.10:40011 │
   │  serveur contrôle TCP:40015              │ │
   │  heartbeat → master toutes les 5 s       │ │
   │                                          ↓ │
   │  AgentBob ────────────────────────────────► │
   └──────────────────────────────────────────┘

- Les agents s'enregistrent auprès du master AMS avec leur endpoint
  ZMQ TCP (``tcp://192.168.1.20:5xxxx``).
- ``acl_send("bob", msg)`` résout l'endpoint de Bob en interrogeant
  l'AMS, puis se connecte **directement** à la machine de Bob.
- Le master n'est jamais dans le chemin des messages.

Démarrage
----------

**Machine master** :

.. code-block:: bash

   ./bin/agentplatform --master --ip 192.168.1.10

Cela :

- Démarre AMS sur Unix + TCP:40011 et DF sur Unix + TCP:40012
- Lance le watchdog des esclaves (timeout 15 s)
- Écrit ``/tmp/gagent.cfg`` pour les agents locaux du master

**Machines esclaves** (autant que nécessaire) :

.. code-block:: bash

   ./bin/agentplatform --slave 192.168.1.10:40011

Cela :

- Se connecte au master et s'y enregistre
- Démarre un serveur de contrôle sur TCP:40015
- Envoie un heartbeat toutes les 5 secondes
- Écrit ``/tmp/gagent.cfg`` pour les agents locaux de l'esclave

Options supplémentaires
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   # Forcer l'IP de cette machine (utile si multi-NIC)
   ./bin/agentplatform --slave 192.168.1.10:40011 --ip 192.168.1.20

   # Changer le port AMS du master
   ./bin/agentplatform --master --ip 192.168.1.10 --port 41000

   # Changer le port de contrôle esclave (défaut 40015)
   ./bin/agentplatform --slave 192.168.1.10:40011 --control-port 40020

   # Changer la plage de ports ZMQ (défaut 50000–64999)
   ./bin/agentplatform --slave 192.168.1.10:40011 --base-port 55000

Le code des agents
-------------------

Le code est **identique** sur toutes les machines. Chaque agent lit
``/tmp/gagent.cfg`` via ``PlatformConfig`` pour savoir comment joindre
l'AMS.

.. code-block:: cpp

   #include <gagent/core/AgentCore.hpp>
   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/messaging/AclMQ.hpp>
   #include <gagent/messaging/ACLMessage.hpp>

   using namespace gagent;
   using namespace gagent::messaging;

   // ── Serveur (tourne sur n'importe quelle machine) ─────────────────────────────

   class ServeurBehaviour : public CyclicBehaviour {
   public:
       ServeurBehaviour(Agent* ag) : CyclicBehaviour(ag) {}

       void action() override {
           auto msg = acl_receive("serveur", 10000);
           if (!msg) return;

           std::cout << "[Serveur] reçu de " << msg->getSender().name
                     << " : " << msg->getContent() << std::endl;

           ACLMessage rep = msg->createReply(ACLMessage::Performative::INFORM);
           rep.setContent("reponse : " + msg->getContent());
           acl_send(msg->getSender().name, rep);
       }
   };

   class AgentServeur : public Agent {
   public:
       void setup() override {
           addBehaviour(new ServeurBehaviour(this));
       }
       void takeDown() override { acl_unlink("serveur"); }
   };

   // ── Client (tourne sur n'importe quelle machine) ──────────────────────────────

   class ClientBehaviour : public OneShotBehaviour {
   public:
       ClientBehaviour(Agent* ag) : OneShotBehaviour(ag) {}

       void action() override {
           ACLMessage req(ACLMessage::Performative::REQUEST);
           req.setSender(AgentIdentifier{"client"});
           req.setContent("bonjour depuis " + std::string("cette machine"));

           acl_send("serveur", req);   // trouve "serveur" via l'AMS

           auto rep = acl_receive("client", 5000);
           if (rep)
               std::cout << "[Client] réponse : " << rep->getContent() << std::endl;
       }
   };

   class AgentClient : public Agent {
   public:
       void setup() override {
           addBehaviour(new ClientBehaviour(this));
       }
       void takeDown() override { acl_unlink("client"); }
   };

   int main() {
       AgentCore::initAgentSystem();
       AgentServeur serveur; serveur.init();
       AgentClient  client;  client.init();
       AgentCore::syncAgentSystem();
   }

Lancer ce même binaire sur la machine du serveur **et** sur la machine
du client — les agents se trouvent automatiquement via l'AMS.

Ce qui se passe en détail
--------------------------

1. ``AgentServeur::init()`` appelle ``acl_bind("serveur")`` :
   - en mode cluster : lie un socket ZMQ sur ``tcp://*:5xxxx``
   - enregistre ``tcp://192.168.1.A:5xxxx`` auprès du master AMS

2. ``AgentClient`` appelle ``acl_send("serveur", msg)`` :
   - interroge le master AMS : ``LOOKUP serveur``
   - récupère ``tcp://192.168.1.A:5xxxx``
   - se connecte **directement** (le master n'est pas dans le chemin)
   - envoie le message

3. Heartbeat : l'esclave envoie ``HEARTBEAT`` toutes les 5 secondes.
   Si aucun heartbeat en 15 secondes, le master purge tous les agents
   de cet esclave.

Supervision
-----------

``agentmanager`` fonctionne depuis n'importe quelle machine du cluster :

.. code-block:: bash

   # Lister tous les agents (locaux + distants)
   agentmanager list

   # Tuer un agent sur une machine distante
   agentmanager kill bob     # le serveur de contrôle esclave exécute sigqueue

La colonne ADRESSE affiche l'endpoint ZMQ complet de chaque agent.

Pare-feu
---------

Ouvrez les ports suivants entre toutes les machines :

.. list-table::
   :widths: 15 15 70
   :header-rows: 1

   * - Port
     - Protocole
     - Usage
   * - 40011
     - TCP
     - AMS master (depuis esclaves + agents distants)
   * - 40012
     - TCP
     - DF master (depuis esclaves + agents distants)
   * - 40015
     - TCP
     - Serveur de contrôle esclave (depuis agentmanager)
   * - 50000–64999
     - TCP
     - Endpoints ZMQ agents (bidirectionnel entre toutes les machines)

.. code-block:: bash

   # Vérifier la connectivité avant de démarrer
   nc -zv 192.168.1.10 40011   # accès au master AMS
   nc -zv 192.168.1.20 40015   # accès au contrôle esclave
