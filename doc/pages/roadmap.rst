Feuille de route
================

Phase 1 — Stabilisation ✅
---------------------------

- [x] Suppression de CORBA
- [x] Migration ``char*`` → ``std::string``
- [x] Parser FIPA ACL Flex/Bison C++ (remplacement Boost.Spirit)
- [x] Réorganisation CMake moderne (``include/`` / ``src/``)
- [x] Démonstration échange ACL entre deux agents

Phase 2 — Infrastructure FIPA ✅
----------------------------------

- [x] **AMS** : registre central d'agents (Unix socket, protocole texte)
- [x] **DF** : annuaire de services fonctionnel (recherche par type/ontologie)
- [x] **Clients C++** : ``AMSClient``, ``DFClient``, ``EnvClient`` dans ``libgagent``
- [x] **Auto-enregistrement** : ``Agent::_init()`` s'enregistre dans l'AMS automatiquement
- [x] **agentmanager** CLI : list, watch, kill, suspend, wake, df search
- [x] **agentmonitor** : logs UDP temps réel
- [x] **Visualisation web** : ``agentview`` — HTTP + SVG, zéro dépendance externe
- [x] **Socket Environnement** : ``Environnement::serve()`` expose ``list_visual_agents`` en JSON
- [x] **Tests unitaires** : ``test_acl`` (66 assertions), ``test_platform`` (24 assertions)
- [x] **Cycle de vie documenté** : états, transitions, signaux RT, diagramme Graphviz

Phase 3 — Neuro-symbolique 🎯
------------------------------

- [ ] **Transport gRPC** : remplacer POSIX MQ par un bus gRPC (multi-machine, Python interop)
- [ ] **Thread-based agents** : option d'agents légers en threads plutôt que ``fork``
- [ ] **Couche ontologie** : définitions formelles liées aux champs ACL
- [ ] **Moteur HTN** : planificateur hiérarchique embarqué
- [ ] **Bridge PDDL** : interface vers fast-downward ou un solver externe
- [x] **PythonAgent** : ``PythonBehaviour`` — bridge C++ ↔ Python avec historique,
  config depuis C++ (system_prompt, model, max_tokens), détection crash
- [x] **Contract Net** (FIPA SC00029H) : ``ContractNetInitiator`` / ``ContractNetParticipant``
- [ ] **Subscribe-Notify** (FIPA SC00035H)
- [ ] **Autres protocoles FIPA** : Iterated Contract Net, Dutch Auction, etc.
- [ ] **Logging structuré** : format JSON pour ingestion ELK/Grafana
