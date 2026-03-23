Plateforme gAgent
=================

La plateforme gAgent est un **daemon FIPA** composé de trois services
regroupés dans un seul binaire (``gAgentPlatform``) lancé sous trois
noms différents via des symlinks.

.. code-block:: text

   gAgentPlatform  (binaire)
     ├── agentplatform  →  AMS + DF  (registre agents et services)
     ├── agentmanager   →  CLI de gestion en temps réel
     └── agentmonitor   →  affichage des logs UDP des agents


Démarrage
---------

Lancer la plateforme **avant** les agents :

.. code-block:: bash

   # Terminal dédié (ou en arrière-plan avec &)
   ./agentplatform

   # Résultat :
   # [Platform] AMS + DF démarrés
   # [AMS] socket Unix prêt : /tmp/gagent_ams.sock
   # [DF]  socket Unix prêt : /tmp/gagent_df.sock
   # [Platform] en attente (Ctrl+C pour arrêter)...

Arrêt propre avec ``Ctrl+C`` ou ``kill <pid>``.

Configuration réseau
~~~~~~~~~~~~~~~~~~~~

Le fichier ``config.cfg`` (dans le répertoire courant) surcharge les
valeurs par défaut :

.. code-block:: text

   plt_address = "127.0.0.1";   # AMS + DF
   plt_port    = "40011";
   mng_address = "127.0.0.1";   # agentmanager (réservé Phase 2)
   mng_port    = "40012";
   mon_address = "127.0.0.1";   # agentmonitor UDP
   mon_port    = "40013";

Les chemins des sockets Unix sont surchargeables via variables
d'environnement :

.. code-block:: bash

   export GAGENT_AMS_SOCK=/run/gagent/ams.sock
   export GAGENT_DF_SOCK=/run/gagent/df.sock
   ./agentplatform

Commandes interactives
~~~~~~~~~~~~~~~~~~~~~~

Depuis le terminal où ``agentplatform`` tourne, on peut taper :

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - Commande
     - Effet
   * - ``dump``
     - Affiche le contenu de l'AMS et du DF
   * - ``quit``
     - Arrêt propre de la plateforme


AMS — Agent Management System
------------------------------

L'AMS est le **registre central** de la plateforme. Chaque agent doit
s'y enregistrer avant de pouvoir communiquer. L'enregistrement est
automatique dans ``Agent::_init()``.

