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
   * - ``SequentialBehaviour``
     - Enchaîner plusieurs sous-behaviours dans l'ordre
   * - ``ParallelBehaviour``
     - Exécuter plusieurs sous-behaviours en même temps
   * - ``FSMBehaviour``
     - Machine à états finis — un état = un behaviour

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

Behaviours composites
----------------------

Les behaviours composites permettent d'**orchestrer** plusieurs sous-tâches
à l'intérieur d'un seul behaviour. Ils s'ajoutent à l'agent comme n'importe
quel autre behaviour, mais délèguent leur exécution à des enfants.

.. note::

   Les sous-behaviours d'un composite s'exécutent dans le **même thread**
   que le composite lui-même. Pour une vraie exécution en parallèle sur des
   threads séparés, utilisez ``addBehaviour()`` directement dans ``setup()``.

``SequentialBehaviour`` — enchaîner des étapes dans l'ordre
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Exécute les sous-behaviours **un par un** dans l'ordre où ils ont été
ajoutés. Dès qu'un enfant est terminé (``done()`` retourne ``true``),
le suivant démarre.

**Exemple — processus en trois étapes :**

.. code-block:: cpp

   class EtapeInit : public OneShotBehaviour {
   public:
       EtapeInit(Agent* ag) : OneShotBehaviour(ag) {}
       void action() override {
           std::cout << "Étape 1 : initialisation" << std::endl;
       }
   };

   class EtapeTraitement : public Behaviour {
       int steps_ = 0;
   public:
       EtapeTraitement(Agent* ag) : Behaviour(ag) {}
       void action() override {
           std::cout << "Étape 2 : traitement " << ++steps_ << "/3" << std::endl;
           sleep(1);
       }
       bool done() override { return steps_ >= 3; }
   };

   class EtapeConclusion : public OneShotBehaviour {
   public:
       EtapeConclusion(Agent* ag) : OneShotBehaviour(ag) {}
       void action() override {
           std::cout << "Étape 3 : rapport final" << std::endl;
       }
   };

   class MonAgent : public Agent {
   public:
       void setup() override {
           auto* seq = new SequentialBehaviour(this);
           seq->addSubBehaviour(new EtapeInit(this));
           seq->addSubBehaviour(new EtapeTraitement(this));
           seq->addSubBehaviour(new EtapeConclusion(this));
           addBehaviour(seq);
       }
   };

``ParallelBehaviour`` — plusieurs tâches en un seul behaviour
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Appelle ``action()`` sur tous les enfants à chaque tick. La condition
de fin est configurable :

- ``WhenDone::ALL`` (défaut) — attend que **tous** les enfants aient fini
- ``WhenDone::ANY`` — termine dès qu'**un** enfant est terminé

**Exemple — attendre que deux capteurs soient prêts :**

.. code-block:: cpp

   class LectureCapteur : public Behaviour {
       std::string nom_;
       int lectures_ = 0;
   public:
       LectureCapteur(Agent* ag, std::string nom)
           : Behaviour(ag), nom_(std::move(nom)) {}

       void action() override {
           std::cout << nom_ << " : lecture " << ++lectures_ << std::endl;
           sleep(1);
       }
       bool done() override { return lectures_ >= 5; }
   };

   class MonAgent : public Agent {
   public:
       void setup() override {
           // Termine quand les DEUX capteurs ont fait 5 lectures
           auto* par = new ParallelBehaviour(this, ParallelBehaviour::WhenDone::ALL);
           par->addSubBehaviour(new LectureCapteur(this, "Capteur-A"));
           par->addSubBehaviour(new LectureCapteur(this, "Capteur-B"));
           addBehaviour(par);
       }
   };

``FSMBehaviour`` — machine à états finis
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Chaque état est un behaviour. Quand un état est terminé, sa méthode
``exitValue()`` détermine la prochaine transition.

**Exemple simple — feu de signalisation (Rouge → Vert → Orange) :**

.. code-block:: cpp

   class EtatFeu : public OneShotBehaviour {
       std::string couleur_;
       int exit_val_;
   public:
       EtatFeu(Agent* ag, std::string couleur, int exit_val)
           : OneShotBehaviour(ag), couleur_(std::move(couleur))
           , exit_val_(exit_val) {}

       void action() override {
           std::cout << "Feu : " << couleur_ << std::endl;
           sleep(2);
       }
       int exitValue() override { return exit_val_; }
   };

   class AgentFeu : public Agent {
   public:
       void setup() override {
           auto* fsm = new FSMBehaviour(this);

           fsm->registerFirstState(new EtatFeu(this, "ROUGE",  0), "rouge");
           fsm->registerState     (new EtatFeu(this, "VERT",   0), "vert");
           fsm->registerLastState (new EtatFeu(this, "ORANGE", 0), "orange");

           fsm->registerDefaultTransition("rouge",  "vert");
           fsm->registerDefaultTransition("vert",   "orange");
           // orange est un état "last" : plus de transition nécessaire

           addBehaviour(fsm);
       }
   };

Pour une FSM qui boucle indéfiniment, utilisez des états normaux (non
``last``) avec une transition qui revient au départ :

.. code-block:: cpp

   fsm->registerState(new EtatFeu(this, "ROUGE",  0), "rouge");
   fsm->registerState(new EtatFeu(this, "VERT",   0), "vert");
   fsm->registerState(new EtatFeu(this, "ORANGE", 0), "orange");

   fsm->registerDefaultTransition("rouge",  "vert");
   fsm->registerDefaultTransition("vert",   "orange");
   fsm->registerDefaultTransition("orange", "rouge");   // ← boucle

   fsm->registerFirstState(/* même état */ ..., "rouge");

