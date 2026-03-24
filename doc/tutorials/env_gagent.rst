11 — L'environnement dans gAgent
=================================

gAgent fournit deux classes pour modéliser l'environnement situé :
``Environnement`` et ``VisualAgent``.

La classe ``Environnement``
-----------------------------

``Environnement`` est la classe de base à surcharger. Elle gère
automatiquement la collecte des états d'agents, la mémoire partagée,
et la communication avec ``agentview``.

.. code-block:: cpp

   #include <gagent/env/Environnement.hpp>
   using namespace gagent;

   class MonEnvironnement : public Environnement {
   public:
       void init_env()      override { /* initialisation */ }
       void link_attribut() override { /* déclarer les attributs surveillés */ }
       void event_loop()    override { /* boucle principale */ }
   };

Les trois méthodes à surcharger
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**``init_env()``** — appelée une fois au démarrage. Configurez ici les
dimensions de la carte et toute initialisation de votre environnement.

.. code-block:: cpp

   void init_env() override {
       map_width  = 800;   // largeur de la carte en pixels
       map_height = 400;   // hauteur de la carte
   }

**``link_attribut()``** — déclare la correspondance entre les noms
d'attributs de vos agents et les champs internes de l'environnement.
C'est ici que vous dites "dans mes agents, la position X s'appelle
``x``, la couleur s'appelle ``color``".

.. code-block:: cpp

   void link_attribut() override {
       link_id    ("id");     // identifiant unique de l'agent
       link_name  ("name");   // nom affiché
       link_pos_x ("x");      // position horizontale
       link_pos_y ("y");      // position verticale
       link_shape ("shape");  // forme : circle, square, triangle, diamond, star
       link_color ("color");  // couleur CSS : "red", "#4fc3f7", etc.
       link_size_x("size_x"); // largeur en pixels
       link_size_y("size_y"); // hauteur en pixels
       link_val   ("val");    // valeur textuelle affichée
   }

**``event_loop()``** — la boucle principale de l'environnement. Elle
tourne en continu tant que la simulation fonctionne. C'est ici que vous
mettez à jour l'état de l'environnement, appelez ``make_agent()`` pour
rafraîchir la vue, etc.

.. code-block:: cpp

   void event_loop() override {
       while (true) {
           make_agent();  // rafraîchir la liste des agents visuels
           std::this_thread::sleep_for(std::chrono::milliseconds(100));
       }
   }

La méthode ``make_agent()``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

``make_agent()`` reconstruit la liste interne ``list_visual_agents`` à
partir des derniers attributs reçus de tous les agents. C'est cette
liste qu'``agentview`` lit pour afficher la carte.

Appelez-la régulièrement dans ``event_loop()`` pour que la visualisation
reste à jour.

La classe ``VisualAgent``
---------------------------

Un ``VisualAgent`` est la représentation visuelle d'un agent dans
l'environnement. Il contient les attributs d'affichage :

.. list-table::
   :widths: 20 20 60
   :header-rows: 1

   * - Attribut
     - Type
     - Description
   * - ``id``
     - string
     - Identifiant unique (doit être unique dans l'environnement)
   * - ``name``
     - string
     - Nom affiché dans agentview
   * - ``pos_x``, ``pos_y``
     - float
     - Position dans la carte (en pixels)
   * - ``shape``
     - string
     - Forme : ``circle``, ``square``, ``triangle``, ``diamond``, ``star``
   * - ``color``
     - string
     - Couleur CSS : ``"red"``, ``"#4fc3f7"``, ``"rgba(255,100,0,0.8)"``
   * - ``size``
     - float
     - Taille uniforme (en pixels)
   * - ``size_x``, ``size_y``
     - float
     - Dimensions indépendantes (largeur × hauteur)
   * - ``val``
     - string
     - Valeur textuelle affichée sur ou sous l'agent

Comment les agents publient leur état
---------------------------------------

Un agent situé déclare ses attributs visuels dans ``setup()``, puis les
met à jour au fil du temps en appelant ``attributUpdated()``.

.. code-block:: cpp

   class MonAgent : public Agent {
   public:
       void setup() override {
           // 1. Déclarer les attributs
           addAttribut("id");     addAttribut("name");
           addAttribut("x");      addAttribut("y");
           addAttribut("shape");  addAttribut("color");
           addAttribut("size_x"); addAttribut("size_y");

           // 2. Initialiser les valeurs
           setAttribut("id",     "agent-1");
           setAttribut("name",   "Agent 1");
           setAttribut("shape",  "circle");
           setAttribut("color",  "#4fc3f7");
           setAttribut("size_x", "20");
           setAttribut("size_y", "20");
           setAttribut("x",      "100");
           setAttribut("y",      "150");

           // 3. Publier l'état initial
           attributUpdated();
       }
   };

À chaque appel à ``attributUpdated()``, l'agent envoie ses attributs
courants à l'environnement via la file de messages interne. L'environnement
met alors à jour sa vue.

Lancer l'environnement
------------------------

L'environnement se lance avec ``AgentCore::initEnvironnementSystem()``
**avant** d'initialiser les agents :

.. code-block:: cpp

   int main() {
       AgentCore::initAgentSystem();

       MonEnvironnement env;
       AgentCore::initEnvironnementSystem(env);  // lance l'environnement

       MonAgent a;
       a.init();

       AgentCore::syncAgentSystem();
       return 0;
   }

``initEnvironnementSystem()`` démarre l'environnement dans un processus
séparé (comme les agents). La méthode ``start()`` est appelée
automatiquement — elle exécute ``init_env()``, ``link_attribut()``, puis
``event_loop()`` de façon bloquante.

Résumé du flux
---------------

.. code-block:: text

   Agent                 Environnement              agentview
     │                       │                          │
     │  setAttribut(...)      │                          │
     │  attributUpdated() ───►│                          │
     │                        │  make_agent()            │
     │                        │  (reconstruit la vue)    │
     │                        │◄──── GET_AGENTS ─────────│
     │                        │───── JSON ──────────────►│
     │                        │                    affiche la carte
