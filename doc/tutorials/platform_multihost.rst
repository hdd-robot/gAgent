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
   * - 40016
     - TCP
     - Serveur de migration d'agents — uniquement si le programme appelle
       ``AgentFactory::startMigrationServer()``
   * - 50000–64999
     - TCP
     - Endpoints ZMQ agents (bidirectionnel entre toutes les machines)

.. code-block:: bash

   # Vérifier la connectivité avant de démarrer
   nc -zv 192.168.1.10 40011   # accès au master AMS
   nc -zv 192.168.1.20 40015   # accès au contrôle esclave


Modèle de menace
----------------

.. warning::

   Le mode cluster n'authentifie rien. Il est conçu pour un **réseau de
   confiance** : LAN privé, VPN (WireGuard, OpenVPN) ou pare-feu restreignant
   les ports ci-dessus aux seules IP des nœuds. Ne l'exposez pas sur Internet
   ni sur un réseau partagé avec des machines tierces.

En mode local (par défaut), l'AMS et le DF passent par des sockets Unix dans
``/tmp`` : l'accès est limité aux utilisateurs de la machine, comme pour
n'importe quel fichier. Le mode ``--master`` bascule ces mêmes services sur TCP,
en écoute sur toutes les interfaces, sans mot de passe ni chiffrement.

Ce qu'un tiers ayant accès au réseau peut faire, sans authentification :

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Surface
     - Conséquence
   * - AMS TCP (40011)
     - ``REGISTER_ENDPOINT bob 0 tcp://pirate:5555`` détourne **tous les
       messages destinés à bob** vers une autre machine. Le détournement est
       silencieux : l'agent légitime continue de tourner, ses messages partent
       ailleurs sans erreur. ``DEREGISTER bob`` coupe l'agent de la plateforme.
   * - DF TCP (40012)
     - Publier de faux services, désenregistrer les vrais, fausser les
       résultats de recherche des agents.
   * - Migration (40016)
     - ``ARRIVE <type> <nom> <attrs>`` fait **instancier des agents** sur le
       nœud, autant de fois que demandé. Les types créables se limitent à ceux
       enregistrés via ``AgentFactory::registerType()``, mais aucune limite de
       nombre n'est appliquée.
   * - Endpoints ZMQ (50000+)
     - Les sockets PULL acceptent n'importe quelle connexion PUSH : injection
       de messages ACL arbitraires dans n'importe quel agent, avec un champ
       ``:sender`` librement choisi.

Le champ ``:sender`` d'un message ACL est déclaratif : rien ne garantit qu'un
message vienne de l'agent qu'il désigne. Un behaviour ne doit donc pas accorder
de privilège sur la seule foi de ce champ dans un déploiement multi-machine.

Recommandations de déploiement
------------------------------

1. **Isoler le cluster.** VPN entre les nœuds, ou pare-feu n'autorisant les
   ports 40011, 40012, 40015, 40016 et 50000–64999 que depuis les IP des
   autres nœuds :

   .. code-block:: bash

      # Sur chaque nœud — n'autoriser que les pairs du cluster
      sudo ufw default deny incoming
      sudo ufw allow from 192.168.1.10 to any port 40011:40016 proto tcp
      sudo ufw allow from 192.168.1.10 to any port 50000:64999 proto tcp
      sudo ufw allow from 192.168.1.20 to any port 40011:40016 proto tcp
      sudo ufw allow from 192.168.1.20 to any port 50000:64999 proto tcp

2. **Ne démarrer le serveur de migration que si la migration est utilisée.**
   Il est désactivé par défaut : il ne s'ouvre que sur appel explicite à
   ``AgentFactory::startMigrationServer()``.

3. **Ne pas exposer** ``agentview`` **sur une interface publique.** Il écoute
   sur ``0.0.0.0:8080`` et publie sans authentification la liste des agents et
   leurs endpoints. Le placer derrière un reverse proxy authentifié, ou le
   restreindre à ``localhost`` (tunnel SSH depuis le poste de supervision).

4. **Ne pas faire transiter de données sensibles** dans le champ ``:content``
   des messages ACL : ils circulent en clair sur le réseau.

Ces limites sont assumées : gAgent est une plateforme de recherche. Si le
déploiement impose un réseau non fiable, il faut ajouter une couche
d'authentification — secret partagé sur les commandes AMS/DF/migration a
minima, ZMQ CURVE pour le plan de données — ou faire porter la sécurité par le
réseau (VPN), ce qui reste l'option la plus simple et la plus sûre.
