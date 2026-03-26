21 — Logging et débogage
=========================

Déboguer un système multi-agent est plus difficile qu'un programme
classique : plusieurs processus s'exécutent en parallèle, les messages
circulent de façon asynchrone, et un problème peut venir d'un timing,
d'un message perdu, ou d'un état inattendu. gAgent fournit plusieurs
outils pour comprendre ce qui se passe.

Les deux modes de log
-----------------------

gAgent dispose de deux systèmes de logs indépendants :

.. list-table::
   :widths: 20 30 50
   :header-rows: 1

   * - Mode
     - Activation
     - Format
   * - **Texte**
     - Toujours actif (stdout)
     - ``2026-03-24 14:32:05.123 [INFO ] message``
   * - **JSON Lines**
     - Variable ``GAGENT_LOG=fichier.jsonl``
     - Un objet JSON par ligne (structuré, filtrable)

Le mode JSON Lines est celui à utiliser pour analyser sérieusement
un problème. Le mode texte suffit pour les logs en cours de développement.

Activer les logs JSON
----------------------

.. code-block:: bash

   GAGENT_LOG=gagent.jsonl ./build/ma_simulation

Chaque message ACL envoyé ou reçu, chaque démarrage/arrêt d'agent,
chaque transition d'état est automatiquement enregistré. Si
``GAGENT_LOG`` n'est pas défini, aucun fichier n'est créé (pas
d'overhead en production).

Ce qui est enregistré automatiquement
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Événement
     - Contenu
   * - ``agent_start``
     - Nom de l'agent + PID au démarrage
   * - ``agent_stop``
     - Nom de l'agent + PID à l'arrêt
   * - ``agent_lifecycle``
     - Transitions d'état : ``active``, ``suspended``, ``waiting``…
   * - ``acl_send``
     - Expéditeur, destinataire, performative, conversation_id, contenu (120 car. max)
   * - ``acl_recv``
     - Idem, à la réception

Exemple de log JSON :

.. code-block:: json

   {"ts":"2026-03-24T14:32:05.001Z","event":"agent_start","agent":"coordinateur","pid":"12347"}
   {"ts":"2026-03-24T14:32:05.012Z","event":"acl_send","from":"coordinateur","to":"transporteur-a","perf":"cfp","conv":"cnp-coord-75727","content":"livraison Paris→Lyon"}
   {"ts":"2026-03-24T14:32:05.045Z","event":"acl_recv","to":"transporteur-a","from":"coordinateur","perf":"cfp","conv":"cnp-coord-75727","content":"livraison Paris→Lyon"}
   {"ts":"2026-03-24T14:32:05.089Z","event":"acl_send","from":"transporteur-a","to":"coordinateur","perf":"propose","conv":"cnp-coord-75727","content":"95"}

Ajouter ses propres événements
--------------------------------

Vous pouvez émettre vos propres lignes JSON depuis n'importe quel
agent ou behaviour :

.. code-block:: cpp

   #include <gagent/utils/Logger.hpp>

   // Macro (recommandée)
   LOG_JSON("decision",
       {"agent",   "coordinateur"},
       {"gagnant", meilleur_candidat},
       {"prix",    std::to_string(meilleur_prix)}
   );

   // Log texte classique
   LOG_INFO("décision prise : " + meilleur_candidat);
   LOG_WARNING("aucune proposition reçue");

Les niveaux disponibles : ``LOG_DEBUG``, ``LOG_INFO``, ``LOG_WARNING``,
``LOG_ERROR``, ``LOG_CRITICAL``.

Analyser les logs avec jq
--------------------------

``jq`` est l'outil de référence pour interroger des fichiers JSON Lines.

**Suivre les messages en temps réel :**

.. code-block:: bash

   tail -f gagent.jsonl | jq .

**Voir tous les messages d'une conversation :**

.. code-block:: bash

   jq 'select(.conv == "cnp-coord-75727")' gagent.jsonl

**Reconstituer la chronologie d'un agent :**

.. code-block:: bash

   jq 'select(.agent == "coordinateur" or .from == "coordinateur" or .to == "coordinateur")' gagent.jsonl

**Compter les messages par performative :**

.. code-block:: bash

   jq -r '.perf // empty' gagent.jsonl | sort | uniq -c | sort -rn

.. code-block:: text

      8 inform
      4 cfp
      4 propose
      2 accept-proposal
      1 reject-proposal

**Détecter les timeouts** (messages envoyés sans réponse) :

.. code-block:: bash

   # Lister les CFP sans PROPOSE correspondant
   jq 'select(.event=="acl_send" and .perf=="cfp") | .conv' gagent.jsonl

Puis vérifier qu'il existe bien un ``propose`` pour chaque ``conv``.

Stratégies de débogage courantes
----------------------------------

**Problème : un agent ne reçoit jamais de message**

1. Vérifiez que l'agent appelle ``acl_bind("nom")`` dans ``setup()``
   avant tout ``acl_receive()``.
2. Vérifiez dans les logs que ``acl_send`` vers ce nom a bien été émis.
3. Vérifiez que le nom dans ``acl_send("nom", ...)`` est identique au
   nom dans ``acl_bind("nom")`` et ``acl_receive("nom", ...)``.

**Problème : les messages arrivent dans le mauvais ordre**

Les agents sont des processus parallèles — l'ordre d'exécution n'est
pas garanti. Utilisez les ``conversation_id`` pour corréler les
échanges, et les timeouts pour gérer les cas où un message est en retard.

**Problème : un agent se bloque**

.. code-block:: bash

   # Voir l'état de tous les agents
   ./bin/agentmanager watch

Si un agent est en état ``waiting`` depuis longtemps, il est probablement
bloqué sur un ``acl_receive()`` sans message entrant. Vérifiez :

- Que l'expéditeur a bien envoyé son message (``acl_send`` dans les logs).
- Que le timeout de ``acl_receive()`` est suffisamment long.
- Que les noms d'agents correspondent des deux côtés.

**Problème : messages perdus au démarrage**

C'est le problème du *slow joiner* : un agent envoie un message avant
que le destinataire ait eu le temps de binder son socket. Solution :
appelez ``acl_bind("nom")`` en tout début de ``setup()``, avant même
d'ajouter des behaviours.

.. code-block:: cpp

   void setup() override {
       acl_bind("mon-agent");   // ← en premier
       addBehaviour(new MonBehaviour(this));
   }

Utiliser agentmanager en complément
-------------------------------------

Pendant l'exécution, ``agentmanager watch`` vous donne une vue en temps
réel des agents et de leur état. Combinez-le avec les logs JSON pour
une vision complète :

.. code-block:: bash

   # Terminal 1 — logs en temps réel
   tail -f gagent.jsonl | jq 'select(.event | startswith("acl"))'

   # Terminal 2 — état des agents
   ./bin/agentmanager watch 500

   # Terminal 3 — votre simulation
   GAGENT_LOG=gagent.jsonl ./build/ma_simulation
