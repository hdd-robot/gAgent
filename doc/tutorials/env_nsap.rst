14 — Les NSAPs : capturer et rejouer l'état
============================================

Un **NSAP** (*Network Service Access Point*) est, dans gAgent, un
**snapshot de l'état complet de l'environnement** à un instant donné.
C'est une photographie : la position, la couleur et tous les attributs
de chaque agent, horodatés et empilés en mémoire.

À quoi ça sert ?
-----------------

- **Déboguer une simulation** — vous capturez l'état juste avant un
  événement anormal, puis vous le restaurez pour rejouer la scène.
- **Analyser des instants clés** — comparer l'état à t=0, t=30s,
  t=60s sans avoir à tout relancer.
- **Implémenter un "undo"** — permettre à une simulation interactive
  de revenir à un état précédent.

La pile NSAP
-------------

Les snapshots sont organisés en **pile LIFO** (Last In, First Out).
Trois opérations sont disponibles :

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Méthode
     - Rôle
   * - ``push_nsap()``
     - Empile l'état courant. Retourne le numéro de séquence du snapshot.
   * - ``pull_nsap()``
     - Dépile et **restaure** le snapshot le plus récent. Retourne la
       taille restante de la pile, ou ``-1`` si elle était vide.
   * - ``clear_nsap()``
     - Vide toute la pile sans modifier l'état courant.
   * - ``get_nsaps()``
     - Retourne l'index de la pile : ``{numéro → timestamp}``.

Exemple — capturer et restaurer
---------------------------------

Voici un environnement qui capture un snapshot toutes les 5 secondes,
et restaure le premier en fin de simulation :

.. code-block:: cpp

   class EnvAvecNsap : public Environnement {
       int tick_ = 0;
   public:
       void init_env() override {
           map_width  = 600;
           map_height = 300;
       }

       void link_attribut() override {
           link_id("id");  link_name("name");
           link_pos_x("x"); link_pos_y("y");
           link_shape("shape"); link_color("color");
           link_size_x("size_x"); link_size_y("size_y");
       }

       void event_loop() override {
           while (true) {
               make_agent();

               // Capturer un snapshot toutes les 5 secondes
               tick_++;
               if (tick_ % 50 == 0) {  // 50 × 100ms = 5s
                   int n = push_nsap();
                   std::cout << "[Env] snapshot #" << n
                             << " capturé" << std::endl;
               }

               std::this_thread::sleep_for(std::chrono::milliseconds(100));
           }
       }
   };

Afficher l'index des snapshots
--------------------------------

Vous pouvez consulter les snapshots disponibles à tout moment :

.. code-block:: cpp

   auto* index = env.get_nsaps();
   for (auto& [seq, timestamp] : *index) {
       std::cout << "Snapshot #" << seq
                 << " @ " << timestamp << std::endl;
   }

Résultat typique :

.. code-block:: text

   Snapshot #0 @ 2026-03-24T14:32:05.123
   Snapshot #1 @ 2026-03-24T14:32:10.234
   Snapshot #2 @ 2026-03-24T14:32:15.345

Restaurer un état précédent
-----------------------------

``pull_nsap()`` restaure le snapshot le plus récent de la pile (LIFO)
et le retire :

.. code-block:: cpp

   int reste = env.pull_nsap();
   if (reste >= 0) {
       std::cout << "État restauré. "
                 << reste << " snapshot(s) restant(s)." << std::endl;
   } else {
       std::cout << "Aucun snapshot disponible." << std::endl;
   }

Après ``pull_nsap()``, ``make_agent()`` reconstruira la liste des agents
visuels à partir de l'état restauré — ``agentview`` affichera alors la
carte telle qu'elle était au moment du snapshot.

Consulter les NSAPs depuis agentview
--------------------------------------

``agentview`` peut aussi interroger la pile NSAP via la commande
``GET_NSAP`` sur le socket de l'environnement. La réponse JSON :

.. code-block:: json

   {
     "count": 3,
     "snaps": [
       {"seq": 0, "timestamp": "2026-03-24T14:32:05.123"},
       {"seq": 1, "timestamp": "2026-03-24T14:32:10.234"},
       {"seq": 2, "timestamp": "2026-03-24T14:32:15.345"}
     ]
   }

Résumé
-------

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Opération
     - Effet
   * - ``push_nsap()``
     - Sauvegarde l'état courant dans la pile
   * - ``pull_nsap()``
     - Restaure et retire le snapshot le plus récent
   * - ``clear_nsap()``
     - Vide toute la pile
   * - ``get_nsaps()``
     - Retourne l'index ``{seq → timestamp}``
