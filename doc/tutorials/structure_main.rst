2 — Structurer son main
========================

Tout programme gAgent suit le même squelette dans ``main()``.
Ce tutoriel explique le rôle de chaque brique, sans rentrer dans les
détails techniques.

Le squelette de base
--------------------

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/core/AgentCore.hpp>

   using namespace gagent;

   class MonBehaviour : public Behaviour { /* ... */ };

   class MonAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new MonBehaviour(this));
       }
   };

   int main() {
       AgentCore::initAgentSystem();   // 1. préparer l'environnement
       MonAgent agent;
       agent.init();                   // 2. démarrer l'agent
       AgentCore::syncAgentSystem();   // 3. attendre la fin
       return 0;
   }

``initAgentSystem()`` — préparer l'environnement
-------------------------------------------------

C'est la première chose à appeler, **avant tout**. Elle prépare
l'environnement dans lequel vos agents vont vivre :

- Elle s'assure que si vous appuyez sur **Ctrl+C**, tous les agents
  s'arrêtent proprement, sans laisser de processus orphelins en arrière-plan.

- Elle lit le fichier ``config.cfg`` si vous en avez un, pour savoir où
  trouver la plateforme FIPA. Sans ce fichier, les agents fonctionnent
  quand même en mode autonome.

Pensez à ``initAgentSystem()`` comme à l'allumage du tableau de bord
avant de démarrer le moteur — on le fait une seule fois, en premier.

``agent.init()`` — démarrer un agent
--------------------------------------

C'est ici que l'agent prend vie. Dès l'appel à ``init()``, l'agent
démarre et commence à travailler **de façon indépendante** : vous n'avez
pas à attendre qu'il ait fini pour continuer à lancer d'autres agents.

.. code-block:: cpp

   AgentA a;  a.init();   // A démarre
   AgentB b;  b.init();   // B démarre aussitôt après
   // A et B tournent maintenant en parallèle

Vous pouvez lancer autant d'agents que vous voulez à la suite.

``syncAgentSystem()`` — attendre la fin
-----------------------------------------

``syncAgentSystem()`` met le programme principal en attente jusqu'à ce
que **tous** les agents aient terminé. Un seul appel suffit, quel que
soit le nombre d'agents lancés.

.. code-block:: cpp

   AgentA a;  a.init();
   AgentB b;  b.init();
   AgentC c;  c.init();

   AgentCore::syncAgentSystem();  // attend que A, B et C aient tous terminé

Arrêter la simulation
---------------------

La simulation s'arrête de deux façons :

- **Naturellement** — quand un agent a terminé tout son travail (tous ses
  behaviours sont finis), il s'éteint de lui-même. Quand tous les agents
  sont éteints, le programme se termine.

- **Ctrl+C** — à tout moment, Ctrl+C arrête proprement l'ensemble des
  agents. C'est ``initAgentSystem()`` qui a mis cela en place.

.. note::

   Pour éteindre un agent spécifique depuis le code sans tout arrêter,
   utilisez ``agent.doDelete()``.

``setup()`` — donner ses tâches à l'agent
------------------------------------------

``setup()`` est la méthode que vous implémentez dans votre agent pour lui
donner ses tâches au démarrage. C'est ici et seulement ici que vous
ajoutez des behaviours avec ``addBehaviour()``.

.. code-block:: cpp

   class MonAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new TacheA(this));
           addBehaviour(new TacheB(this));
           // TacheA et TacheB s'exécutent en parallèle
       }
   };

Si vous ajoutez plusieurs behaviours, ils s'exécutent tous **en même
temps**, chacun de façon indépendante.

``action()`` et ``done()`` — le cycle d'un behaviour
------------------------------------------------------

Un behaviour est une tâche que l'agent répète en boucle. Le moteur
appelle ``action()`` encore et encore, jusqu'à ce que ``done()``
indique que c'est terminé.

.. code-block:: text

   ┌──────────────────────────────────┐
   │  Est-ce terminé ? (done())       │
   │    Non  →  exécuter (action()) ──┤
   │    Oui  →  s'arrêter             │
   └──────────────────────────────────┘

Vous avez aussi deux méthodes optionnelles pour préparer et nettoyer :

.. code-block:: cpp

   class MonBehaviour : public Behaviour {
       int count_ = 0;
   public:
       MonBehaviour(Agent* ag) : Behaviour(ag) {}

       void onStart() override {
           // appelé une seule fois au démarrage
           std::cout << "Je commence !" << std::endl;
       }

       void action() override {
           std::cout << "tick " << ++count_ << std::endl;
           sleep(1);
       }

       bool done() override {
           return count_ >= 5;  // arrêt après 5 exécutions
       }

       void onEnd() override {
           // appelé une seule fois à la fin
           std::cout << "J'ai terminé." << std::endl;
       }
   };

.. tip::

   Si vous voulez un behaviour qui tourne indéfiniment sans condition d'arrêt,
   faites hériter votre classe de ``CyclicBehaviour`` au lieu de ``Behaviour``
   — vous n'avez alors pas besoin d'implémenter ``done()``.
   Pour une tâche qui ne s'exécute qu'une seule fois, utilisez
   ``OneShotBehaviour``.
