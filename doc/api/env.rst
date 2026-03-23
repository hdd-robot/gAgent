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

VisualAgent
-----------

.. doxygenclass:: gagent::VisualAgent
   :members:
