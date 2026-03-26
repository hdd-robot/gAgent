21 — Tutoriel : premier déploiement réseau
===========================================

Ce tutoriel guide pas-à-pas le déploiement d'un SMA sur deux machines
physiques (ou deux VMs). À la fin, un agent ``ping`` sur la machine A
échange des messages avec un agent ``pong`` sur la machine B.

**Hypothèses :**

- Machine A (master) : ``192.168.1.10``
- Machine B (esclave) : ``192.168.1.20``
- gAgent est compilé et ``dist/`` est disponible sur les deux machines

.. note::

   Si les deux machines n'ont pas encore gAgent, compilez-le sur l'une
   et copiez le répertoire ``dist/`` sur l'autre (même architecture) :

   .. code-block:: bash

      # Depuis la machine A
      scp -r dist/ user@192.168.1.20:~/dist/

---

Étape 1 — Ouvrir les ports
---------------------------

Sur **les deux machines**, ouvrez les ports nécessaires :

.. code-block:: bash

   # Ports AMS/DF du master (depuis les esclaves)
   sudo ufw allow 40011/tcp   # AMS
   sudo ufw allow 40012/tcp   # DF

   # Port de contrôle de l'esclave (depuis agentmanager)
   sudo ufw allow 40015/tcp

   # Plage ZMQ : communication directe entre agents
   sudo ufw allow 50000:65000/tcp

Vérifiez la connectivité avant de continuer :

.. code-block:: bash

   # Depuis la machine B — doit répondre
   nc -zv 192.168.1.10 40011

---

Étape 2 — Démarrer la plateforme master (Machine A)
-----------------------------------------------------

Sur la **machine A**, ouvrez un terminal dédié :

.. code-block:: bash

   ./dist/bin/agentplatform --master --ip 192.168.1.10

Sortie attendue :

.. code-block:: text

   [AMS] socket Unix prêt : /tmp/gagent_ams.sock
   [AMS] socket TCP prêt sur port 40011
   [DF] socket Unix prêt : /tmp/gagent_df.sock
   [DF] socket TCP prêt sur port 40012
   [Platform] master démarré — IP=192.168.1.10 AMS TCP:40011 DF TCP:40012
   [platform] configuration écrite dans /tmp/gagent.cfg
   [Platform] en attente (Ctrl+C pour arrêter)...

Le fichier ``/tmp/gagent.cfg`` est créé sur la machine A :

.. code-block:: ini

   master_port=40011
   slave_ip=192.168.1.10
   control_port=40015
   base_port=50000

---

Étape 3 — Démarrer la plateforme esclave (Machine B)
------------------------------------------------------

Sur la **machine B**, ouvrez un terminal dédié :

.. code-block:: bash

   ./dist/bin/agentplatform --slave 192.168.1.10:40011

Sortie attendue :

.. code-block:: text

   [AMS] socket Unix prêt : /tmp/gagent_ams.sock
   [DF] socket Unix prêt : /tmp/gagent_df.sock
   [Platform] esclave démarré — IP=192.168.1.20 master=192.168.1.10:40011
   [platform] configuration écrite dans /tmp/gagent.cfg
   [Platform] en attente (Ctrl+C pour arrêter)...

Simultanément, sur la machine A, vous verrez :

.. code-block:: text

   [SlaveRegistry] esclave enregistré : 192.168.1.20 (control_port=40015)

Le fichier ``/tmp/gagent.cfg`` est créé sur la machine B :

.. code-block:: ini

   master_ip=192.168.1.10
   master_port=40011
   slave_ip=192.168.1.20
   control_port=40015
   base_port=50000

---

Étape 4 — Vérifier le cluster (Machine A ou B)
------------------------------------------------

Depuis **n'importe quelle machine** :

.. code-block:: bash

   ./dist/bin/agentmanager watch 500

Vous verrez le cluster vide mais fonctionnel. Les agents apparaîtront
dès qu'ils seront lancés.

---

Étape 5 — Écrire les agents
-----------------------------

Créez un fichier ``ping_pong.cpp`` (le même binaire sera utilisé sur
les deux machines) :

.. code-block:: cpp

   #include <gagent/core/AgentCore.hpp>
   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/messaging/AclMQ.hpp>
   #include <gagent/messaging/ACLMessage.hpp>

   #include <iostream>
   #include <string>
   #include <cstdlib>   // getenv

   using namespace gagent;
   using namespace gagent::messaging;

   // ── Agent PONG — attend un PING, répond PONG ─────────────────────────────

   class PongBehaviour : public CyclicBehaviour {
       int count_ = 0;
   public:
       PongBehaviour(Agent* ag) : CyclicBehaviour(ag) {}

       void action() override {
           auto msg = acl_receive("pong", 10000);
           if (!msg) return;

           std::cout << "[Pong] reçu de " << msg->getSender().name
                     << " : " << msg->getContent() << std::endl;

           ACLMessage rep = msg->createReply(ACLMessage::Performative::INFORM);
           rep.setContent("PONG " + std::to_string(++count_));
           acl_send(msg->getSender().name, rep);

           if (count_ >= 5) setDone(true);
       }
   };

   class AgentPong : public Agent {
   public:
       void setup()    override { addBehaviour(new PongBehaviour(this)); }
       void onStart()  override { acl_bind("pong"); }
       void takeDown() override { acl_unlink("pong"); }
   };

   // ── Agent PING — envoie 5 PING et attend les PONG ────────────────────────

   class PingBehaviour : public OneShotBehaviour {
   public:
       PingBehaviour(Agent* ag) : OneShotBehaviour(ag) {}

       void action() override {
           for (int i = 1; i <= 5; ++i) {
               ACLMessage msg(ACLMessage::Performative::REQUEST);
               msg.setSender(AgentIdentifier{"ping"});
               msg.addReceiver(AgentIdentifier{"pong"});
               msg.setContent("PING " + std::to_string(i));

               std::cout << "[Ping] envoi → pong : PING " << i << std::endl;
               acl_send("pong", msg);

               auto rep = acl_receive("ping", 5000);
               if (rep)
                   std::cout << "[Ping] reçu ← pong : "
                             << rep->getContent() << std::endl;
               else
                   std::cout << "[Ping] timeout !" << std::endl;
           }
       }
   };

   class AgentPing : public Agent {
   public:
       void setup()    override { addBehaviour(new PingBehaviour(this)); }
       void onStart()  override { acl_bind("ping"); }
       void takeDown() override { acl_unlink("ping"); }
   };

   // ── main ─────────────────────────────────────────────────────────────────

   int main(int argc, char* argv[])
   {
       AgentCore::initAgentSystem();

       std::string role = (argc > 1) ? argv[1] : "";

       if (role == "pong") {
           AgentPong pong;
           pong.init();
       } else if (role == "ping") {
           AgentPing ping;
           ping.init();
       } else {
           std::cerr << "Usage : " << argv[0] << " ping|pong\\n";
           return 1;
       }

       AgentCore::syncAgentSystem();
       return 0;
   }

