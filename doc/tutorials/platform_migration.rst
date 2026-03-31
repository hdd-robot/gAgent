Migration d'agents
==================

La **migration d'agents** permet de déplacer un agent en cours d'exécution
d'une machine vers une autre. L'agent transfère ses **attributs** (son état
observable) et se recrée sur la machine cible, grâce à une factory enregistrée
côté destination.

.. note::

   En C++, les objets ne sont pas sérialisables automatiquement comme en Java.
   La migration gAgent transfère les **attributs** de l'agent
   (``addAttribut`` / ``setAttribut``), mais pas l'état interne des behaviours.
   L'agent repart de ``setup()`` avec ses attributs restaurés.

----

Principe de fonctionnement
---------------------------

::

   Machine A (source)                  Machine B (destination)
   ──────────────────                  ───────────────────────
   Agent X actif                       AgentFactory::startMigrationServer()
        │                                    │
        │  doMove({"192.168.1.20"})           │
        │──────── ARRIVE MonAgent X ─────────>│
        │         id:x;pos_x:10;pos_y:5;      │
        │                                     │  factory.create("MonAgent")
        │                               <─── OK │  agent->setAgentName("X")
        │                                     │  restoreAttributs(...)
        │  deregister AMS/DF                  │  agent->init()
        │  _exit(0)                           │

Le port de migration est ``control_port + 1`` (défaut : **40016**).

----

Étape 1 — Déclarer le type d'agent
------------------------------------

Chaque agent migrable doit surcharger ``agentTypeName()`` pour retourner
un identifiant de type unique :

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/AgentFactory.hpp>

   class AgentMobile : public Agent {
       int missions_faites_ = 0;

   public:
       // ← Obligatoire pour la migration
       std::string agentTypeName() const override { return "AgentMobile"; }

       void setup() override {
           // Les attributs sont restaurés AVANT setup()
           std::string pos = getAttribut("position");
           std::cout << "Démarrage à la position : " << pos << "\n";

           addBehaviour(new MissionBehaviour(this));
       }
   };

----

Étape 2 — Enregistrer le type et démarrer le serveur de migration
------------------------------------------------------------------

Dans le ``main()`` de **chaque machine** participante :

.. code-block:: cpp

   #include <gagent/core/AgentFactory.hpp>
   #include <thread>

   int main() {
       // Enregistrer le type
       AgentFactory::instance().registerType(
           "AgentMobile",
           []() -> Agent* { return new AgentMobile(); }
       );

       // Démarrer le serveur de migration en arrière-plan (port 40016 par défaut)
       std::thread([] {
           AgentFactory::instance().startMigrationServer();
       }).detach();

       // Créer et lancer l'agent normalement
       AgentMobile ag;
       ag.init();

       AgentCore::syncAgentSystem();
       return 0;
   }

----

Étape 3 — Déclencher la migration depuis un behaviour
------------------------------------------------------

Depuis n'importe quel behaviour, appelez ``doMove(target)`` :

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>

   class MissionBehaviour : public Behaviour {
       int etapes_ = 0;
   public:
       MissionBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           etapes_++;
           this_agent->setAttribut("etapes", std::to_string(etapes_));
           std::cout << "Étape " << etapes_ << " terminée\n";

           if (etapes_ >= 3) {
               std::cout << "Migration vers 192.168.1.20...\n";
               // doMove suspend les behaviours, sérialise les attributs,
               // contacte la destination et se supprime si succès.
               this_agent->doMove(Agent::MigrationTarget{"192.168.1.20"});
           }
       }

       bool done() override { return false; }  // la migration fait _exit(0)
   };

----

Exemple complet — agent nomade ping/pong
-----------------------------------------

Deux machines : ``192.168.1.10`` (A) et ``192.168.1.20`` (B).
L'agent démarre sur A, migre vers B après 3 itérations, puis termine.

**Code commun (même binaire déployé sur A et B) :**

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/AgentCore.hpp>
   #include <gagent/core/AgentFactory.hpp>
   #include <iostream>
   #include <thread>

   class BehaviourNomade : public Behaviour {
       int tours_ = 0;
   public:
       BehaviourNomade(Agent* ag) : Behaviour(ag) {}

       void onStart() override {
           // Récupérer le compteur depuis les attributs migrés
           std::string s = this_agent->getAttribut("tours");
           if (!s.empty()) tours_ = std::stoi(s);
           std::cout << "[" << this_agent->getAgentId().getAgentName()
                     << "] Démarré, tours = " << tours_ << "\n";
       }

       void action() override {
           tours_++;
           this_agent->setAttribut("tours", std::to_string(tours_));
           std::cout << "[agent] Tour " << tours_ << "\n";
           sleep(1);

           if (tours_ == 3) {
               std::cout << "[agent] Migration vers B...\n";
               this_agent->doMove(Agent::MigrationTarget{"192.168.1.20"});
           }
       }

       bool done() override { return tours_ >= 6; }
       void onEnd() override { this_agent->doDelete(); }
   };

   class AgentNomade : public Agent {
   public:
       std::string agentTypeName() const override { return "AgentNomade"; }

       void setup() override {
           addAttribut("tours");
           addBehaviour(new BehaviourNomade(this));
       }
   };

   int main() {
       AgentFactory::instance().registerType(
           "AgentNomade", []() -> Agent* { return new AgentNomade(); });

       std::thread([] {
           AgentFactory::instance().startMigrationServer();
       }).detach();

       // Sur machine A uniquement : créer l'agent initial
       AgentNomade ag;
       ag.setAgentName("nomade");
       ag.init();

       AgentCore::syncAgentSystem();
       return 0;
   }

**Lancement :**

.. code-block:: bash

   # Sur B (démarrer d'abord le serveur de réception) :
   ./nomade_agent   # le serveur de migration écoute sur :40016

   # Sur A (créer l'agent) :
   ./nomade_agent   # l'agent démarre, migre vers B après 3 tours

----

Attributs et migration
-----------------------

Seuls les attributs déclarés avec ``addAttribut()`` sont transférés.
Déclarez-les dans ``setup()`` avant d'utiliser ``setAttribut()`` :

.. code-block:: cpp

   void setup() override {
       addAttribut("position");   // déclarer
       addAttribut("compteur");

       // Lire la valeur migrée (vide si premier démarrage)
       std::string pos = getAttribut("position");
       if (pos.empty()) pos = "depart";

       setAttribut("position", pos);
       // ...
   }

----

Limitations
-----------

- L'état **interne** des behaviours (variables locales, compteurs) n'est pas
  migré automatiquement — passez-le via les attributs comme dans l'exemple.
- La classe de l'agent doit être **compilée et enregistrée** sur la machine
  cible.
- En cas d'échec de connexion vers la destination, la migration est annulée
  et l'agent reprend son exécution normalement.
- Port 40016 (``control_port + 1``) doit être ouvert entre les machines.
