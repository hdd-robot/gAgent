Visualisation web — agentview
==============================

``agentview`` est un serveur HTTP minimal (zéro dépendance externe) qui expose
une page de visualisation SVG temps réel de l'environnement gAgent.

.. code-block:: text

   ┌──────────────────────────────────────────────────┐
   │  Navigateur  GET /api/agents  ─►  agentview       │
   │     ↑                              │               │
   │  SVG rendu                   EnvClient             │
   │     ↑                        AMSClient             │
   │  polling JS 500 ms               │                 │
   └─────────────────────────────────┼─────────────────┘
                                     ▼
                        /tmp/gagent_env.sock  (Environnement)
                        /tmp/gagent_ams.sock  (AMS)


Démarrage
---------

.. code-block:: bash

   # Lancer la simulation (produit le socket /tmp/gagent_env.sock)
   ./mon_application

   # Dans un autre terminal :
   ./agentview            # port 8080 par défaut
   ./agentview 9090       # port personnalisé

   # Ouvrir dans le navigateur :
   #   http://localhost:8080

Variables d'environnement
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Variable
     - Rôle
   * - ``GAGENT_VIEW_PORT``
     - Port HTTP (défaut : ``8080``)
   * - ``GAGENT_ENV_SOCK``
     - Chemin du socket Environnement (défaut : ``/tmp/gagent_env.sock``)
   * - ``GAGENT_AMS_SOCK``
     - Chemin du socket AMS (défaut : ``/tmp/gagent_ams.sock``)


API HTTP
--------

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Route
     - Réponse
   * - ``GET /``
     - Page HTML (visualisation SVG + JS)
   * - ``GET /api/agents``
     - JSON — liste des agents visuels (Environnement)
   * - ``GET /api/ams``
     - JSON — liste des agents enregistrés (AMS)
   * - ``GET /api/nsap``
     - JSON — pile de snapshots NSAP

Tous les endpoints JSON incluent ``Access-Control-Allow-Origin: *``.

Format ``/api/agents``
~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: json

   {
     "width": 600,
     "height": 300,
     "agents": [
       {
         "id":      "a1",
         "name":    "alice",
         "shape":   "circle",
         "color":   "#e94560",
         "pattern": "",
         "x":       120.0,
         "y":       80.0,
         "size_x":  20.0,
         "size_y":  20.0,
         "size_z":  10.0,
         "size":    10.0,
         "val":     "en attente"
       }
     ]
   }

Format ``/api/ams``
~~~~~~~~~~~~~~~~~~~~

.. code-block:: json

   {
     "agents": [
       {"name": "alice", "pid": 4521, "address": "/acl_alice", "state": "active"},
       {"name": "bob",   "pid": 4522, "address": "/acl_bob",   "state": "suspended"}
     ]
   }


Shapes SVG supportées
---------------------

.. list-table::
   :header-rows: 1
   :widths: 20 80

   * - Valeur ``shape``
     - Rendu
   * - ``circle``
     - Cercle centré sur ``(x, y)``, rayon ``max(size_x, size_y)/2``
   * - ``square``
     - Rectangle ``size_x × size_y`` centré sur ``(x, y)``
   * - ``triangle``
     - Triangle isocèle pointant vers le haut
   * - ``diamond``
     - Losange (4 sommets cardinaux)
   * - ``hexagon``
     - Hexagone régulier
   * - ``star``
     - Étoile à 5 branches
   * - ``M...``
     - Chemin SVG arbitraire (doit commencer par ``M``) — appliqué avec ``translate(x, y)``

Si ``shape`` est vide ou non reconnu, ``circle`` est utilisé par défaut.


Interface utilisateur
---------------------

.. code-block:: text

   ┌─ header ──────────────────────────────────────────┐
   │ gAgent Visualization   3 agents    ● live          │
   └────────────────────────────────────────────────────┘
   ┌─ canvas SVG ──────────────────┐ ┌─ sidebar ───────┐
   │                               │ │ Agents visuels  │
   │   ●alice   ■bob   ◆carol      │ │ ● alice  ok     │
   │                               │ │ ■ bob    wait   │
   │                               │ │ ◆ carol         │
   │                               │ ├─────────────────┤
   │                               │ │ AMS             │
   │                               │ │ alice  active   │
   │                               │ │ bob    suspended│
   │                               │ ├─────────────────┤
   │                               │ │ Rafraîchissement│
   │                               │ │ ──●──  500 ms   │
   └───────────────────────────────┘ └─────────────────┘

- Le canvas SVG s'adapte aux dimensions déclarées par ``Environnement``
  (``map_width`` × ``map_height``)
- Le taux de rafraîchissement est réglable de 100 ms à 5 s via un slider
- En mode dégradé (env socket absent), le canvas est vide et le statut
  affiche ``env offline``


Intégration dans l'Environnement
---------------------------------

``Environnement::serve()`` est démarré automatiquement par
``AgentCore::initEnvironnementSystem()`` dans le processus enfant, juste
avant ``event_loop()``.

.. code-block:: cpp

   class MyEnv : public gagent::Environnement {
   public:
       void init_env() override {
           // Configurer les dimensions de la carte
           map_width  = 800;
           map_height = 400;
       }
       void link_attribut() override {
           link_id("id");
           link_name("name");
           link_pos_x("pos_x");
           link_pos_y("pos_y");
           link_shape("shape");
           link_color("color");
           link_size_x("size_x");
           link_size_y("size_y");
       }
       void event_loop() override {
           while (true) {
               make_agent();     // relit list_attr → list_visual_agents
               sleep(1);
           }
       }
   };

   int main() {
       AgentCore::initAgentSystem();
       MyEnv env;
       AgentCore::initEnvironnementSystem(env);  // fork + serve() auto

       // ...agents...

       AgentCore::syncAgentSystem();
   }

Le socket Environnement est créé à ``/tmp/gagent_env.sock`` par défaut
(surchargeable via ``GAGENT_ENV_SOCK``).

.. note::

   ``agentview`` et ``agentplatform`` sont indépendants. On peut lancer
   ``agentview`` sans ``agentplatform`` (le panneau AMS sera vide) et
   inversement.


Protocole du socket Environnement
----------------------------------

``EnvClient`` (dans ``libgagent``) peut être utilisé depuis n'importe quel
programme C++ pour interroger l'Environnement :

.. code-block:: cpp

   #include <gagent/platform/EnvClient.hpp>

   gagent::platform::EnvClient env;
   std::string json = env.getAgents();  // JSON complet
   std::string nsap = env.getNsap();    // JSON pile NSAP

Protocole bas niveau (ligne de commande, une connexion par requête) :

.. code-block:: text

   → GET_AGENTS
   ← {"width":600,"height":300,"agents":[...]}

   → GET_NSAP
   ← {"count":2,"snaps":[{"seq":0,"timestamp":"2026-03-23T14:00:00.000"},...]}

Exemple en Python :

.. code-block:: python

   import socket, json

   def env_request(cmd):
       s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
       s.connect('/tmp/gagent_env.sock')
       s.sendall((cmd + '\n').encode())
       data = b''
       s.settimeout(1.0)
       try:
           while True:
               chunk = s.recv(4096)
               if not chunk: break
               data += chunk
       except: pass
       s.close()
       return json.loads(data.decode().strip())

   agents = env_request('GET_AGENTS')
   print(f"{len(agents['agents'])} agent(s) dans l'environnement")
