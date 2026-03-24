# Mon premier agent gAgent

Ce dossier contient un exemple minimal : un agent qui dit "Hello" toutes les
secondes, cinq fois, puis se termine.

## Compilation

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Lancer

```bash
./build/hello
```

Résultat attendu :

```
[HelloAgent] Hello ! (1/5)
[HelloAgent] Hello ! (2/5)
[HelloAgent] Hello ! (3/5)
[HelloAgent] Hello ! (4/5)
[HelloAgent] Hello ! (5/5)
```

## Aller plus loin

- Ajoute plusieurs behaviours dans `setup()` : ils s'exécutent en parallèle.
- Utilise `acl_send()` / `acl_receive()` pour faire communiquer deux agents.
- Consulte la documentation complète dans `doc/`.

## Structure de dist/

```
dist/
├── src/                ← tu es ici
├── include/gagent/     ← headers publics
├── lib/libgagent.so    ← bibliothèque
└── bin/
    ├── agentplatform   ← daemon FIPA (AMS + DF)
    ├── agentmanager    ← CLI de supervision
    └── agentview       ← visualisation web (http://localhost:8080)
```
