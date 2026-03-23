API — Environnement
===================

Module ``gagent/env`` : environnement partagé et pile de snapshots NSAP.

Environnement
-------------

.. doxygenclass:: gagent::Environnement
   :members:

Méthodes NSAP
~~~~~~~~~~~~~

.. list-table::
   :widths: 30 15 55
   :header-rows: 1

   * - Méthode
     - Retour
     - Description
   * - ``push_nsap()``
     - ``int``
     - Empile l'état courant. Retourne la taille de la pile.
   * - ``pull_nsap()``
     - ``int``
     - Restaure le snapshot le plus récent (LIFO). Retourne la taille restante, ``-1`` si vide.
   * - ``clear_nsap()``
     - ``void``
     - Vide toute la pile sans modifier l'état courant.
   * - ``get_nsaps()``
     - ``map<int,string>*``
     - Retourne l'index ``{ séquence → timestamp ISO }``.

Voir :doc:`../pages/nsap` pour des exemples complets.

Socket de visualisation
~~~~~~~~~~~~~~~~~~~~~~~

``serve(socket_path)`` démarre un serveur Unix socket (dans un thread
séparé) qui expose ``list_attr`` en JSON. Appelé automatiquement par
``AgentCore::initEnvironnementSystem()``.

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Commande socket
     - Réponse
   * - ``GET_AGENTS``
     - JSON ``{width, height, agents:[...]}``
   * - ``GET_NSAP``
     - JSON ``{count, snaps:[{seq, timestamp},...]}``

Voir :doc:`../pages/visualization` pour l'usage complet.

VisualAgent
-----------

.. doxygenclass:: gagent::VisualAgent
   :members:

EnvClient
---------

``gagent::platform::EnvClient`` (``include/gagent/platform/EnvClient.hpp``)
est le client C++ du socket Environnement.

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Méthode
     - Description
   * - ``getAgents() → string``
     - Retourne le JSON ``GET_AGENTS`` (chaîne vide si env absent)
   * - ``getNsap() → string``
     - Retourne le JSON ``GET_NSAP`` (chaîne vide si env absent)

.. code-block:: cpp

   #include <gagent/platform/EnvClient.hpp>

   gagent::platform::EnvClient env;
   std::string json = env.getAgents();
   if (json.empty())
       std::cerr << "Environnement non disponible\n";
