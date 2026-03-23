Démarrage rapide
================

Prérequis
---------

.. code-block:: bash

   # Debian/Ubuntu
   sudo apt install cmake g++ libboost-all-dev libconfig++-dev flex bison

Compilation
-----------

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON
   make -j$(nproc)

Options CMake
~~~~~~~~~~~~~

.. list-table::
   :widths: 30 15 55
   :header-rows: 1

   * - Option
     - Défaut
     - Description
   * - ``BUILD_TESTS``
     - ``ON``
     - Compile les tests
   * - ``BUILD_EXAMPLES``
     - ``ON``
     - Compile les exemples
   * - ``BUILD_PLATFORM``
     - ``ON``
     - Compile ``agentplatform`` / ``agentmanager`` / ``agentmonitor``
   * - ``BUILD_VIEW``
     - ``ON``
     - Compile ``agentview`` (visualisation web)

Premier agent
-------------

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/core/AgentCore.hpp>
   #include <gagent/env/Environnement.hpp>
   #include <iostream>

   using namespace gagent;

   // Behaviour : affiche "hello" toutes les secondes, 3 fois
   class HelloBehaviour : public Behaviour {
       int count_ = 0;
   public:
       HelloBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           std::cout << "Hello from agent! (" << ++count_ << "/3)\n";
           sleep(1);
       }

       bool done() override { return count_ >= 3; }
   };

   class HelloAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new HelloBehaviour(this));
       }
   };

   // Environnement minimal
   class MyEnv : public Environnement {
   public:
       void init_env()      override {}
       void link_attribut() override {}
       void event_loop()    override {}
   };

   int main() {
       AgentCore::initAgentSystem();

       HelloAgent agent;
       agent.init();

       AgentCore::syncAgentSystem();
       return 0;
   }

Exemple d'échange ACL
---------------------

Voir :doc:`messaging` pour un exemple complet d'échange REQUEST/INFORM
entre deux agents.

Lancer les tests
----------------

.. code-block:: bash

   cd build/tests
   ./test_gagent         # test de base
   ./two_agents_acl      # échange FIPA ACL Alice ↔ Bob
   ./test_acl            # tests unitaires ACLMessage (66 assertions)

   # Tests d'intégration plateforme (agentplatform doit tourner)
   ../platform/agentplatform &
   ./test_platform       # AMS + DF (24 assertions)
   kill %1

Visualisation web
-----------------

.. code-block:: bash

   # Lancer votre simulation
   ./mon_application &

   # Démarrer le serveur de visualisation
   ./view/agentview       # → http://localhost:8080

Voir :doc:`visualization` pour la documentation complète.
