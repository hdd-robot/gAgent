Cycle de vie d'un agent
=======================

Chaque agent gAgent suit un cycle de vie conforme à la spécification
**FIPA Agent Management** (FIPA SC00023J). Ce cycle comprend neuf états
et les transitions entre eux sont déclenchées soit par l'agent lui-même,
soit par un signal externe depuis un autre processus.

États
-----

.. list-table::
   :header-rows: 1
   :widths: 15 10 75

   * - Constante
     - Valeur
     - Description
   * - ``AGENT_UNKNOWN``
     - 0
     - État initial avant toute construction. L'agent n'existe pas encore.
   * - ``AGENT_CREATED``
     - 1
     - L'objet C++ est construit, l'``AgentID`` (identifiant aléatoire 8 caractères) est généré.
   * - ``AGENT_INITED``
     - 2
     - ``init()`` a été appelé. Le processus enfant a été créé (``fork()``), les threads internes démarrent. L'agent n'est pas encore enregistré auprès de l'AMS.
   * - ``AGENT_ACTIVE``
     - 3
     - L'agent est enregistré auprès de l'AMS. Tous ses ``Behaviour`` s'exécutent en parallèle. C'est l'état de travail normal.
   * - ``AGENT_SUSPENDED``
     - 4
     - Tous les behaviours sont figés. L'agent reste en mémoire mais n'exécute rien. Il peut être réactivé via ``doActivate()``.
   * - ``AGENT_WAITING``
     - 6
     - Pause temporaire, typiquement en attendant un message ou une ressource. Réveillable par ``doWake()``.
   * - ``AGENT_WAKING``
     - 8
     - État transitoire entre WAITING et ACTIVE : les behaviours reprennent, l'agent redevient actif.
   * - ``AGENT_TRANSIT``
     - 5
     - L'agent migre vers une autre station (Phase 2). Tous ses behaviours sont suspendus le temps de la migration.
   * - ``AGENT_DELETED``
     - 7
     - L'agent a terminé son travail, s'est désenregistré de l'AMS et du DF, et son processus a appelé ``_exit(0)``.

Diagramme d'états
-----------------

.. graphviz::

   digraph agent_lifecycle {
       rankdir=TB;
       node [shape=roundrectangle, style=filled, fontname="Helvetica", fontsize=11];
       edge [fontname="Helvetica", fontsize=10];

       UNKNOWN   [label="UNKNOWN\n(0)",   fillcolor="#e0e0e0"];
       CREATED   [label="CREATED\n(1)",   fillcolor="#cce5ff"];
       INITED    [label="INITED\n(2)",    fillcolor="#cce5ff"];
       ACTIVE    [label="ACTIVE\n(3)",    fillcolor="#c3e6cb", shape=doublecircle];
       SUSPENDED [label="SUSPENDED\n(4)", fillcolor="#fff3cd"];
       TRANSIT   [label="TRANSIT\n(5)",   fillcolor="#ffeeba"];
       WAITING   [label="WAITING\n(6)",   fillcolor="#fff3cd"];
       WAKING    [label="WAKING\n(8)",    fillcolor="#d4edda"];
       DELETED   [label="DELETED\n(7)",   fillcolor="#f8d7da", shape=doublecircle];

       UNKNOWN   -> CREATED   [label="constructeur"];
       CREATED   -> INITED    [label="init() / fork()"];
       INITED    -> ACTIVE    [label="AMS::register()"];

       ACTIVE    -> SUSPENDED [label="doSuspend()"];
       SUSPENDED -> ACTIVE    [label="doActivate()"];

       ACTIVE    -> WAITING   [label="doWait()"];
       SUSPENDED -> WAITING   [label="doWait()"];
       WAITING   -> WAKING    [label="doWake()"];
       WAKING    -> ACTIVE    [label="(automatique)"];

       ACTIVE    -> TRANSIT   [label="doMove()"];
       TRANSIT   -> ACTIVE    [label="(arrivée)"];

       ACTIVE    -> DELETED   [label="doDelete()\nou fin behaviours"];
       SUSPENDED -> DELETED   [label="doDelete()"];
       WAITING   -> DELETED   [label="doDelete()"];
       TRANSIT   -> DELETED   [label="doDelete()"];
   }

Transitions et méthodes
-----------------------

