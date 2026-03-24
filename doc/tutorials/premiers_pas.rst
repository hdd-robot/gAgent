1 — Premiers pas
=================

Ce tutoriel part de zéro : vous n'avez pas encore compilé gAgent et vous voulez
faire tourner votre premier agent en moins de dix minutes.

Prérequis système
-----------------

.. code-block:: bash

   sudo apt install cmake g++ libboost-all-dev libconfig++-dev \
                    flex bison libzmq3-dev

Étape 1 — Compiler gAgent
--------------------------

Clonez le dépôt et lancez le build :

.. code-block:: bash

   git clone <url-du-depot> gAgent
   cd gAgent
   cmake -S . -B build
   cmake --build build -j$(nproc)

Puis installez dans ``dist/`` (le préfixe par défaut) :

.. code-block:: bash

   cmake --install build/

Vous obtenez le répertoire ``dist/`` à la racine du projet :

.. code-block:: text

   dist/
   ├── src/               ← template de démarrage
   ├── include/gagent/    ← headers publics
   ├── lib/libgagent.so
   └── bin/
       ├── agentplatform
       ├── agentmanager
       ├── agentmonitor
       └── agentview

.. note::

   Si vous avez déjà un ``dist/`` fourni (archive, release), passez directement
   à l'étape 2 — pas besoin de recompiler.

Étape 2 — Copier le template dans votre projet
---------------------------------------------

``dist/src/`` contient un projet prêt à l'emploi.
Copiez-le là où vous voulez travailler :

.. code-block:: bash

   cp -r dist/src/ ~/mes-agents/hello
   cd ~/mes-agents/hello

Le contenu :

.. code-block:: text

   hello/
   ├── CMakeLists.txt   ← trouve gAgent automatiquement
   ├── main.cpp         ← un agent qui dit Hello toutes les secondes
   └── README.md

Étape 3 — Compiler votre agent
-----------------------------

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   cmake --build .

CMake cherche gAgent dans ``../`` (le ``dist/`` adjacent).
Si vous avez déplacé ``dist/`` ailleurs, précisez son chemin :

.. code-block:: bash

   cmake .. -DCMAKE_PREFIX_PATH=/chemin/vers/dist

Étape 4 — Lancer
-----------------

La bibliothèque ``libgagent.so`` se trouve dans ``dist/lib/``.
Il faut indiquer ce chemin au linker dynamique au lancement :

.. code-block:: bash

   LD_LIBRARY_PATH=/chemin/vers/dist/lib ./build/hello

Résultat attendu :

.. code-block:: text

   [HelloAgent] Hello ! (1/5)
   [HelloAgent] Hello ! (2/5)
   [HelloAgent] Hello ! (3/5)
   [HelloAgent] Hello ! (4/5)
   [HelloAgent] Hello ! (5/5)

.. tip::

   Pour ne pas retaper ``LD_LIBRARY_PATH`` à chaque fois, vous pouvez l'exporter
   dans votre shell ou utiliser ``ldconfig`` :

   .. code-block:: bash

      # Option A — export dans le shell courant
      export LD_LIBRARY_PATH=/chemin/vers/dist/lib:$LD_LIBRARY_PATH

      # Option B — installation système (nécessite sudo)
      echo "/chemin/vers/dist/lib" | sudo tee /etc/ld.so.conf.d/gagent.conf
      sudo ldconfig

Suite
-----

- Modifiez ``main.cpp`` pour ajouter vos propres behaviours.
- Lancez ``dist/bin/agentplatform`` pour activer la plateforme FIPA (AMS + DF).
- Consultez :doc:`/pages/messaging` pour faire communiquer deux agents.
