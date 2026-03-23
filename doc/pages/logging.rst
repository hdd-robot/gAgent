Logging structuré (JSON Lines)
===============================

gAgent dispose de deux systèmes de logs indépendants et complémentaires :

+------------------+----------------------------+-----------------------------+
| Mode             | Activation                 | Format                      |
+==================+============================+=============================+
| **Texte**        | toujours actif (stdout)    | ``YYYY-MM-DD HH:MM:SS [LVL] msg`` |
+------------------+----------------------------+-----------------------------+
| **JSON Lines**   | variable ``GAGENT_LOG``    | un objet JSON par ligne     |
+------------------+----------------------------+-----------------------------+

Le mode JSON Lines est conçu pour l'ingestion dans des outils comme
**ELK** (Elasticsearch/Kibana) ou **Grafana/Loki**.

Activation
----------

Définir la variable d'environnement ``GAGENT_LOG`` avec le chemin du fichier
de sortie avant de lancer l'application :

.. code-block:: bash

   GAGENT_LOG=gagent.jsonl ./build/tests/test_subscribe_notify

   # ou pour une démo
   GAGENT_LOG=/var/log/gagent/events.jsonl ./build/examples/demo_visualization

Si ``GAGENT_LOG`` n'est pas défini, les appels ``logJson()`` sont des no-ops
(aucun overhead en production).

Format d'une ligne
------------------

Chaque ligne est un objet JSON autonome (JSON Lines / NDJSON) :

.. code-block:: json

   {"ts":"2026-03-24T10:00:00.123Z","event":"acl_send","from":"alice","to":"bob","perf":"request","conv":"cnp-alice-75727","content":"compute-plan"}

Les champs communs à toutes les lignes :

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - Champ
     - Description
   * - ``ts``
     - Horodatage UTC ISO 8601 avec milliseconde (``YYYY-MM-DDTHH:MM:SS.mmmZ``)
   * - ``event``
     - Type d'événement (voir ci-dessous)

Événements émis
---------------

agent_start
~~~~~~~~~~~

Émis dans ``Agent::_init()`` après l'enregistrement AMS, une fois par agent
au démarrage de son processus child.

.. code-block:: json

   {"ts":"...","event":"agent_start","agent":"alice","pid":"12345"}

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - Champ
     - Description
   * - ``agent``
     - Nom de l'agent (``AgentID::getAgentName()``)
   * - ``pid``
     - PID du processus child

agent_stop
~~~~~~~~~~

Émis dans ``Agent::doDelete()`` avant le désenregistrement AMS/DF.

.. code-block:: json

   {"ts":"...","event":"agent_stop","agent":"alice","pid":"12345"}

agent_lifecycle
~~~~~~~~~~~~~~~

Émis à chaque transition d'état dans ``control_Thread()``.

.. code-block:: json

   {"ts":"...","event":"agent_lifecycle","agent":"alice","state":"suspended"}

États possibles : ``active``, ``suspended``, ``waiting``, ``waking``,
``transit``.

acl_send
~~~~~~~~

Émis dans ``AclMQ::acl_send()`` après chaque envoi réussi.

.. code-block:: json

   {"ts":"...","event":"acl_send","from":"monitor","to":"sensor","perf":"subscribe","conv":"snp-monitor-75727","content":"temperature"}

.. list-table::
   :header-rows: 1
   :widths: 15 85

   * - Champ
     - Description
   * - ``from``
     - Expéditeur (champ ``sender`` du message ACL)
   * - ``to``
     - Destinataire (nom de la queue cible)
   * - ``perf``
     - Performative FIPA (``request``, ``inform``, ``agree``, etc.)
   * - ``conv``
     - Identifiant de conversation (``conversation-id``)
   * - ``content``
     - Contenu du message, tronqué à 120 caractères

acl_recv
~~~~~~~~

Émis dans ``AclMQ::acl_receive()`` après chaque réception réussie.

.. code-block:: json

   {"ts":"...","event":"acl_recv","to":"sensor","from":"monitor","perf":"subscribe","conv":"snp-monitor-75727","content":"temperature"}

Champs identiques à ``acl_send`` (rôles ``from``/``to`` inversés).

Utilisation depuis le code C++
-------------------------------

La macro ``LOG_JSON`` est disponible en incluant ``Logger.hpp`` :

.. code-block:: cpp

   #include <gagent/utils/Logger.hpp>

   // Dans un Behaviour ou un Agent
   LOG_JSON("mon_event", {
       {"agent", "alice"},
       {"detail", some_string},
       {"count",  std::to_string(n)},
   });

Toutes les valeurs sont des chaînes. Pour émettre des champs numériques,
utiliser ``std::to_string()``.

Appel direct sur le singleton :

.. code-block:: cpp

   gagent::Logger::getInstance().logJson("mon_event", {
       {"key", "value"},
   });

Exploitation avec jq
--------------------

.. code-block:: bash

   # Suivre les messages en temps réel
   tail -f gagent.jsonl | jq .

   # Filtrer les envois d'une conversation
   jq 'select(.event=="acl_send" and .conv=="cnp-alice-75727")' gagent.jsonl

   # Compter les messages par performative
   jq -r '.perf' gagent.jsonl | sort | uniq -c | sort -rn

   # Chronologie d'un agent
   jq 'select(.agent=="alice")' gagent.jsonl

Intégration ELK / Grafana Loki
--------------------------------

Le format JSON Lines est nativement reconnu par **Filebeat** :

.. code-block:: yaml

   # filebeat.yml (extrait)
   filebeat.inputs:
     - type: log
       paths: ["/var/log/gagent/*.jsonl"]
       json.keys_under_root: true
       json.add_error_key: true

   output.elasticsearch:
     hosts: ["localhost:9200"]

Pour **Grafana Loki** avec Promtail :

.. code-block:: yaml

   # promtail-config.yml (extrait)
   scrape_configs:
     - job_name: gagent
       static_configs:
         - targets: [localhost]
           labels:
             job: gagent
             __path__: /var/log/gagent/*.jsonl
       pipeline_stages:
         - json:
             expressions:
               event: event
               agent: agent
               perf: perf