----

**Exemple avec graphe — distributeur automatique**

Le graphe ci-dessous montre les états et les transitions conditionnelles.
Les labels sur les flèches correspondent aux valeurs retournées par
``exitValue()``.

.. graphviz::

   digraph distributeur {
       rankdir=LR;
       node [shape=rectangle, style=rounded, fontname="sans-serif"];
       edge [fontname="sans-serif", fontsize=10];

       start  [shape=point, width=0.2];
       DIST   [label="DISTRIBUTION\n(last)", shape=doublecircle];
       ANNUL  [label="ANNULATION\n(last)",   shape=doublecircle];

       start       -> ATTENTE;
       ATTENTE     -> SELECTION    [label=" 0 : monnaie insérée"];
       SELECTION   -> VERIFICATION [label=" 0 : produit choisi"];
       SELECTION   -> ANNULATION   [label=" 1 : annulé"];
       VERIFICATION -> DISTRIBUTION [label=" 0 : en stock"];
       VERIFICATION -> ANNULATION  [label=" 1 : rupture"];
       ANNULATION  -> ATTENTE      [label=" 0 (défaut)"];
   }

Correspondance graphe → code :

.. code-block:: cpp

   // ── États ────────────────────────────────────────────────────────────────

   // Attend qu'une pièce soit insérée (simulé par un délai).
   // exitValue 0 = pièce détectée
   class EtatAttente : public Behaviour {
       int ticks_ = 0;
   public:
       EtatAttente(Agent* ag) : Behaviour(ag) {}
       void onStart() override {
           std::cout << "[distributeur] En attente d'une pièce...\n";
           ticks_ = 0;
       }
       void action() override { sleep(1); ticks_++; }
       bool done()   override { return ticks_ >= 2; }    // 2s = pièce insérée
       int  exitValue() override { return 0; }
   };

   // Simule le choix d'un produit.
   // exitValue 0 = produit sélectionné, 1 = annulé
   class EtatSelection : public Behaviour {
       int choix_ = 0;
   public:
       EtatSelection(Agent* ag) : Behaviour(ag) {}
       void onStart() override {
           std::cout << "[distributeur] Sélection du produit...\n";
           choix_ = 0;
       }
       void action() override { sleep(1); choix_++; }
       bool done()   override { return choix_ >= 1; }
       // 0 = produit choisi, 1 = annulé (simulé selon la parité)
       int  exitValue() override { return (choix_ % 2 == 0) ? 1 : 0; }
   };

   // Vérifie le stock.
   // exitValue 0 = en stock, 1 = rupture
   class EtatVerification : public OneShotBehaviour {
       bool en_stock_;
   public:
       EtatVerification(Agent* ag, bool en_stock)
           : OneShotBehaviour(ag), en_stock_(en_stock) {}
       void action() override {
           std::cout << "[distributeur] Vérification stock : "
                     << (en_stock_ ? "OK" : "rupture") << "\n";
       }
       int exitValue() override { return en_stock_ ? 0 : 1; }
   };

   class EtatDistribution : public OneShotBehaviour {
   public:
       EtatDistribution(Agent* ag) : OneShotBehaviour(ag) {}
       void action() override {
           std::cout << "[distributeur] Produit distribué !\n";
       }
   };

   class EtatAnnulation : public OneShotBehaviour {
   public:
       EtatAnnulation(Agent* ag) : OneShotBehaviour(ag) {}
       void action() override {
           std::cout << "[distributeur] Monnaie rendue, opération annulée.\n";
       }
       int exitValue() override { return 0; }   // → retour ATTENTE
   };

   // ── Agent ─────────────────────────────────────────────────────────────────

   class AgentDistributeur : public Agent {
   public:
       void setup() override {
           auto* fsm = new FSMBehaviour(this);

           // États
           fsm->registerFirstState(new EtatAttente(this),              "attente");
           fsm->registerState     (new EtatSelection(this),            "selection");
           fsm->registerState     (new EtatVerification(this, true),   "verification");
           fsm->registerLastState (new EtatDistribution(this),         "distribution");
           fsm->registerState     (new EtatAnnulation(this),           "annulation");

           // Transitions depuis ATTENTE
           fsm->registerTransition("attente",      "selection",    0); // pièce OK

           // Transitions depuis SELECTION
           fsm->registerTransition("selection",    "verification", 0); // produit choisi
           fsm->registerTransition("selection",    "annulation",   1); // annulé

           // Transitions depuis VERIFICATION
           fsm->registerTransition("verification", "distribution", 0); // en stock
           fsm->registerTransition("verification", "annulation",   1); // rupture

           // ANNULATION retourne à ATTENTE
           fsm->registerDefaultTransition("annulation", "attente");

           addBehaviour(fsm);
       }
   };

.. note::

   ``registerLastState`` marque ``distribution`` comme état terminal :
   la FSM s'arrête dès que cet état est terminé.
   ``annulation`` est un état ordinaire avec une transition de retour
   vers ``attente`` — la machine continue à servir.

----

Résumé
------

.. list-table::
   :widths: 28 22 50
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
   * - ``SequentialBehaviour``
     - ``addSubBehaviour()``
     - Enchaîne les enfants dans l'ordre
   * - ``ParallelBehaviour``
     - ``addSubBehaviour()``
     - Tick tous les enfants à chaque tour (ALL ou ANY)
   * - ``FSMBehaviour``
     - ``registerFirstState()``, ``registerTransition()``
     - Machine à états, transitions par ``exitValue()``
