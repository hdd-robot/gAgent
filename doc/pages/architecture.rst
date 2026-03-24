Architecture
============

Structure du projet
-------------------

.. code-block:: text

   gAgent/
   ├── include/gagent/          # API publique (headers)
   │   ├── core/                # Agent, Behaviour, AgentCore, AgentID…
   │   ├── messaging/           # ACLMessage, AgentIdentifier
   │   ├── env/                 # Environnement, VisualAgent
   │   ├── comm/                # CommunicationManager
   │   ├── platform/            # AMSClient, DFClient, EnvClient, PlatformConfig
   │   └── utils/               # Logger, ErrorHandler, udp_client_server
   │
   ├── src/                     # Implémentation privée → libgagent.so
   │   ├── core/
   │   ├── messaging/           # fipa_acl.l, fipa_acl.y, FipaAclDriver
   │   ├── env/
   │   ├── platform/            # AMSClient, DFClient, EnvClient
   │   ├── comm/
   │   └── utils/
   │
   ├── platform/                # Daemon FIPA (BUILD_PLATFORM=ON)
   │   ├── ams/                 # AMS — registre des agents
   │   ├── df/                  # DF — annuaire des services
   │   ├── manager/             # agentmanager CLI
   │   └── monitor/             # agentmonitor UDP
   │
   ├── view/                    # Visualisation web (BUILD_VIEW=ON)
   │                            # → agentview (HTTP + SVG)
   │
   ├── tests/                   # Tests et démonstrations
   ├── examples/                # Exemples d'usage
   └── doc/                     # Cette documentation

Cycle de vie d'un agent
-----------------------

.. code-block:: text

   main()
     │
     ├─ AgentCore::initAgentSystem()     # signal handlers, config
     │
     ├─ MyAgent agent;                   # constructeur : génère AgentID
     │
     ├─ agent.init()                     # fork()
     │     │
     │     ├─ [PARENT] retourne          # continue dans main()
     │     │
     │     └─ [CHILD] _init()
     │           ├─ thread : control_Thread()
     │           ├─ thread : listener_extern_signals_Thread()
     │           ├─ thread : control_message()    # POSIX MQ
     │           ├─ setup()                       # addBehaviour(...)
     │           ├─ thread par Behaviour
     │           ├─ join tous les threads
     │           ├─ takeDown()
     │           └─ _exit(0)
     │
     └─ AgentCore::syncAgentSystem()     # waitpid()

États d'un agent
----------------

Le cycle de vie complet (états, transitions, diagramme, signaux RT) est
documenté dans la page dédiée :

:doc:`agent_lifecycle`

Communication inter-agents
--------------------------

Les agents communiquent via **ZeroMQ PUSH/PULL** :

- Canal de contrôle interne : ``/{8-char-random-id}`` — queue POSIX MQ (signaux de lifecycle)
- Canal ACL applicatif : ``ipc:///tmp/acl_<nom>`` — socket ZeroMQ PULL/PUSH

Les connexions PUSH sont persistantes (``PushCache``, une par destination)
pour éviter le problème du *slow joiner* ZeroMQ. Le message est sérialisé
au format FIPA ACL s-expression et parsé par le parser Flex/Bison intégré.

Parser FIPA ACL
---------------

Le parser est généré automatiquement par **Bison LALR(1)** + **Flex** en mode C++ :

.. code-block:: text

   src/messaging/fipa_acl.y  →  (bison)  →  fipa_acl_parser.cpp/.hpp
   src/messaging/fipa_acl.l  →  (flex)   →  fipa_acl_lexer.cpp
                │
                └─ FipaAclDriver orchestre lexer + parser
                   → ACLMessage::parse(string) retourne std::optional<ACLMessage>

Format d'un message FIPA ACL
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: text

   (inform
    :sender    (agent-identifier :name alice)
    :receiver  (set (agent-identifier :name bob))
    :content   "Il est 21:00:00"
    :language  fipa-sl
    :ontology  time-query
    :conversation-id conv-1
   )
