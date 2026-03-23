Behaviours
==========

Un ``Behaviour`` est l'unité de comportement d'un agent. Chaque behaviour
tourne dans son propre thread à l'intérieur du processus child de l'agent.

Hiérarchie
----------

.. code-block:: text

   Behaviour
   ├── SimpleBehaviour
   │   ├── OneShotBehaviour    action() appelé une fois, done() = true
   │   ├── CyclicBehaviour     action() en boucle infinie, done() = false (final)
   │   ├── WakerBehaviour      attend N ms puis appelle onWake()
   │   └── TickerBehaviour     appelle onTick() toutes les N ms
   └── (direct)                done() libre à surcharger → usage recommandé

Utilisation
-----------

Créer un behaviour
~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   class MyBehaviour : public Behaviour {
       int count_ = 0;
   public:
       MyBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           // logique du comportement
           count_++;
       }

       bool done() override {
           return count_ >= 10;  // arrêt après 10 itérations
       }

       void onStart() override { /* appelé avant la boucle */ }
       void onEnd()   override { /* appelé après la boucle */ }
   };

Enregistrer dans setup()
~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   void MyAgent::setup() override {
       addBehaviour(new MyBehaviour(this));
   }

.. warning::

   ``TickerBehaviour`` et ``CyclicBehaviour`` ont ``done()`` déclaré ``final``.
   Si tu as besoin d'un comportement avec arrêt conditionnel, hérite directement
   de ``Behaviour``.

TickerBehaviour
---------------

.. code-block:: cpp

   class MyTicker : public TickerBehaviour {
   public:
       MyTicker(Agent* ag) : TickerBehaviour(ag, 1000) {}  // 1000 ms

       void onTick() override {
           std::cout << "tick!\n";
       }
       // done() est final → tourne indéfiniment
   };

WakerBehaviour
--------------

.. code-block:: cpp

   class MyWaker : public WakerBehaviour {
   public:
       MyWaker(Agent* ag) : WakerBehaviour(ag, 5000) {}  // déclenche après 5s

       void onWake() override {
           std::cout << "réveillé!\n";
       }
   };

Parallélisme
------------

Tous les behaviours d'un agent tournent en **threads parallèles** dans le
processus child. L'agent attend la fin de tous ses behaviours avant de
s'arrêter.

.. code-block:: text

   child process
   ├── thread : control_Thread
   ├── thread : listener_extern_signals_Thread
   ├── thread : control_message (POSIX MQ)
   ├── thread : Behaviour A  ─┐
   ├── thread : Behaviour B   ├── parallèles
   └── thread : Behaviour C  ─┘
