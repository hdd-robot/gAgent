Cycle de vie d'un agent
========================

Un agent gAgent passe par une série d'**états** bien définis, du
démarrage jusqu'à la suppression. Comprendre ces états est essentiel
pour concevoir des agents robustes.

Les états d'un agent
---------------------

.. code-block:: text

   ┌─────────┐   init()    ┌────────┐  doActivate()  ┌────────┐
   │ CREATED │────────────►│ INITED │───────────────►│ ACTIVE │
   └─────────┘             └────────┘                └────┬───┘
                                                          │
                    ┌─────────────────────────────────────┤
                    │                                     │
               doSuspend()                           doWait()
                    │                                     │
                    ▼                                     ▼
             ┌───────────┐                         ┌─────────┐
             │ SUSPENDED │                         │ WAITING │
             └─────┬─────┘                         └────┬────┘
                   │  doWake()                          │  doWake()
                   └──────────────┐    ┌────────────────┘
                                  ▼    ▼
                               ┌────────┐  doDelete()  ┌─────────┐
                               │ ACTIVE │─────────────►│ DELETED │
                               └────────┘              └─────────┘

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - État
     - Signification
   * - ``INITED``
     - Agent créé, behaviours démarrés, en attente d'enregistrement AMS
   * - ``ACTIVE``
     - Agent enregistré, toutes ses activités tournent normalement
   * - ``SUSPENDED``
     - Tous les behaviours sont mis en pause (conservent leur état interne)
   * - ``WAITING``
     - L'agent attend un événement extérieur avant de reprendre
   * - ``TRANSIT``
     - L'agent est en cours de migration vers une autre machine
   * - ``DELETED``
     - L'agent a terminé son cycle de vie, ses ressources sont libérées

Suspendre et reprendre
-----------------------

``doSuspend()`` met en pause tous les behaviours de l'agent. Ils ne
consomment plus de ressources mais conservent leur état interne. Utile
pour "mettre sur pause" un agent le temps qu'une condition externe
soit remplie.

``doWake()`` reprend l'exécution depuis l'état où elle avait été
suspendue.

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/core/AgentCore.hpp>
   using namespace gagent;

   class TravailPeriodique : public TickerBehaviour {
       int compteur_ = 0;
   public:
       TravailPeriodique(Agent* ag)
           : TickerBehaviour(ag, 1000) {}   // tick toutes les secondes

       void onTick() override {
           compteur_++;
           std::cout << "[Agent] tick n°" << compteur_ << "\n";

           if (compteur_ == 3) {
               std::cout << "[Agent] je me suspends 5 secondes\n";
               this_agent->doSuspend();

               // Dans un vrai système : un autre thread ou agent
               // appellera doWake() 5 secondes plus tard.
               // Ici on simule avec un thread séparé :
               std::thread([ag = this_agent]() {
                   std::this_thread::sleep_for(std::chrono::seconds(5));
                   std::cout << "[External] réveil de l'agent\n";
                   ag->doWake();
               }).detach();
           }

           if (compteur_ == 6) {
               std::cout << "[Agent] travail terminé, suppression\n";
               this_agent->doDelete();
           }
       }
   };

   class MonAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new TravailPeriodique(this));
       }
   };

Résultat attendu :

.. code-block:: text

   [Agent] tick n°1
   [Agent] tick n°2
   [Agent] tick n°3
   [Agent] je me suspends 5 secondes
   [External] réveil de l'agent
   [Agent] tick n°4
   [Agent] tick n°5
   [Agent] tick n°6
   [Agent] travail terminé, suppression

Attendre un événement avec ``doWait()``
-----------------------------------------

``doWait()`` est sémantiquement proche de ``doSuspend()`` mais signale
explicitement que l'agent **attend un événement** (une réponse, une
ressource disponible, un verrou libéré). Un autre agent peut alors
envoyer un message de réveil.

.. code-block:: cpp

   class AgentCoordinateur : public Agent {
   public:
       void setup() override {
           addBehaviour(new AttenteDeFichier(this));
       }
   };

   class AttenteDeFichier : public CyclicBehaviour {
       bool fichier_disponible_ = false;
   public:
       AttenteDeFichier(Agent* ag) : CyclicBehaviour(ag) {}

       void action() override {
           if (!fichier_disponible_) {
               std::cout << "[Coord] en attente du fichier...\n";
               this_agent->doWait();
               // L'exécution reprend ici quand doWake() est appelé
           }
           std::cout << "[Coord] fichier disponible, traitement\n";
           fichier_disponible_ = false; // réinitialiser pour prochaine fois
       }
   };

Supprimer un agent avec ``doDelete()``
----------------------------------------

``doDelete()`` termine l'agent proprement : les behaviours reçoivent
leurs ``onEnd()``, ``takeDown()`` est appelé, puis le processus se
termine.

Toujours libérer les ressources dans ``takeDown()`` avant la
suppression :

.. code-block:: cpp

   #include <gagent/messaging/AclMQ.hpp>
   using namespace gagent::messaging;

   class MonAgent : public Agent {
   public:
       void setup() override {
           setAgentName("mon-agent");
           addBehaviour(new MonBehaviour(this));
       }

       void takeDown() override {
           // Libérer la queue de messagerie
           acl_unlink("mon-agent");

           // Désenregistrer du DF si l'agent avait annoncé des services
           // DFClient df; df.deregisterAgent("mon-agent");

           std::cout << "[MonAgent] ressources libérées\n";
       }
   };

Lire l'état courant
--------------------

L'état courant est accessible via ``agentStatus`` :

.. code-block:: cpp

   void action() override {
       if (this_agent->agentStatus == Agent::AGENT_ACTIVE) {
           // agent actif, traitement normal
       } else if (this_agent->agentStatus == Agent::AGENT_SUSPENDED) {
           // ne devrait pas arriver (behaviours suspendus)
       }
   }

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Constante
     - Valeur
   * - ``Agent::AGENT_INITED``
     - Agent démarré, non encore enregistré AMS
   * - ``Agent::AGENT_ACTIVE``
     - Actif et enregistré
   * - ``Agent::AGENT_SUSPENDED``
     - Tous behaviours en pause
   * - ``Agent::AGENT_WAITING``
     - En attente d'un événement
   * - ``Agent::AGENT_TRANSIT``
     - En cours de migration
   * - ``Agent::AGENT_DELETED``
     - Supprimé

Contrôle externe via ``agentmanager``
---------------------------------------

Un agent peut aussi être suspendu/réveillé depuis la ligne de commande
sans modifier son code (voir tutoriel :doc:`platform_outils`) :

.. code-block:: bash

   # Suspendre un agent
   agentmanager suspend mon-agent

   # Le réveiller
   agentmanager wake mon-agent

   # Le supprimer
   agentmanager kill mon-agent

Résumé
-------

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Méthode
     - Effet
   * - ``doSuspend()``
     - Pause tous les behaviours (état conservé)
   * - ``doWait()``
     - Pause en signalant l'attente d'un événement
   * - ``doWake()``
     - Reprend depuis SUSPENDED ou WAITING
   * - ``doDelete()``
     - Arrêt propre : takeDown() puis fin du processus
   * - ``doActivate()``
     - Réactive un agent INITED (normalement automatique)
