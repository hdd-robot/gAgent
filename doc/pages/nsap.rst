Snapshots de simulation (NSAP)
==============================

Les **NSAP** (*Numbered Situation Snapshots*) permettent de sauvegarder
l'état complet d'une simulation et d'y revenir à tout moment.

C'est une pile **LIFO** (Last In, First Out) : le dernier état sauvegardé
est le premier restauré.

.. code-block:: text

   Simulation  →  push  →  push  →  push
                  [S0]      [S1]      [S2]   ← sommet de pile
                                      │
                               pull ──┘   → restaure S2, retire de la pile
                        pull ──┘          → restaure S1
                  pull ──┘                → restaure S0
                  pull                    → pile vide, retourne -1

Cas d'usage
-----------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Cas
     - Description
   * - **Backtracking de planification**
     - Essaie un plan, si échec ``pull_nsap()`` et essaie un autre
   * - **Débogage de simulation**
     - Reviens à l'instant T où le comportement est devenu anormal
   * - **Exploration de branches**
     - Depuis un état S, explore plusieurs scénarios en repartant de S
   * - **Replay pas-à-pas**
     - Enregistre chaque étape, rejoue en avant/arrière
   * - **Démonstration pédagogique**
     - Montre une simulation étape par étape avec retour en arrière

API
---

.. code-block:: cpp

   #include <gagent/env/Environnement.hpp>

   Environnement env;

   // Sauvegarde l'état courant
   // Retourne le numéro de séquence du snapshot (= taille de la pile)
   int seq = env.push_nsap();

   // Restaure le snapshot le plus récent (LIFO)
   // Retourne la taille restante, ou -1 si la pile était vide
   int remaining = env.pull_nsap();

   // Liste tous les snapshots disponibles : { seq → timestamp ISO }
   std::map<int, std::string>* index = env.get_nsaps();

   // Vide toute la pile (l'état courant n'est pas modifié)
   env.clear_nsap();

.. note::

   ``push_nsap()`` et ``pull_nsap()`` sont thread-safe : ils peuvent être
   appelés depuis un behaviour d'agent pendant que l'environnement reçoit
   des mises à jour d'autres agents.

Exemple 1 — Retour arrière simple
----------------------------------

.. code-block:: cpp

   // t0 : agent1 à (10, 20)
   // ... agents bougent, l'environnement se met à jour via MQ ...

   env.push_nsap();    // sauvegarde t0

   // t1 : agent1 à (50, 80), agent2 est apparu
   // ...

   env.push_nsap();    // sauvegarde t1

   // On revient à t1
   env.pull_nsap();    // agent1 à (50,80), agent2 présent

   // On revient à t0
   env.pull_nsap();    // agent1 à (10,20), agent2 absent

   // Pile vide
   int r = env.pull_nsap();  // r == -1

Exemple 2 — Backtracking d'un planificateur
--------------------------------------------

Scénario : un agent planificateur tente deux stratégies depuis le même état.

.. code-block:: cpp

   class PlannerBehaviour : public Behaviour {
   public:
       PlannerBehaviour(Agent* ag, Environnement* env)
           : Behaviour(ag), env_(env) {}

       void action() override
       {
           // Sauvegarde l'état avant de tenter le plan A
           env_->push_nsap();
           std::cout << "Tentative plan A...\n";

           bool success = try_plan_A();

           if (!success) {
               std::cout << "Plan A échoué → retour arrière\n";
               env_->pull_nsap();   // restaure l'état initial

               std::cout << "Tentative plan B...\n";
               try_plan_B();
           }
       }

       bool done() override { return true; }

   private:
       Environnement* env_;

       bool try_plan_A() {
           // ... envoie des ACL REQUEST aux agents ...
           // ... retourne false si l'objectif n'est pas atteint
           return false;
       }
       void try_plan_B() {
           // ... stratégie alternative ...
       }
   };

Exemple 3 — Exploration de scénarios (branches)
-------------------------------------------------

Depuis un état pivot, explore N scénarios indépendants.

.. code-block:: cpp

   // Sauvegarde l'état pivot
   env.push_nsap();

   for (int scenario = 0; scenario < 3; scenario++) {
       std::cout << "=== Scénario " << scenario << " ===\n";

       // Simule le scénario
       run_scenario(scenario, env);

       // Affiche les résultats
       env.make_agent();
       for (auto* va : env.list_visual_agents)
           std::cout << va->id << " → (" << va->pos_x << ", " << va->pos_y << ")\n";

       // Retour à l'état pivot pour le prochain scénario
       env.pull_nsap();   // restaure le pivot
       env.push_nsap();   // re-sauvegarde pour la prochaine itération
   }

   env.pull_nsap();  // libère le dernier pivot

Exemple 4 — Replay pas-à-pas
------------------------------

Enregistre chaque étape d'une simulation pour la rejouer.

.. code-block:: cpp

   // Phase 1 : enregistrement
   std::vector<int> steps;
   for (int t = 0; t < 10; t++) {
       simulate_one_step(env);
       steps.push_back(env.push_nsap());
       std::cout << "Step " << t << " sauvegardé\n";
   }

   // Affiche l'index des snapshots
   auto* idx = env.get_nsaps();
   for (auto& [seq, ts] : *idx)
       std::cout << "snap #" << seq << " @ " << ts << "\n";

   // Phase 2 : retour arrière étape par étape
   std::cout << "\n--- Replay en arrière ---\n";
   while (env.pull_nsap() >= 0) {
       env.make_agent();
       std::cout << env.list_visual_agents.size() << " agents\n";
   }

Inspecter la pile
-----------------

.. code-block:: cpp

   auto* idx = env.get_nsaps();
   if (idx->empty()) {
       std::cout << "Pile vide\n";
   } else {
       std::cout << idx->size() << " snapshot(s) :\n";
       for (auto& [seq, ts] : *idx)
           std::cout << "  #" << seq << " → " << ts << "\n";
   }

   // Exemple de sortie :
   //   3 snapshot(s) :
   //   #0 → 2026-03-23T21:18:16.058
   //   #1 → 2026-03-23T21:18:16.139
   //   #2 → 2026-03-23T21:18:17.042

Intégration avec la planification neuro-symbolique
----------------------------------------------------

Les NSAP sont le mécanisme de **backtracking** naturel pour un planificateur
HTN ou PDDL embarqué dans gAgent :

.. code-block:: text

   Planificateur HTN
   │
   ├─ decompose(tâche)  →  génère sous-tâches
   │       │
   │       ├─ push_nsap()           # point de choix
   │       ├─ essaie méthode M1
   │       │       │
   │       │       ├─ succès  →  continue
   │       │       └─ échec   →  pull_nsap()  →  essaie M2
   │       │
   │       └─ toutes méthodes échouées  →  pull_nsap()  →  échec global
   │
   └─ solution trouvée  →  clear_nsap()  →  exécution
