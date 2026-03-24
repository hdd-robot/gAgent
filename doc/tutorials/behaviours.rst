3 — Les behaviours
==================

Un behaviour est une **tâche** que vous confiez à un agent. L'agent peut
en avoir plusieurs, qui s'exécutent toutes en même temps, de façon
indépendante.

Ce tutoriel présente les différents types de behaviours disponibles et
quand les utiliser.

Qu'est-ce qu'un behaviour ?
----------------------------

Imaginez un agent comme un employé. Cet employé peut avoir plusieurs
responsabilités en parallèle : répondre aux mails, surveiller un tableau
de bord, envoyer des rapports toutes les heures. Chacune de ces
responsabilités est un behaviour.

Un behaviour répond à deux questions :

- **Que faire ?** → défini dans ``action()``
- **Est-ce terminé ?** → défini dans ``done()``

Le moteur de gAgent appelle ``action()`` encore et encore, jusqu'à ce
que ``done()`` réponde "oui".

----

Vue d'ensemble des types
-------------------------

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Type
     - Quand l'utiliser
   * - ``Behaviour``
     - Quand vous voulez contrôler vous-même la condition d'arrêt
   * - ``OneShotBehaviour``
     - Une tâche qui s'exécute une seule fois puis s'arrête
   * - ``CyclicBehaviour``
     - Une tâche qui tourne indéfiniment
   * - ``WakerBehaviour``
     - Attendre un certain délai, puis faire quelque chose une fois
   * - ``TickerBehaviour``
     - Répéter quelque chose à intervalle régulier

----

``Behaviour`` — le behaviour de base
--------------------------------------

C'est le type le plus flexible. Vous implémentez ``action()`` pour décrire
ce que fait l'agent, et ``done()`` pour dire quand il doit s'arrêter.

**Exemple — un agent qui compte jusqu'à 10 :**

.. code-block:: cpp

   class CompteurBehaviour : public Behaviour {
       int count_ = 0;
   public:
       CompteurBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           std::cout << "Compteur : " << ++count_ << std::endl;
           sleep(1);
       }

       bool done() override {
           return count_ >= 10;
       }
   };

Vous pouvez aussi utiliser ``onStart()`` et ``onEnd()`` pour exécuter du
code une seule fois au début et à la fin :

.. code-block:: cpp

   class AvecInitBehaviour : public Behaviour {
       int count_ = 0;
   public:
       AvecInitBehaviour(Agent* ag) : Behaviour(ag) {}

       void onStart() override {
           std::cout << "Démarrage de la tâche." << std::endl;
       }

       void action() override {
           std::cout << "Travail en cours... (" << ++count_ << ")" << std::endl;
           sleep(1);
       }

       bool done() override {
           return count_ >= 5;
       }

       void onEnd() override {
           std::cout << "Tâche terminée." << std::endl;
       }
   };

----

``OneShotBehaviour`` — faire quelque chose une seule fois
----------------------------------------------------------

Le behaviour s'exécute exactement une fois, puis s'arrête
automatiquement. Vous n'avez pas à implémenter ``done()``.

Utile pour : une initialisation, l'envoi d'un message de démarrage,
l'écriture d'un fichier de log, une action déclenchée par un événement.

**Exemple — envoyer une notification au démarrage :**

.. code-block:: cpp

   class NotificationDemarrage : public OneShotBehaviour {
   public:
       NotificationDemarrage(Agent* ag) : OneShotBehaviour(ag) {}

       void action() override {
           std::cout << "Agent démarré, notification envoyée." << std::endl;
           // envoyer un message, écrire dans un fichier, etc.
       }
   };

----

``CyclicBehaviour`` — tourner indéfiniment
-------------------------------------------

Le behaviour ne s'arrête jamais de lui-même. Vous n'avez pas à implémenter
``done()``. L'agent continue jusqu'à ce qu'il reçoive l'ordre de
s'arrêter (Ctrl+C ou ``doDelete()``).