.. list-table::
   :header-rows: 1
   :widths: 30 20 50

   * - Méthode / Signal
     - Transition
     - Signal POSIX RT envoyé (si externe)
   * - ``doActivate()``
     - → ACTIVE
     - ``SIG_AGENT_ACTIVE`` (``SIGRTMIN+3``)
   * - ``doSuspend()``
     - → SUSPENDED
     - ``SIG_AGENT_SUSPEND`` (``SIGRTMIN+4``)
   * - ``doWait()``
     - → WAITING
     - ``SIG_AGENT_WAIT`` (``SIGRTMIN+6``)
   * - ``doWake()``
     - → WAKING → ACTIVE
     - ``SIG_AGENT_WAKE`` (``SIGRTMIN+5``)
   * - ``doMove()``
     - → TRANSIT
     - ``SIG_AGENT_TRANSIT`` (``SIGRTMIN+7``)
   * - ``doDelete()``
     - → DELETED
     - ``SIG_AGENT_DELETE`` (``SIGRTMIN+2``)

Les transitions sont **locales** si la méthode est appelée depuis l'intérieur
du processus de l'agent, ou **par signal** si appelée depuis l'extérieur
(processus parent ou autre agent).

Cycle complet d'exécution
--------------------------

Voici ce qui se passe exactement quand ``agent.init()`` est appelé :

.. code-block:: text

   [PARENT]  agent.init()
               └─ fork()
               └─ retourne immédiatement

   [CHILD]   _init()
               ├─ agentStatus = INITED
               ├─ thread : listener_extern_signals_Thread()
               │             └─ écoute SIG_AGENT_DELETE / ACTIVE / SUSPEND…
               ├─ thread : control_Thread()
               │             └─ traite les actions (runingThred on/off)
               ├─ thread : control_message()
               │             └─ POSIX MQ /{8-char-id}  (contrôle interne)
               │
               ├─ AMSClient::registerAgent()      → agentStatus = ACTIVE
               │
               ├─ setup()                         ← à implémenter : addBehaviour(...)
               │
               ├─ thread par Behaviour
               │     ├─ onStart()
               │     ├─ loop : action() tant que done() == false
               │     └─ onEnd()
               │
               ├─ join() tous les threads behaviour
               │
               ├─ takeDown()                      ← à implémenter : nettoyage
               │
               └─ doDelete()
                     ├─ AMSClient::deregisterAgent()
                     ├─ DFClient::deregisterAgent()
                     ├─ mq_unlink(/{8-char-id})
                     └─ _exit(0)

Suspension et reprise des behaviours
--------------------------------------

La suspension agit sur un flag partagé ``runingThred`` protégé par un
``std::mutex``. Chaque thread behaviour attend sur une
``std::condition_variable`` :

.. code-block:: cpp

   // Dans exthread() — boucle interne de chaque Behaviour
   while (beh->done() == false) {
       std::unique_lock<std::mutex> lck(mtxInterThred);
       while (!runingThred)
           cvInterThred.wait(lck);   // bloqué si suspendu ou en attente
       lck.unlock();
       beh->action();
   }

Un ``doSuspend()`` met ``runingThred = false`` et notifie la condition :
tous les behaviours se bloquent à la prochaine itération.
Un ``doActivate()`` ou ``doWake()`` remet ``runingThred = true`` et
les débloques simultanément.

Signaux RT et concurrence
--------------------------

Les signaux temps-réel POSIX (``SIGRTMIN+2`` à ``SIGRTMIN+7``) sont
interceptés dans un thread dédié (``listener_extern_signals_Thread``)
via ``boost::asio::signal_set``. Cela garantit qu'ils ne perturbent
pas les threads de behaviour en pleine exécution.

.. warning::

   ``doDelete()`` appelé **localement** (depuis le processus de l'agent
   lui-même) appelle ``_exit(0)`` directement, sans envoyer de signal.
   Ceci évite une boucle infinie : s'envoyer ``SIGTERM`` déclencherait
   le handler de signal qui enverrait ``SIGTERM`` à tout le groupe de
   processus.

Enregistrement FIPA
-------------------

Depuis la version 0.9, l'enregistrement auprès de la plateforme est
automatique si ``agentplatform`` est lancé. Il suffit de démarrer le
daemon avant les agents :

.. code-block:: bash

   # Terminal 1 — lancer la plateforme
   ./agentplatform

   # Terminal 2 — lancer ton application
   ./my_agent_app

Si la plateforme n'est pas disponible, les agents fonctionnent en
**mode dégradé** : les behaviours s'exécutent normalement, mais
``agentStatus`` reste ``AGENT_INITED`` et les fonctions de découverte
(lookup, search DF) ne sont pas disponibles.

Pour vérifier l'état de la plateforme en cours d'exécution :

.. code-block:: bash

   # Dans le terminal de agentplatform, taper :
   dump

   # Résultat exemple :
   # [AMS] registre (2 agents) :
   #   alice  pid=4521  addr=/acl_alice  état=active
   #   bob    pid=4522  addr=/acl_bob    état=active
   # [DF] annuaire (1 service(s)) :
   #   my-planner  type=planning  agent=alice  ontologie=logistics
