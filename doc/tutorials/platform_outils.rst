19 — agentmanager et agentmonitor
===================================

gAgent fournit deux outils en ligne de commande pour superviser un
système d'agents en cours d'exécution.

Prérequis : ``agentplatform`` doit être lancé (voir section `Ordre de
lancement recommandé`_ ci-dessous pour le mode local et le mode cluster).

.. code-block:: bash

   ./bin/agentplatform                              # mode local
   ./bin/agentplatform --master --ip 192.168.1.10  # mode master (cluster)

agentmanager
-------------

``agentmanager`` est le tableau de bord en ligne de commande. Il permet
de lister les agents, surveiller leur état en temps réel, et agir sur
eux à distance.

**Lister les agents :**

.. code-block:: bash

   ./bin/agentmanager list

.. code-block:: text

   [AMS] 4 agent(s)
   NOM                 PID     ENDPOINT ZMQ                   SLAVE        ÉTAT
   --------------------------------------------------------------------------
   planificateur       12345   ipc:///tmp/acl_planificateur   local        active
   transporteur-a      12346   ipc:///tmp/acl_transporteur-a  local        active
   transporteur-b      12347   ipc:///tmp/acl_transporteur-b  local        active
   coordinateur        12348   ipc:///tmp/acl_coordinateur    local        waiting

En mode cluster, la colonne ``ENDPOINT ZMQ`` affiche des adresses TCP et
la colonne ``SLAVE`` indique l'IP de la machine hébergeant l'agent :

.. code-block:: text

   [AMS] 3 agent(s)
   NOM                 PID     ENDPOINT ZMQ                   SLAVE        ÉTAT
   --------------------------------------------------------------------------
   alice               10101   ipc:///tmp/acl_alice           local        active
   bob                 20202   tcp://192.168.1.20:52345        192.168.1.20 active
   charlie             30303   tcp://192.168.1.21:57891        192.168.1.21 active

L'état d'un agent peut être :

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - État
     - Signification
   * - ``active``
     - L'agent tourne et exécute ses behaviours
   * - ``waiting``
     - L'agent attend un message (bloqué sur ``acl_receive``)
   * - ``suspended``
     - L'agent a été suspendu manuellement
   * - ``deleted``
     - L'agent s'est terminé (en cours de nettoyage)

**Surveillance en temps réel :**

.. code-block:: bash

   ./bin/agentmanager watch        # rafraîchi toutes les secondes
   ./bin/agentmanager watch 500    # rafraîchi toutes les 500 ms

L'écran se met à jour en continu. Vous voyez les agents apparaître au
démarrage et disparaître à l'arrêt. Les services DF enregistrés sont
affichés en dessous.

.. code-block:: text

   gAgent — 14:32:05  (Ctrl+C pour quitter)

   Agents (3)
   NOM                 PID     ENDPOINT ZMQ                   SLAVE        ÉTAT
   --------------------------------------------------------------------------
   capteur             12345   ipc:///tmp/acl_capteur         local        active
   moniteur            12346   ipc:///tmp/acl_moniteur        local        waiting
   coordinateur        12347   ipc:///tmp/acl_coordinateur    local        active

   Services DF (2)
   AGENT               TYPE            SERVICE              LANGAGE
   ------------------------------------------------------------------------------
   capteur             sensor          temperature-reader   fipa-sl
   coordinateur        planning        task-allocator       fipa-sl

**Contrôler un agent à distance :**

.. code-block:: bash

   # Suspendre un agent (il stoppe ses behaviours mais reste en mémoire)
   ./bin/agentmanager suspend capteur

   # Réveiller un agent suspendu
   ./bin/agentmanager wake capteur

   # Supprimer un agent (envoie un signal de suppression)
   ./bin/agentmanager kill capteur

**Chercher dans le DF depuis le terminal :**

.. code-block:: bash

   # Tous les agents de type "transport"
   ./bin/agentmanager df search transport

   # Filtrer par type et ontologie
   ./bin/agentmanager df search transport logistics