Utile pour : surveiller une source de données en continu, écouter des
messages entrants, maintenir un état actif en permanence.

**Exemple — surveiller une valeur en continu :**

.. code-block:: cpp

   class SurveillanceBehaviour : public CyclicBehaviour {
   public:
       SurveillanceBehaviour(Agent* ag) : CyclicBehaviour(ag) {}

       void action() override {
           float temperature = lireTemperature();  // ta fonction
           if (temperature > 80.0f) {
               std::cout << "ALERTE : température critique !" << std::endl;
           }
           sleep(5);  // vérifie toutes les 5 secondes
       }
   };

----

``WakerBehaviour`` — attendre puis agir
----------------------------------------

Le behaviour attend un certain délai (en millisecondes), puis exécute
``onWake()`` une seule fois et s'arrête. C'est l'équivalent d'un
minuteur.

Utile pour : déclencher une action après un délai, simuler un temps de
traitement, planifier une tâche différée.

**Exemple — envoyer un rapport 30 secondes après le démarrage :**

.. code-block:: cpp

   class RapportDiffere : public WakerBehaviour {
   public:
       RapportDiffere(Agent* ag) : WakerBehaviour(ag, 30000) {}
       //                                               ↑
       //                                          30 000 ms = 30 s

       void onWake() override {
           std::cout << "30 secondes écoulées — envoi du rapport." << std::endl;
           // générer et envoyer le rapport
       }
   };

----

``TickerBehaviour`` — répéter à intervalle régulier
-----------------------------------------------------

Le behaviour appelle ``onTick()`` toutes les N millisecondes, indéfiniment.
C'est l'équivalent d'une horloge ou d'un timer périodique.

Utile pour : envoyer des mises à jour régulières, collecter des
métriques, publier un état toutes les X secondes.

**Exemple — publier l'état de l'agent toutes les 10 secondes :**

.. code-block:: cpp

   class PublicationEtat : public TickerBehaviour {
   public:
       PublicationEtat(Agent* ag) : TickerBehaviour(ag, 10000) {}
       //                                                ↑
       //                                           10 000 ms = 10 s

       void onTick() override {
           std::cout << "État : en fonctionnement." << std::endl;
           // publier l'état, mettre à jour une base de données, etc.
       }
   };

----

Combiner plusieurs behaviours
------------------------------

Un agent peut avoir autant de behaviours que nécessaire. Ils s'exécutent
tous **en même temps**, chacun de façon indépendante.

**Exemple — un agent de supervision avec trois tâches simultanées :**

.. code-block:: cpp

   class AgentSupervision : public Agent {
   public:
       void setup() override {
           // Envoie une notification de démarrage (une seule fois)
           addBehaviour(new NotificationDemarrage(this));

           // Surveille la température en continu
           addBehaviour(new SurveillanceBehaviour(this));

           // Publie un rapport toutes les 10 secondes
           addBehaviour(new PublicationEtat(this));
       }
   };

Les trois tâches tournent en parallèle dès que l'agent démarre.
L'agent s'arrête lorsque tous ses behaviours sont terminés — ici,
``SurveillanceBehaviour`` et ``PublicationEtat`` étant cycliques,
l'agent tourne jusqu'à Ctrl+C.

----

Résumé
------

.. list-table::
   :widths: 25 20 55
   :header-rows: 1

   * - Type
     - À implémenter
     - Comportement
   * - ``Behaviour``
     - ``action()``, ``done()``
     - S'arrête quand ``done()`` retourne ``true``
   * - ``OneShotBehaviour``
     - ``action()``
     - S'exécute une seule fois puis s'arrête
   * - ``CyclicBehaviour``
     - ``action()``
     - Tourne indéfiniment
   * - ``WakerBehaviour``
     - ``onWake()``
     - Attend N millisecondes, exécute ``onWake()``, s'arrête
   * - ``TickerBehaviour``
     - ``onTick()``
     - Exécute ``onTick()`` toutes les N millisecondes, indéfiniment
