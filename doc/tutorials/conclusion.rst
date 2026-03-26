Aller plus loin
================

Vous avez parcouru l'essentiel de gAgent : les bases, la communication
FIPA ACL, les protocoles d'interaction, l'environnement situé, les
agents LLM, la plateforme et ses outils. Vous avez de quoi construire
un système multi-agent complet.

Ce que vous savez faire
------------------------

.. list-table::
   :widths: 30 70
   :header-rows: 0

   * - **Créer des agents**
     - ``Agent``, ``setup()``, ``takeDown()``, ``AgentCore``
   * - **Structurer la logique**
     - ``Behaviour``, ``OneShotBehaviour``, ``CyclicBehaviour``,
       ``TickerBehaviour``, ``WakerBehaviour``
   * - **Communiquer**
     - ``acl_send``, ``acl_receive``, ``ACLMessage``, performatives FIPA
   * - **Utiliser des protocoles**
     - ``RequestInitiator``, ``ContractNetInitiator``,
       ``SubscribeInitiator`` et leurs participants
   * - **Situer les agents**
     - ``Environnement``, ``VisualAgent``, ``agentview``
   * - **Intégrer un LLM**
     - ``PythonBehaviour``, ``gagent_py``, OpenAI / Claude / Ollama
   * - **Superviser**
     - ``agentmanager``, ``agentmonitor``, logs JSON Lines
   * - **Déployer**
     - ``GAGENT_ENDPOINT_<NOM>`` pour le multi-machine

Pour aller plus loin
---------------------

**La Référence API** documente toutes les classes, méthodes et
paramètres en détail. Consultez-la quand vous avez besoin de connaître
une signature exacte ou les options d'un constructeur.

**Le Guide utilisateur** couvre des sujets plus avancés :

- L'architecture interne de gAgent
- La plateforme AMS/DF en détail
- La messagerie FIPA ACL complète (tous les performatives)
- Les patterns avancés de behaviours
- Le logging structuré et l'intégration ELK/Grafana

**Les exemples** dans ``examples/`` sont des programmes fonctionnels
que vous pouvez compiler et lancer directement :

.. code-block:: bash

   # Visualisation — 5 agents avec des mouvements distincts
   ./build/examples/demo_visualization

   # Agent LLM — questions/réponses via OpenAI ou Ollama
   OPENAI_API_KEY=sk-... ./build/examples/llm_agent

Quelques idées de projets
--------------------------

- **Simulation de fourmis** — environnement 2D, phéromones comme
  état partagé, comportement émergent de recherche de nourriture.

- **Réseau de capteurs intelligent** — agents capteurs publient des
  mesures via Subscribe-Notify, agent agrégateur calcule des statistiques,
  agent alarme déclenche des alertes via Request.

- **Planificateur multi-robot** — Contract Net pour l'attribution de
  tâches, AMS/DF pour la découverte dynamique des robots disponibles.

- **Assistant LLM collaboratif** — plusieurs agents LLM avec des
  spécialisations différentes (expert domaine, modérateur, synthétiseur)
  qui collaborent pour répondre à une question complexe.

Contribuer
-----------

Le code source est disponible dans le dépôt gAgent. Pour signaler un
bug, proposer une amélioration ou contribuer du code, consultez la
page :doc:`contributing <../pages/contributing>`.
