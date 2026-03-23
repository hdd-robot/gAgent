Agents Python et LLM
=====================

``PythonBehaviour`` permet de déléguer la logique d'un agent C++ à un script
Python. Le script peut appeler n'importe quel LLM (OpenAI, Anthropic, Ollama…)
pour prendre des décisions basées sur des messages FIPA ACL.

Architecture
------------

.. code-block:: text

   Agent C++ (cycle de vie FIPA)
   ├── enregistré dans AMS / DF
   ├── reçoit / envoie des ACLMessage
   ├── gère suspend / wake / delete
   │
   └── PythonBehaviour::action()
           │  stdin / stdout  (JSON newline-delimited)
           ▼
       Script Python
       ├── reçoit les événements (message, tick…)
       ├── maintient l'historique de conversation
       ├── appelle un LLM
       └── retourne une action (send, delete, noop…)

Le processus Python est spawné par ``fork + exec`` au démarrage du behaviour.
La communication est synchrone : un événement → une action.

Protocole JSON
--------------

**C++ → Python** (une ligne JSON par événement sur stdin) :

.. code-block:: json

   {"event":"start","system_prompt":"Tu es...","model":"gpt-4o-mini","max_tokens":200,"max_history":20}
   {"event":"message","msg":{"performative":"request","sender":"alice","content":"...","conversation_id":"..."}}
   {"event":"tick"}
   {"event":"stop"}

**Python → C++** (une ligne JSON par action sur stdout) :

.. code-block:: json

   {"action":"noop"}
   {"action":"send","to":"bob","msg":{"performative":"inform","content":"..."}}
   {"action":"delete"}
   {"action":"suspend"}
   {"action":"wake"}

PythonBehaviour (C++)
---------------------

Inclusion :

.. code-block:: cpp

   #include <gagent/python/PythonBehaviour.hpp>
   using namespace gagent::python;

Constructeur :

.. code-block:: cpp

   PythonBehaviour(Agent*             ag,
                   const std::string& my_name,
                   const std::string& script_path,
                   const std::string& system_prompt = "",
                   const std::string& model         = "gpt-4o-mini",
                   int                max_tokens     = 200,
                   int                max_history    = 20,
                   int                tick_ms        = 200);

