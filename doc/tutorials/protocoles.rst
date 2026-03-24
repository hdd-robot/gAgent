6 — Les protocoles d'interaction
==================================

Vous savez maintenant envoyer et recevoir des messages individuels. Mais
dans la réalité, les agents ne s'envoient pas des messages isolés — ils
ont des **conversations structurées** qui suivent des règles précises.
C'est ce qu'on appelle un protocole.

Qu'est-ce qu'un protocole ?
----------------------------

Un protocole est un **scénario de conversation convenu à l'avance**
entre plusieurs agents. Il définit :

- **Qui parle en premier** et avec quelle performative
- **Quelles réponses sont attendues** à chaque étape
- **Comment la conversation se termine** (succès, refus, timeout...)

Pensez à un protocole comme à un formulaire administratif : tout le monde
sait dans quel ordre remplir les cases, ce que signifie chaque case, et
quand le formulaire est complet. Sans ce cadre commun, chaque échange
serait à réinventer de zéro.

**Sans protocole** — deux agents doivent inventer leur propre langage :

.. code-block:: text

   Alice → Bob : "calcule 6 * 7"
   Bob → Alice : "42"
   Alice → Bob : "c'est quoi 42 ?"   ← ambiguïté : est-ce la réponse ?

**Avec un protocole** — le déroulement est prévisible pour tout le monde :

.. code-block:: text

   Alice → Bob : REQUEST  "calcule 6 * 7"    ← demande formelle
   Bob → Alice : INFORM   "42"               ← réponse formelle
   → les deux agents savent exactement ce que signifie chaque message

Pourquoi les protocoles existent-ils ?
---------------------------------------

Dans un système avec de nombreux agents qui ne se connaissent pas
forcément, les protocoles garantissent l'**interopérabilité** : un agent
conçu par une équipe peut parler à un agent conçu par une autre équipe,
du moment qu'ils respectent le même protocole.

C'est la même logique qu'un standard de communication humain :
quand vous appelez un médecin, la consultation suit un protocole implicite
(symptômes → diagnostic → ordonnance). Chacun sait ce qui vient ensuite.

Les protocoles disponibles dans gAgent
----------------------------------------

gAgent implémente trois protocoles standardisés par FIPA. Chacun est
fourni sous forme de behaviours prêts à l'emploi — vous n'avez pas à gérer
les échanges de messages manuellement.

.. list-table::
   :widths: 20 35 45
   :header-rows: 1

   * - Protocole
     - Cas d'usage typique
     - Behaviours disponibles
   * - **Request**
     - Demander à un agent d'effectuer une action et attendre sa réponse
     - ``RequestInitiator``, ``RequestParticipant``
   * - **Contract Net**
     - Confier une tâche au meilleur candidat parmi plusieurs agents
     - ``ContractNetInitiator``, ``ContractNetParticipant``
   * - **Subscribe-Notify**
     - Recevoir des notifications automatiques à chaque changement d'état
     - ``SubscribeInitiator``, ``SubscribeParticipant``

Comment gAgent simplifie les protocoles
-----------------------------------------

Sans framework, implémenter un protocole demande de gérer soi-même
l'envoi de chaque message, les timeouts, les cas d'erreur, le suivi des
conversations... C'est long et source de bugs.

Avec gAgent, chaque protocole est encapsulé dans un behaviour. Vous
n'implémentez que **la logique métier** — ce qui est spécifique à votre
application — et le framework gère le reste :

.. code-block:: text

   ┌─────────────────────────────────────────────────┐
   │  Votre logique métier                           │
   │  (que proposer ? comment choisir ? quoi faire ?)│
   └───────────────┬─────────────────────────────────┘
                   │ vous implémentez uniquement ces méthodes
   ┌───────────────▼─────────────────────────────────┐
   │  Behaviour gAgent (ContractNetInitiator, etc.)  │
   │  (envoi des CFP, attente des réponses,          │
   │   gestion des timeouts, accept/reject...)       │
   └─────────────────────────────────────────────────┘

Les trois tutoriels suivants présentent chaque protocole en détail.