.. code-block:: text

   [DF] 2 service(s) pour type="transport"
   AGENT               TYPE            SERVICE              LANGAGE     ONTOLOGIE
   ------------------------------------------------------------------------------
   transporteur-a      transport       road-carrier         fipa-sl     logistics
   transporteur-b      transport       road-carrier         fipa-sl     logistics

agentmonitor
-------------

``agentmonitor`` affiche les messages de log envoyés par les agents via
``sendMsgMonitor()``. C'est un flux UDP en temps réel — chaque message
arrive dans le terminal avec son numéro de séquence.

.. code-block:: bash

   ./bin/agentmonitor

.. code-block:: text

   [Monitor] écoute sur 127.0.0.1:40013
   00000001 : Environnement -> carte initialisée 600x300
   00000002 : Environnement -> agent capteur connecté
   00000003 : Environnement -> température publiée : 22°C

Pour envoyer un message depuis votre environnement :

.. code-block:: cpp

   // Dans une classe héritant de Environnement
   sendMsgMonitor("température publiée : " + std::to_string(temp_) + "°C");

.. note::

   ``agentmonitor`` est destiné à l'environnement (classe
   ``Environnement``). Pour logger depuis un agent, utilisez plutôt
   le ``Logger`` de gAgent qui écrit dans les fichiers de log
   structurés (JSON Lines).

Ordre de lancement recommandé
--------------------------------

**Mode local (une seule machine) :**

.. code-block:: bash

   # Terminal 1 — plateforme (AMS + DF via sockets Unix)
   ./bin/agentplatform

   # Terminal 2 — visualisation web (si vous utilisez l'environnement)
   ./bin/agentview

   # Terminal 3 — monitoring live
   ./bin/agentmanager watch 500

   # Terminal 4 — logs environnement
   ./bin/agentmonitor

   # Terminal 5 — votre simulation
   ./build/ma_simulation

**Mode cluster (plusieurs machines) :**

Sur la **machine master** (ex. 192.168.1.10) — à lancer en premier :

.. code-block:: bash

   # Terminal 1 — plateforme master (AMS TCP:40011, DF TCP:40012)
   ./bin/agentplatform --master --ip 192.168.1.10

   # Terminal 2 — supervision du cluster complet
   ./bin/agentmanager watch 500

Sur chaque **machine esclave** (ex. 192.168.1.20) :

.. code-block:: bash

   # Terminal 1 — plateforme esclave (se connecte au master)
   ./bin/agentplatform --slave 192.168.1.10:40011

   # Terminal 2 — votre simulation (les agents lisent /tmp/gagent.cfg)
   ./build/ma_simulation

.. note::

   ``agentmanager`` peut être lancé depuis n'importe quelle machine du
   cluster — il interroge le master AMS et affiche tous les agents,
   locaux et distants, avec leur machine d'appartenance (colonne
   ``SLAVE``).

   Pour les options avancées (``--port``, ``--ip``, ``--control-port``,
   ``--base-port``), voir :doc:`platform_multihost`.

Référence des commandes
------------------------

.. list-table::
   :widths: 45 55
   :header-rows: 1

   * - Commande
     - Description
   * - ``agentmanager list``
     - Liste tous les agents dans l'AMS
   * - ``agentmanager watch [ms]``
     - Surveillance en temps réel (défaut : 1000 ms)
   * - ``agentmanager kill <nom>``
     - Supprime l'agent
   * - ``agentmanager suspend <nom>``
     - Suspend l'agent
   * - ``agentmanager wake <nom>``
     - Réveille l'agent suspendu
   * - ``agentmanager df search <type>``
     - Cherche les services par type dans le DF
   * - ``agentmanager df search <type> <ontologie>``
     - Cherche avec filtre sur l'ontologie
   * - ``agentmonitor``
     - Affiche les logs de l'environnement en temps réel