.. list-table::
   :header-rows: 1
   :widths: 25 75

   * - Paramètre
     - Description
   * - ``my_name``
     - Nom de la queue ACL (``/acl_<my_name>``) — pour recevoir des messages
   * - ``script_path``
     - Chemin vers le script Python à exécuter
   * - ``system_prompt``
     - Prompt système envoyé au LLM — définit le rôle / la personnalité de l'agent
   * - ``model``
     - Modèle LLM (transmis au script Python via l'événement ``start``)
   * - ``max_tokens``
     - Longueur maximale de la réponse LLM
   * - ``max_history``
     - Nombre de tours de conversation conservés en mémoire
   * - ``tick_ms``
     - Intervalle en ms entre deux ticks quand aucun message n'arrive

Exemple d'agent C++ :

.. code-block:: cpp

   class MonAgent : public Agent {
       std::string script_;
   public:
       explicit MonAgent(std::string script) : script_(std::move(script)) {}

       void setup() override {
           addBehaviour(new PythonBehaviour(
               this, "mon-agent", script_,
               "Tu es un agent spécialisé en planification logistique.",
               "gpt-4o-mini", 300, 20
           ));
       }

       void takeDown() override {
           messaging::acl_unlink("mon-agent");
       }
   };

Librairie Python — gagent_py
-----------------------------

Le package ``python/gagent_py/`` fournit la classe de base ``Agent`` à utiliser
dans les scripts Python.

Utilisation :

.. code-block:: python

   import sys, os
   sys.path.insert(0, "/chemin/vers/gAgent/python")
   import gagent_py

Classe Agent
~~~~~~~~~~~~

.. code-block:: python

   class MonAgent(gagent_py.Agent):

       def on_start(self):
           # self.system_prompt, self.model, self.max_tokens,
           # self.max_history sont disponibles ici
           pass

       def on_message(self, msg: gagent_py.ACLMessage):
           if msg.performative == "request":
               return self.reply(msg, "inform", "réponse")
           if msg.performative == "cancel":
               return self.delete()
           return self.noop()

       def on_tick(self):
           return self.noop()

   MonAgent().run()

Méthodes à surcharger :

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Méthode
     - Description
   * - ``on_start()``
     - Appelé une fois au démarrage. ``self.config`` est disponible.
   * - ``on_message(msg)``
     - Appelé à chaque message ACL reçu.
   * - ``on_tick()``
     - Appelé périodiquement quand aucun message n'arrive.
   * - ``on_stop()``
     - Appelé avant l'arrêt propre.

Helpers d'action :

.. list-table::
   :header-rows: 1
   :widths: 40 60

   * - Méthode
     - Description
   * - ``send(to, performative, content, **kwargs)``
     - Envoyer un message ACL à un agent.
   * - ``reply(original, performative, content, **kwargs)``
     - Répondre en conservant le ``conversation_id``.
   * - ``noop()``
     - Ne rien faire (action par défaut).
   * - ``delete()``
     - Supprimer l'agent.
   * - ``suspend()`` / ``wake()``
     - Suspendre / réveiller l'agent.

Classe ACLMessage
~~~~~~~~~~~~~~~~~

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Attribut
     - Description
   * - ``performative``
     - Type du message (``"request"``, ``"inform"``, ``"cfp"``…)
   * - ``sender``
     - Nom de l'agent émetteur
   * - ``content``
     - Contenu du message
   * - ``ontology``
     - Ontologie utilisée
   * - ``language``
     - Langage du contenu
   * - ``conversation_id``
     - Identifiant de la conversation
   * - ``reply_with`` / ``in_reply_to``
     - Corrélation de réponse

Intégration LLM
---------------

Avec OpenAI :

.. code-block:: python

   import os
   from openai import OpenAI

   client = OpenAI(api_key=os.environ["OPENAI_API_KEY"])

   class MonAgent(gagent_py.Agent):

       def on_start(self):
           self._history = []

       def on_message(self, msg):
           if msg.performative != "request":
               return self.noop()

           self._history.append({"role": "user", "content": msg.content})

           # Limiter l'historique
           if len(self._history) > self.max_history * 2:
               self._history = self._history[-(self.max_history * 2):]

           response = client.chat.completions.create(
               model=self.model,
               messages=[
                   {"role": "system", "content": self.system_prompt},
               ] + self._history,
               max_tokens=self.max_tokens,
           )
           reply = response.choices[0].message.content.strip()
           self._history.append({"role": "assistant", "content": reply})

           return self.reply(msg, "inform", reply)

Avec Anthropic (Claude) :

.. code-block:: python

   import anthropic

   client = anthropic.Anthropic()

   class ClaudeAgent(gagent_py.Agent):
       def on_message(self, msg):
           if msg.performative != "request":
               return self.noop()
           response = client.messages.create(
               model="claude-haiku-4-5-20251001",
               max_tokens=self.max_tokens,
               system=self.system_prompt,
               messages=[{"role": "user", "content": msg.content}],
           )
           return self.reply(msg, "inform", response.content[0].text)

Avec Ollama (local, sans clé API) :

.. code-block:: python

   import requests

   def ask_ollama(prompt, model="llama3"):
       r = requests.post("http://localhost:11434/api/generate",
           json={"model": model, "prompt": prompt, "stream": False})
       return r.json()["response"]

   class OllamaAgent(gagent_py.Agent):
       def on_message(self, msg):
           if msg.performative != "request":
               return self.noop()
           return self.reply(msg, "inform", ask_ollama(msg.content))

Exemple complet
---------------

Voir ``examples/llm_agent.cpp`` et ``examples/llm_agent.py`` pour un exemple
fonctionnel : un ``TesterAgent`` envoie des questions en ACL REQUEST, le
``LLMAgent`` les traite via OpenAI et répond en INFORM.

.. code-block:: bash

   export OPENAI_API_KEY=sk-...
   ./build/examples/llm_agent

   # Sans LLM (mode echo)
   ./build/examples/llm_agent

Robustesse
----------

``PythonBehaviour`` gère les cas d'erreur :

- **SIGPIPE ignoré** — si Python meurt pendant un ``write()``, ``EPIPE`` est
  détecté et ``done_`` est mis à vrai (pas de crash du processus C++).
- **EOF sur stdout** — si Python ferme son stdout, ``read()`` retourne 0 et
  le behaviour se termine proprement.
- **Sortie inattendue** — ``waitpid(WNOHANG)`` vérifie à chaque tick que le
  processus Python est toujours vivant.
- **Timeout LLM** — les exceptions OpenAI/réseau sont catchées dans le script
  Python et retournent un message d'erreur sans rompre le protocole.