Et le ``CMakeLists.txt`` associé :

.. code-block:: cmake

   cmake_minimum_required(VERSION 3.15)
   project(ping_pong)

   find_package(gAgent REQUIRED)

   add_executable(ping_pong ping_pong.cpp)
   target_link_libraries(ping_pong PRIVATE gAgent::gAgent)

---

Étape 6 — Compiler et déployer
--------------------------------

Sur **les deux machines** (ou compilez une fois et copiez le binaire) :

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DCMAKE_PREFIX_PATH=~/dist
   cmake --build .

---

Étape 7 — Lancer les agents
-----------------------------

Les plateformes (étapes 2 et 3) doivent être actives.

Sur la **machine B** — lancer l'agent pong en premier :

.. code-block:: bash

   LD_LIBRARY_PATH=~/dist/lib ./build/ping_pong pong

Sur la **machine A** — lancer l'agent ping :

.. code-block:: bash

   LD_LIBRARY_PATH=~/dist/lib ./build/ping_pong ping

---

Résultat attendu
-----------------

**Machine B (pong) :**

.. code-block:: text

   [Pong] reçu de ping : PING 1
   [Pong] reçu de ping : PING 2
   [Pong] reçu de ping : PING 3
   [Pong] reçu de ping : PING 4
   [Pong] reçu de ping : PING 5

**Machine A (ping) :**

.. code-block:: text

   [Ping] envoi → pong : PING 1
   [Ping] reçu ← pong : PONG 1
   [Ping] envoi → pong : PING 2
   [Ping] reçu ← pong : PONG 2
   [Ping] envoi → pong : PING 3
   [Ping] reçu ← pong : PONG 3
   [Ping] envoi → pong : PING 4
   [Ping] reçu ← pong : PONG 4
   [Ping] envoi → pong : PING 5
   [Ping] reçu ← pong : PONG 5

**agentmanager watch (depuis n'importe quelle machine) :**

.. code-block:: text

   gAgent — 14:35:12  (Ctrl+C pour quitter)

   Agents (2)
   NOM                 PID     ENDPOINT ZMQ                   SLAVE        ÉTAT
   --------------------------------------------------------------------------
   ping                10101   tcp://192.168.1.10:52341        192.168.1.10 active
   pong                20202   tcp://192.168.1.20:58763        192.168.1.20 active

---

Ce qui se passe en coulisses
------------------------------

.. code-block:: text

   Machine A (ping)                 Master AMS                Machine B (pong)
   ─────────────────               ───────────────            ─────────────────
   acl_bind("ping")  ──REGISTER──► enregistre ping
                                                              acl_bind("pong")
                                   enregistre pong ◄─REGISTER──
   acl_send("pong") ──LOOKUP ────► renvoie tcp://192.168.1.20:58763
                    ◄─────────────
   ZMQ PUSH connect tcp://192.168.1.20:58763 ──────────────► ZMQ PULL (pong)
   (message direct, le master n'est plus dans le chemin)

1. Chaque agent appelle ``acl_bind("nom")`` dans ``onStart()`` : cela lie
   un socket ZMQ ``tcp://*:<port>`` et publie l'endpoint dans le master AMS.
2. ``acl_send("pong", msg)`` interroge le master AMS une seule fois pour
   obtenir l'endpoint TCP de ``pong``, puis se connecte directement.
3. Le master n'est **jamais** dans le chemin des messages — il sert
   uniquement d'annuaire.

---

Dépannage
----------

.. list-table::
   :widths: 40 60
   :header-rows: 1

   * - Symptôme
     - Cause probable / solution
   * - ``[AMSClient] plateforme non disponible``
     - ``agentplatform`` non lancé, ou ``/tmp/gagent.cfg`` absent.
       Vérifiez que la plateforme tourne avant les agents.
   * - ``[Ping] timeout !``
     - Port ZMQ bloqué par le pare-feu, ou ``agentplatform --slave``
       n'a pas été lancé sur la machine B. Vérifiez avec
       ``nc -zv 192.168.1.20 <port>``.
   * - Esclave non visible dans ``agentmanager``
     - Heartbeat non reçu. Vérifiez la connectivité
       ``nc -zv 192.168.1.10 40011`` depuis la machine B.
   * - Agents sur la même machine mais réseau attendu
     - En mode local pur (sans ``--master``), les agents utilisent IPC.
       Lancez bien ``agentplatform --master`` même si tout est sur A.
