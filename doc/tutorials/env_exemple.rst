12 — Coder un environnement simple
====================================

Dans ce tutoriel, vous allez créer un environnement 2D avec trois agents
qui se déplacent : un qui tourne en cercle, un qui rebondit
horizontalement, et un qui patrouille verticalement.

L'environnement
----------------

Commencez par définir votre environnement. La carte fait 600 × 300 pixels,
et les attributs s'appellent ``x``, ``y``, ``color``, etc.

.. code-block:: cpp

   #include <gagent/core/AgentCore.hpp>
   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/env/Environnement.hpp>
   #include <cmath>
   #include <thread>
   #include <chrono>
   #include <iostream>

   using namespace gagent;

   class Terrain : public Environnement {
   public:
       void init_env() override {
           map_width  = 600;
           map_height = 300;
           std::cout << "[Env] terrain " << map_width
                     << "x" << map_height << std::endl;
       }

       void link_attribut() override {
           link_id    ("id");
           link_name  ("name");
           link_pos_x ("x");
           link_pos_y ("y");
           link_shape ("shape");
           link_color ("color");
           link_size_x("size_x");
           link_size_y("size_y");
       }

       void event_loop() override {
           while (true) {
               make_agent();  // rafraîchir la vue toutes les 100 ms
               std::this_thread::sleep_for(std::chrono::milliseconds(100));
           }
       }
   };

Fonctions utilitaires
----------------------

Ces deux fonctions factorisent l'initialisation et la mise à jour de
position — vous les réutiliserez dans chaque agent.

.. code-block:: cpp

   // Initialise les attributs visuels d'un agent
   void init_visual(Agent* ag,
                    const std::string& id,
                    const std::string& name,
                    const std::string& shape,
                    const std::string& color,
                    float sx, float sy)
   {
       ag->addAttribut("id");     ag->addAttribut("name");
       ag->addAttribut("x");      ag->addAttribut("y");
       ag->addAttribut("shape");  ag->addAttribut("color");
       ag->addAttribut("size_x"); ag->addAttribut("size_y");

       ag->setAttribut("id",     id);
       ag->setAttribut("name",   name);
       ag->setAttribut("shape",  shape);
       ag->setAttribut("color",  color);

       char buf[16];
       snprintf(buf, sizeof(buf), "%.0f", sx);
       ag->setAttribut("size_x", buf);
       snprintf(buf, sizeof(buf), "%.0f", sy);
       ag->setAttribut("size_y", buf);
   }

   // Met à jour la position et publie l'état
   void set_pos(Agent* ag, float x, float y)
   {
       char bx[16], by[16];
       snprintf(bx, sizeof(bx), "%.1f", x);
       snprintf(by, sizeof(by), "%.1f", y);
       ag->setAttribut("x", bx);
       ag->setAttribut("y", by);
       ag->attributUpdated();
   }

Les behaviours de déplacement
--------------------------------

Chaque behaviour hérite de ``TickerBehaviour`` pour se déclencher
périodiquement.

**Orbite circulaire :**

.. code-block:: cpp

   class OrbitBehaviour : public TickerBehaviour {
       float t_, cx_, cy_, r_;
   public:
       OrbitBehaviour(Agent* ag, float cx, float cy, float r)
           : TickerBehaviour(ag, 60), t_(0), cx_(cx), cy_(cy), r_(r) {}

       void onTick() override {
           t_ += 0.06f;
           set_pos(this_agent,
                   cx_ + r_ * std::cos(t_),
                   cy_ + r_ * std::sin(t_));
       }
   };

**Rebond horizontal :**

.. code-block:: cpp

   class BounceBehaviour : public TickerBehaviour {
       float x_, y_, dx_, xmin_, xmax_;
   public:
       BounceBehaviour(Agent* ag, float x, float y, float dx,
                       float xmin, float xmax)
           : TickerBehaviour(ag, 40),
             x_(x), y_(y), dx_(dx), xmin_(xmin), xmax_(xmax) {}

       void onTick() override {
           x_ += dx_;
           if (x_ < xmin_ || x_ > xmax_) dx_ = -dx_;
           set_pos(this_agent, x_, y_);
       }
   };

**Patrouille verticale :**

.. code-block:: cpp

   class PatrolBehaviour : public TickerBehaviour {
       float x_, y_, dy_, ymin_, ymax_;
   public:
       PatrolBehaviour(Agent* ag, float x, float y, float dy,
                       float ymin, float ymax)
           : TickerBehaviour(ag, 50),
             x_(x), y_(y), dy_(dy), ymin_(ymin), ymax_(ymax) {}

       void onTick() override {
           y_ += dy_;
           if (y_ < ymin_ || y_ > ymax_) dy_ = -dy_;
           set_pos(this_agent, x_, y_);
       }
   };

Les agents
-----------

Chaque agent initialise ses attributs visuels dans ``setup()``, puis
ajoute son behaviour de déplacement.

.. code-block:: cpp

   class AgentOrbite : public Agent {
   public:
       void setup() override {
           init_visual(this, "orbite", "Orbite",
                       "circle", "#4fc3f7", 24, 24);
           attributUpdated();
           addBehaviour(new OrbitBehaviour(this, 300, 150, 100));
       }
   };

   class AgentRebond : public Agent {
   public:
       void setup() override {
           init_visual(this, "rebond", "Rebond",
                       "square", "#e94560", 22, 22);
           attributUpdated();
           addBehaviour(new BounceBehaviour(this, 50, 80, 5, 20, 580));
       }
   };

   class AgentPatrouille : public Agent {
   public:
       void setup() override {
           init_visual(this, "patrouille", "Patrouille",
                       "triangle", "#4caf50", 24, 24);
           attributUpdated();
           addBehaviour(new PatrolBehaviour(this, 500, 30, 3, 20, 280));
       }
   };

Le ``main``
------------

.. code-block:: cpp

   int main() {
       AgentCore::initAgentSystem();

       Terrain env;
       AgentCore::initEnvironnementSystem(env);

       AgentOrbite     a1; a1.init();
       AgentRebond     a2; a2.init();
       AgentPatrouille a3; a3.init();

       AgentCore::syncAgentSystem();
       return 0;
   }

Lancer la simulation
---------------------

.. code-block:: bash

   # Terminal 1 — visualisation web
   ./bin/agentview

   # Terminal 2 — lancer la simulation
   ./build/ma_simulation

Ouvrez ``http://localhost:8080`` dans votre navigateur : les trois agents
apparaissent sur la carte et se déplacent en temps réel.

.. note::

   ``initEnvironnementSystem()`` doit être appelé **avant** les
   ``init()`` des agents. L'environnement doit être prêt à recevoir
   les premiers ``attributUpdated()`` dès le démarrage des agents.

Les formes disponibles
-----------------------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Valeur ``shape``
     - Rendu dans agentview
   * - ``circle``
     - Cercle
   * - ``square``
     - Carré
   * - ``triangle``
     - Triangle
   * - ``diamond``
     - Losange
   * - ``star``
     - Étoile

Les couleurs acceptent n'importe quelle valeur CSS : ``"red"``,
``"#4fc3f7"``, ``"rgba(255, 100, 0, 0.8)"``.
