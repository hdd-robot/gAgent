Vue d'ensemble
==============

**gAgent** (*generative Agent*) est une plateforme multi-agent C++17 basée sur
le standard **FIPA ACL** (Foundation for Intelligent Physical Agents — Agent
Communication Language).

Objectif
--------

Le projet vise la **planification neuro-symbolique** : combiner la rigueur des
planificateurs symboliques (HTN, PDDL) avec la flexibilité des modèles de
langage (LLM), via des agents qui communiquent en FIPA ACL.

.. code-block:: text

   ┌──────────────────────────────────────────────────┐
   │                   gAgent Platform                │
   │                                                  │
   │   ┌─────────┐  ACL/FIPA  ┌─────────────────┐    │
   │   │  Alice  │ ─────────► │      Bob        │    │
   │   │ (Agent) │ ◄───────── │ (PlannerAgent)  │    │
   │   └─────────┘  REQUEST/  └────────┬────────┘    │
   │                 INFORM            │              │
   │                              HTN/PDDL            │
   │                              Planner             │
   └──────────────────────────────────────────────────┘

Architecture technique
----------------------

Chaque agent tourne dans son **propre processus** (``fork()``). La communication
inter-agent se fait via des queues POSIX MQ identifiées par nom d'agent.

Modules
-------

.. list-table::
   :widths: 20 80
   :header-rows: 1

   * - Module
     - Rôle
   * - ``gagent/core``
     - Cycle de vie des agents (``Agent``, ``Behaviour``, ``AgentCore``)
   * - ``gagent/messaging``
     - Parser FIPA ACL Flex/Bison, sérialisation ``ACLMessage``
   * - ``gagent/env``
     - Environnement partagé entre agents (``Environnement``)
   * - ``gagent/comm``
     - Gestionnaire de communication réseau (``CommunicationManager``)
   * - ``gagent/utils``
     - Utilitaires (``Logger``, ``ErrorHandler``, UDP)

Standards respectés
-------------------

- **FIPA ACL** : performatives, identificateurs d'agents, ontologies
- **FIPA AMS** : gestion du cycle de vie des agents (partiel)
- **FIPA DF**  : annuaire de services (partiel)
