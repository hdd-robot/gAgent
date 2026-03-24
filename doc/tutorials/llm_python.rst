16 — Écrire un agent Python
============================

Le script Python de votre agent hérite de ``gagent_py.Agent``. Cette
classe gère le protocole de communication avec le C++ (lecture JSON sur
``stdin``, écriture JSON sur ``stdout``) — vous ne vous occupez que de
la logique.

La bibliothèque gagent_py
--------------------------

``gagent_py`` est incluse dans le dépôt gAgent, dans le répertoire
``python/``.

.. code-block:: python

   import sys, os
   sys.path.insert(0, "/chemin/vers/dist/python")
   import gagent_py

Structure d'un agent Python
-----------------------------

.. code-block:: python

   class MonAgent(gagent_py.Agent):

       def on_start(self):
           """Appelé une fois au démarrage."""
           # self.system_prompt — prompt système configuré côté C++
           # self.model         — modèle LLM ("gpt-4o-mini", etc.)
           # self.max_tokens    — longueur max de réponse
           # self.max_history   — nombre de tours mémorisés
           pass

       def on_message(self, msg: gagent_py.ACLMessage):
           """Appelé à chaque message ACL reçu."""
           return self.noop()

       def on_tick(self):
           """Appelé périodiquement quand aucun message n'arrive."""
           return self.noop()

       def on_stop(self):
           """Appelé avant l'arrêt propre."""
           pass

   if __name__ == "__main__":
       MonAgent().run()

Les méthodes d'action
~~~~~~~~~~~~~~~~~~~~~~

Chaque méthode de rappel doit retourner **une action** :

.. list-table::
   :widths: 40 60
   :header-rows: 1

   * - Méthode
     - Effet
   * - ``self.noop()``
     - Ne rien faire
   * - ``self.send(to, performative, content, **kwargs)``
     - Envoyer un message ACL à ``to``
   * - ``self.reply(msg, performative, content, **kwargs)``
     - Répondre à ``msg`` (conserve le ``conversation_id``)
   * - ``self.delete()``
     - Supprimer l'agent
   * - ``self.suspend()``
     - Suspendre l'agent
   * - ``self.wake()``
     - Réveiller l'agent

La classe ACLMessage
~~~~~~~~~~~~~~~~~~~~~

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Attribut
     - Description
   * - ``msg.performative``
     - ``"request"``, ``"inform"``, ``"cfp"``, ``"cancel"``…
   * - ``msg.sender``
     - Nom de l'agent émetteur
   * - ``msg.content``
     - Contenu textuel du message
   * - ``msg.conversation_id``
     - Identifiant de la conversation
   * - ``msg.ontology``
     - Ontologie du message

Exemple — agent avec OpenAI
-----------------------------

.. code-block:: python

   import os, sys
   sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "python"))
   import gagent_py

   try:
       from openai import OpenAI
       client = OpenAI(api_key=os.environ.get("OPENAI_API_KEY", ""))
   except ImportError:
       client = None

   class AgentConseil(gagent_py.Agent):

       def on_start(self):
           self._history = []
           mode = f"OpenAI {self.model}" if client else "mode echo"
           sys.stderr.write(f"[agent] démarré en {mode}\n")

       def on_message(self, msg):
           if msg.performative == "cancel":
               return self.delete()

           if msg.performative != "request":
               return self.noop()

           reponse = self._demander(msg.content)
           return self.reply(msg, "inform", reponse)

       def on_tick(self):
           return self.noop()

       def _demander(self, question):
           if not client:
               return f"[echo] {question}"

           self._history.append({"role": "user", "content": question})

           # Fenêtre glissante : garder les max_history derniers tours
           if len(self._history) > self.max_history * 2:
               self._history = self._history[-(self.max_history * 2):]

           try:
               rep = client.chat.completions.create(
                   model=self.model,
                   messages=[
                       {"role": "system", "content": self.system_prompt}
                   ] + self._history,
                   max_tokens=self.max_tokens,
               )
               texte = rep.choices[0].message.content.strip()
               self._history.append({"role": "assistant", "content": texte})
               return texte
           except Exception as e:
               self._history.pop()
               return f"[erreur] {e}"

   if __name__ == "__main__":
       AgentConseil().run()

Exemple — agent avec Claude (Anthropic)
-----------------------------------------

.. code-block:: python

   import anthropic, gagent_py

   client = anthropic.Anthropic()  # lit ANTHROPIC_API_KEY

   class AgentClaude(gagent_py.Agent):

       def on_message(self, msg):
           if msg.performative == "cancel":
               return self.delete()
           if msg.performative != "request":
               return self.noop()

           rep = client.messages.create(
               model="claude-haiku-4-5-20251001",
               max_tokens=self.max_tokens,
               system=self.system_prompt,
               messages=[{"role": "user", "content": msg.content}],
           )
           return self.reply(msg, "inform", rep.content[0].text)

   if __name__ == "__main__":
       AgentClaude().run()

Exemple — agent avec Ollama (local, sans clé API)
---------------------------------------------------

Ollama fait tourner un LLM localement. Aucune clé API, aucun envoi de
données à l'extérieur.

.. code-block:: bash

   # Installer Ollama puis télécharger un modèle
   ollama run llama3

.. code-block:: python

   import requests, gagent_py

   def ask_ollama(prompt, model="llama3"):
       r = requests.post(
           "http://localhost:11434/api/generate",
           json={"model": model, "prompt": prompt, "stream": False}
       )
       return r.json()["response"]

   class AgentOllama(gagent_py.Agent):

       def on_message(self, msg):
           if msg.performative == "cancel":
               return self.delete()
           if msg.performative != "request":
               return self.noop()
           return self.reply(msg, "inform", ask_ollama(msg.content, self.model))

   if __name__ == "__main__":
       AgentOllama().run()

.. tip::

   Configurez ``model="llama3"`` côté C++ dans le ``PythonBehaviour``
   pour sélectionner le modèle Ollama à utiliser.

Gestion des erreurs
--------------------

Quelques bonnes pratiques :

- Encapsulez l'appel LLM dans un ``try/except`` pour éviter qu'une
  erreur réseau ne tue le script.
- Écrivez les logs de debug sur ``sys.stderr`` (pas ``stdout``) —
  ``stdout`` est réservé au protocole JSON avec le C++.
- Retournez toujours une action, même en cas d'erreur (``noop()`` ou
  un message d'erreur via ``reply()``).

.. code-block:: python

   def on_message(self, msg):
       try:
           reponse = self._appeler_llm(msg.content)
           return self.reply(msg, "inform", reponse)
       except Exception as e:
           sys.stderr.write(f"[erreur] {e}\n")
           return self.reply(msg, "failure", str(e))