Protocole (socket Unix ``/tmp/gagent_ams.sock``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   → REGISTER <nom> <pid> <adresse_mq>
   ← OK  |  ERROR already_registered

   → DEREGISTER <nom>
   ← OK  |  ERROR not_found

   → LOOKUP <nom>
   ← OK <nom> <pid> <adresse_mq> <état>  |  ERROR not_found

   → SETSTATE <nom> <état>
   ← OK  |  ERROR not_found

   → LIST
   ← OK <n>
   ← <nom> <pid> <adresse_mq> <état>
   ← ...

États valides : ``active`` | ``suspended`` | ``waiting`` | ``deleted``

Exemple Python (test/script)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: python

   import socket

   def ams(cmd):
       s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
       s.connect('/tmp/gagent_ams.sock')
       s.sendall((cmd + '\n').encode())
       data = b''
       s.settimeout(0.5)
       try:
           while True:
               chunk = s.recv(256)
               if not chunk: break
               data += chunk
       except: pass
       s.close()
       return data.decode().strip()

   print(ams('LIST'))
   print(ams('LOOKUP alice'))
   print(ams('SETSTATE alice suspended'))


DF — Directory Facilitator
--------------------------

Le DF est l'**annuaire des services**. Un agent y publie les services
qu'il offre ; d'autres agents peuvent les découvrir par type ou
ontologie.

Protocole (socket Unix ``/tmp/gagent_df.sock``)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   → REGISTER <agent> <type> <nom_svc> <langage> <ontologie> <protocole> <ownership>
   ← OK

   → DEREGISTER <agent>
   ← OK

   → SEARCH <type>
   ← OK <n>
   ← <agent> <type> <nom_svc> <langage> <ontologie> <protocole> <ownership>
   ← ...

   → SEARCH_ONT <type> <ontologie>
   ← OK <n>
   ← ...

   → SEARCH *
   ← OK <n>          # joker : retourne tous les services enregistrés
   ← ...

Utiliser ``-`` pour les champs optionnels vides (ownership, protocole…).

Intégration C++ (côté agent)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   #include <gagent/platform/DFClient.hpp>

   // Dans setup() :
   gagent::platform::DFClient df;
   df.registerService(
       agentId.getAgentName(),   // nom de l'agent
       "planning",               // type de service
       "mon-planificateur",      // nom du service
       "fipa-sl",                // langage
       "logistics"               // ontologie
   );

   // Depuis n'importe quel agent, chercher un planificateur :
   auto services = df.search("planning");
   auto services = df.search("planning", "logistics");  // + filtre ontologie


agentmanager — CLI de gestion
------------------------------

``agentmanager`` se connecte à l'AMS et au DF pour lister et contrôler
les agents en temps réel.

Référence des commandes
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: bash

   agentmanager help                          # cette aide

   # Consultation
   agentmanager list                          # liste les agents (AMS)
   agentmanager watch                         # live, rafraîchi toutes les 1 s
   agentmanager watch 500                     # rafraîchi toutes les 500 ms
   agentmanager df search <type>              # cherche un service par type
   agentmanager df search <type> <ontologie>  # + filtre ontologie

   # Contrôle
   agentmanager kill    <nom>                 # supprime l'agent
   agentmanager suspend <nom>                 # suspend (behaviours figés)
   agentmanager wake    <nom>                 # réveille

Sortie de ``agentmanager list``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   [AMS] 3 agent(s)
   NOM                 PID     ADRESSE MQ        ÉTAT
   ----------------------------------------------------------
   alice               4521    /acl_alice        active       ← vert
   bob                 4522    /acl_bob          active       ← vert
   carol               4523    /acl_carol        suspended    ← jaune

Les états sont colorés : vert (active), jaune (suspended), cyan
(waiting), rouge (deleted).

Mode ``watch``
~~~~~~~~~~~~~~

``agentmanager watch`` rafraîchit l'écran en continu, utile dans un
coin de tmux lors d'une simulation :

.. code-block:: text

   gAgent — 14:32:07  (Ctrl+C pour quitter)

   Agents (3)
   NOM                 PID     ADRESSE MQ        ÉTAT
   ----------------------------------------------------------
   alice               4521    /acl_alice        active
   bob                 4522    /acl_bob          suspended
   carol               4523    /acl_carol        waiting

   Services DF (2)
   AGENT               TYPE            SERVICE             LANGAGE     ONTOLOGIE
   ------------------------------------------------------------------------------
   alice               planning        mon-planificateur   fipa-sl     logistics
   bob                 translation     traducteur          fipa-sl     nlp

Contrôle des agents
~~~~~~~~~~~~~~~~~~~~

``kill``, ``suspend`` et ``wake`` récupèrent le PID de l'agent dans
l'AMS, puis envoient le signal POSIX RT correspondant :

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Commande
     - Signal
     - Effet
   * - ``kill``
     - ``SIG_AGENT_DELETE`` (SIGRTMIN+2)
     - Désenregistrement AMS/DF + ``_exit(0)``
   * - ``suspend``
     - ``SIG_AGENT_SUSPEND`` (SIGRTMIN+4)
     - Tous les behaviours se figent
   * - ``wake``
     - ``SIG_AGENT_WAKE`` (SIGRTMIN+5)
     - Behaviours reprennent


agentmonitor — logs UDP
------------------------

``agentmonitor`` écoute sur le port UDP 40013 et affiche les messages
envoyés par ``sendMsgMonitor()`` depuis chaque agent.

.. code-block:: bash

   ./agentmonitor [--ip 127.0.0.1] [--port 40013]

.. code-block:: text

   [Monitor] écoute sur 127.0.0.1:40013
   00000001 : alice -> Start agent PID : 4521
   00000002 : bob -> Start agent PID : 4522
   00000003 : alice -> mon message de log
   00000004 : alice -> Stop agent PID : 4521

Depuis le code agent :

.. code-block:: cpp

   sendMsgMonitor("calcul terminé en 42ms");
   // → "00000005 : alice -> calcul terminé en 42ms"


Mode dégradé
------------

Si ``agentplatform`` n'est pas lancé, les agents fonctionnent quand même :

- Les behaviours s'exécutent normalement
- ``agentStatus`` reste ``AGENT_INITED`` (pas de passage à ``AGENT_ACTIVE``)
- ``AMSClient`` et ``DFClient`` retournent silencieusement sans erreur fatale
- Un avertissement s'affiche dans ``stderr`` :

.. code-block:: text

   [AMSClient] plateforme non disponible, agent alice non enregistré

Les fonctions de découverte (``lookup``, ``df.search``) retournent des
résultats vides.
