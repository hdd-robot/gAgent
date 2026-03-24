15 — Agents LLM : le couplage neuro-symbolique
================================================

Jusqu'ici, les agents que vous avez vus prennent leurs décisions de
façon **symbolique** : des règles explicites, des comportements codés,
une logique déterministe. C'est puissant pour des tâches structurées,
mais limité face à des problèmes ouverts, du langage naturel, ou des
situations imprévues.

Les **grands modèles de langage** (LLM — *Large Language Models*)
apportent la capacité de raisonner en langage naturel, de comprendre
des instructions ambiguës, et de générer des réponses adaptées au
contexte. Mais un LLM seul n'a pas de mémoire persistante, pas
d'identité, et ne peut pas interagir avec d'autres agents.

gAgent combine les deux : la **structure FIPA** donne à l'agent son
cycle de vie, sa messagerie, son enregistrement dans l'AMS — et le
**LLM** pilote sa prise de décision.

L'idée de base
---------------

Au lieu d'écrire la logique de l'agent en C++, vous déléguez cette
logique à un **script Python** qui appelle un LLM. Le script Python
reçoit les événements de l'agent (messages ACL reçus, ticks
périodiques) et retourne les actions à effectuer (envoyer un message,
se supprimer, ne rien faire).

.. code-block:: text

   Monde extérieur
        │  ACLMessage (REQUEST, INFORM, CFP…)
        ▼
   ┌─────────────────────────────────────────┐
   │  Agent C++ (cycle de vie FIPA)          │
   │  ├── enregistré dans AMS / DF           │
   │  ├── reçoit / envoie des ACLMessage     │
   │  └── PythonBehaviour                    │
   │           │ stdin/stdout (JSON)          │
   │           ▼                             │
   │      Script Python                      │
   │      ├── maintient l'historique         │
   │      ├── appelle le LLM                 │
   │      └── retourne l'action              │
   └─────────────────────────────────────────┘
        │  ACLMessage (INFORM, REFUSE…)
        ▼
   Autres agents

Pourquoi cette architecture ?
--------------------------------

- **Séparation des responsabilités** — le C++ gère le protocole FIPA,
  Python gère l'intelligence. Vous pouvez changer de LLM (OpenAI,
  Claude, Ollama…) sans toucher au code C++.

- **Mémoire de conversation** — le script Python maintient un
  historique des échanges, ce que le LLM ne fait pas nativement.

- **Multi-modèle** — dans un même SMA, vous pouvez avoir un agent
  piloté par GPT-4o, un autre par Claude, un troisième par un modèle
  local Ollama.

- **Mode dégradé** — si aucune clé API n'est disponible, le script
  peut fonctionner en mode echo ou avec un modèle local.

Ce que gAgent fournit
-----------------------

- **``PythonBehaviour``** (C++) — lance le script Python en sous-processus
  et orchestre la communication par ``stdin``/``stdout`` JSON.

- **``gagent_py``** (Python) — bibliothèque légère qui gère le protocole
  JSON côté Python. Vous n'avez qu'à surcharger ``on_message()`` et
  ``on_tick()``.

Les LLM supportés
------------------

N'importe quel LLM accessible depuis Python est compatible :

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - LLM
     - Accès
   * - **OpenAI** (GPT-4o, GPT-4o-mini…)
     - ``pip install openai`` + ``OPENAI_API_KEY``
   * - **Anthropic** (Claude Haiku, Sonnet…)
     - ``pip install anthropic`` + ``ANTHROPIC_API_KEY``
   * - **Ollama** (Llama 3, Mistral…)
     - ``ollama run llama3`` — API locale, aucune clé
   * - **Tout autre LLM**
     - Toute bibliothèque Python ou API HTTP

La suite de ce chapitre explique comment écrire le script Python
(page suivante) et comment l'intégrer dans un agent C++ (page d'après).
