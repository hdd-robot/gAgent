Contribuer
==========

Compilation en mode développement
----------------------------------

.. code-block:: bash

   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
   make -j$(nproc)

Structure d'un nouvel agent
-----------------------------

1. Crée ta classe dans ``examples/`` ou ``tests/``
2. Hérite de ``gagent::Agent``
3. Implémente ``setup()``
4. Ajoute tes ``Behaviour`` dans ``setup()``

Conventions de code
--------------------

- C++17 minimum
- ``std::string`` partout (pas de ``char*``)
- Headers publics → ``include/gagent/{module}/``
- Implémentation privée → ``src/{module}/``
- Pas d'include Qt dans les headers publics (``#ifdef BUILD_GUI``)

Ajouter un test
----------------

.. code-block:: cmake

   # Dans tests/CMakeLists.txt
   add_gagent_test(mon_test  mon_test.cpp)

Mettre à jour la documentation
--------------------------------

.. code-block:: bash

   cd doc
   make html
   # → doc/_build/html/index.html
