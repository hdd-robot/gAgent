# gAgent — Kit de développement

Bienvenue dans le kit de développement **gAgent**, une plateforme
multi-agents C++17 conforme au standard FIPA ACL.

---

## Contenu de ce répertoire

```
dist/
├── README.md               ← vous êtes ici
│
├── src/                    ← projet de démarrage (copiez ce dossier)
│   ├── main.cpp            ← exemple : un agent qui dit Hello 5 fois
│   ├── CMakeLists.txt      ← configuration CMake prête à l'emploi
│   └── README.md           ← guide rapide du projet exemple
│
├── include/                ← headers publics à inclure dans vos projets
│   └── gagent/
│       ├── core/           ← Agent, Behaviour, AgentCore, AgentID…
│       ├── messaging/      ← ACLMessage, AclMQ (acl_send / acl_receive)
│       ├── protocols/      ← Request, ContractNet, SubscribeNotify
│       ├── platform/       ← AMSClient, DFClient (annuaires FIPA)
│       ├── env/            ← Environnement, VisualAgent
│       └── utils/          ← Logger, ErrorHandler
│
├── lib/
│   ├── libgagent.so        ← bibliothèque à lier dans vos projets
│   └── cmake/gAgent/       ← fichiers CMake pour find_package(gAgent)
│
└── bin/
    ├── agentplatform       ← daemon FIPA : lance AMS + DF
    ├── agentmanager        ← CLI : liste, surveille et contrôle les agents
    ├── agentmonitor        ← affiche les logs des agents en temps réel
    └── agentview           ← visualisation web (http://localhost:8080)
```

---

## Créer votre premier projet

### 1. Copier le projet exemple

```bash
cp -r dist/src/ mon-projet/
cd mon-projet/
```

### 2. Compiler

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

Le `CMakeLists.txt` fourni trouve automatiquement gAgent dans le
répertoire `dist/` adjacent. Si vous avez déplacé `dist/` ailleurs :

```bash
cmake .. -DCMAKE_PREFIX_PATH=/chemin/vers/dist
```

### 3. Lancer

```bash
LD_LIBRARY_PATH=/chemin/vers/dist/lib ./hello
```

Pour ne pas avoir à répéter `LD_LIBRARY_PATH` à chaque fois :

```bash
# Exporter dans le shell courant
export LD_LIBRARY_PATH=/chemin/vers/dist/lib:$LD_LIBRARY_PATH

# Ou installer définitivement (nécessite sudo)
echo "/chemin/vers/dist/lib" | sudo tee /etc/ld.so.conf.d/gagent.conf
sudo ldconfig
```

---

## Créer un projet from scratch avec CMake

Si vous préférez partir d'un projet vide, voici le `CMakeLists.txt`
minimum pour utiliser gAgent :

```cmake
cmake_minimum_required(VERSION 3.15)
project(MonProjet LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(gAgent REQUIRED)

add_executable(mon_app main.cpp)
target_link_libraries(mon_app PRIVATE gAgent::gagent)
```

```bash
cmake .. -DCMAKE_PREFIX_PATH=/chemin/vers/dist
cmake --build .
```

---

## Lancer la plateforme FIPA (optionnel)

La plateforme fournit l'annuaire des agents (AMS) et des services (DF).
Elle est optionnelle — vos agents fonctionnent sans elle — mais
recommandée pour superviser votre système.

```bash
# Démarrer la plateforme dans un terminal dédié
./bin/agentplatform

# Lancer votre application dans un autre terminal
./mon-projet/build/mon_app

# Surveiller les agents en temps réel
./bin/agentmanager watch

# Visualisation web
./bin/agentview    # → ouvrir http://localhost:8080
```

---

## Documentation complète

La documentation est disponible dans le dépôt source, dans le dossier
`doc/`. Générez-la avec Sphinx :

```bash
sphinx-build -b html doc/ doc/_build/
```
