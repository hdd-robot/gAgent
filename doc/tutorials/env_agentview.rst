13 — Visualiser avec agentview
===============================

``agentview`` est le serveur de visualisation web intégré à gAgent. Il
affiche en temps réel la carte de l'environnement et les agents qui s'y
déplacent, directement dans votre navigateur.

Lancer agentview
-----------------

.. code-block:: bash

   ./bin/agentview

Par défaut, ``agentview`` écoute sur le port **8080**. Ouvrez votre
navigateur à l'adresse :

.. code-block:: text

   http://localhost:8080

Vous verrez une carte vide. Dès que vous lancez une simulation avec un
environnement, les agents apparaissent et se déplacent en temps réel.

Ordre de lancement recommandé
--------------------------------

.. code-block:: bash

   # Terminal 1 — plateforme FIPA (optionnel, pour AMS/DF)
   ./bin/agentplatform

   # Terminal 2 — visualisation
   ./bin/agentview

   # Terminal 3 — votre simulation
   ./build/ma_simulation

Comment ça fonctionne
-----------------------

``agentview`` interroge l'environnement toutes les 100 ms via un socket
Unix en envoyant la commande ``GET_AGENTS``. L'environnement répond avec
un JSON décrivant tous les agents visuels :

.. code-block:: json

   {
     "width": 600,
     "height": 300,
     "agents": [
       {
         "id": "orbite",
         "name": "Orbite",
         "shape": "circle",
         "color": "#4fc3f7",
         "x": 342.5,
         "y": 198.3,
         "size_x": 24.0,
         "size_y": 24.0,
         "size": 1.0,
         "val": ""
       }
     ]
   }

``agentview`` dessine ensuite chaque agent sur le canvas HTML5 en
utilisant les attributs ``shape``, ``color``, ``x``, ``y``, et ``size``.

Personnaliser l'apparence des agents
--------------------------------------

Tout se passe dans les attributs que l'agent publie via ``setAttribut()``
et ``attributUpdated()``.

**Changer la forme :**

.. code-block:: cpp

   setAttribut("shape", "star");      // étoile violette
   setAttribut("shape", "diamond");   // losange orange
   setAttribut("shape", "triangle");  // triangle vert

**Changer la couleur :**

.. code-block:: cpp

   setAttribut("color", "#e94560");            // rouge vif
   setAttribut("color", "rgba(76,175,80,0.8)"); // vert semi-transparent

**Afficher une valeur textuelle :**

.. code-block:: cpp

   setAttribut("val", "42°C");   // affiché sous l'agent
   setAttribut("val", "danger"); // étiquette d'état

**Changer la taille en cours de simulation :**

.. code-block:: cpp

   void onTick() override {
       // L'agent grossit progressivement
       taille_ += 0.5f;
       char buf[16];
       snprintf(buf, sizeof(buf), "%.0f", taille_);
       this_agent->setAttribut("size_x", buf);
       this_agent->setAttribut("size_y", buf);
       set_pos(this_agent, x_, y_);
   }

Exemple — agent qui change de couleur selon son état
------------------------------------------------------

.. code-block:: cpp

   class AgentAlerte : public Agent {
       int energie_ = 100;
   public:
       void setup() override {
           addAttribut("id");    addAttribut("name");
           addAttribut("x");     addAttribut("y");
           addAttribut("shape"); addAttribut("color");
           addAttribut("size_x");addAttribut("size_y");
           addAttribut("val");

           setAttribut("id",     "alerte");
           setAttribut("name",   "Alerte");
           setAttribut("shape",  "circle");
           setAttribut("size_x", "20");
           setAttribut("size_y", "20");
           setAttribut("x",      "300");
           setAttribut("y",      "150");
           attributUpdated();

           addBehaviour(new AlerteBehaviour(this));
       }
   };

   class AlerteBehaviour : public TickerBehaviour {
       int& energie_;
   public:
       AlerteBehaviour(Agent* ag, int& e)
           : TickerBehaviour(ag, 500), energie_(e) {}

       void onTick() override {
           energie_--;

           // Couleur selon le niveau d'énergie
           if      (energie_ > 66) this_agent->setAttribut("color", "#4caf50"); // vert
           else if (energie_ > 33) this_agent->setAttribut("color", "#ff9800"); // orange
           else                    this_agent->setAttribut("color", "#e94560"); // rouge

           char buf[16];
           snprintf(buf, sizeof(buf), "%d%%", energie_);
           this_agent->setAttribut("val", buf);
           this_agent->attributUpdated();

           if (energie_ <= 0) done_ = true;
       }
   };

Dans ``agentview``, vous verrez l'agent passer du vert à l'orange puis
au rouge, avec le pourcentage affiché en direct.

Ce qu'affiche agentview
------------------------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Élément affiché
     - Source
   * - Forme de l'agent
     - attribut ``shape``
   * - Couleur
     - attribut ``color``
   * - Position
     - attributs ``x``, ``y``
   * - Taille
     - attributs ``size_x`` / ``size_y`` (ou ``size``)
   * - Étiquette texte
     - attribut ``val``
   * - Nom au survol
     - attribut ``name``
   * - Dimensions de la carte
     - ``map_width`` / ``map_height`` dans ``init_env()``
