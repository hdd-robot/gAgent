Feuille de route
================

Phase 1 — Stabilisation ✅
---------------------------

- [x] Suppression de CORBA
- [x] Migration ``char*`` → ``std::string``
- [x] Parser FIPA ACL Flex/Bison C++ (remplacement Boost.Spirit)
- [x] Isolation Qt5 (``BUILD_GUI=OFF`` par défaut)
- [x] Réorganisation CMake moderne (``include/`` / ``src/``)
- [x] Démonstration échange ACL entre deux agents

Phase 2 — Infrastructure 🔄
-----------------------------

- [ ] **Transport gRPC** : remplacer POSIX MQ par un bus gRPC (multi-machine, Python interop)
- [ ] **AMS** : registre central d'agents (découverte par nom)
- [ ] **DF** : annuaire de services fonctionnel
- [ ] **Thread-based agents** : option d'agents légers en threads plutôt que fork
- [ ] Tests unitaires (Google Test / Catch2)
- [ ] Logging structuré

Phase 3 — Neuro-symbolique 🎯
------------------------------

- [ ] **Couche ontologie** : définitions formelles liées aux champs ACL
- [ ] **Moteur HTN** : planificateur hiérarchique embarqué
- [ ] **Bridge PDDL** : interface vers fast-downward ou un solver externe
- [ ] **PythonAgent** : agent C++ qui délègue les décisions à un LLM Python
- [ ] **Protocoles FIPA** : Contract Net, Subscribe-Notify, etc.
