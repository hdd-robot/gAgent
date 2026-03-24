Introduction — Les systèmes multi-agents
=========================================

Avant de commencer à coder, il est utile de comprendre ce qu'est un
système multi-agents, comment les agents s'organisent entre eux, et
comment la plateforme gAgent les accompagne. Cette page ne contient pas
de code — elle pose les bases conceptuelles de tout ce qui suit.

Qu'est-ce qu'un système multi-agents ?
---------------------------------------

Un **système multi-agents** (SMA) est un programme composé de plusieurs
entités autonomes — les **agents** — qui travaillent ensemble pour
accomplir un objectif.

Chaque agent :

- a ses propres **responsabilités** (ses behaviours)
- **prend ses décisions** de façon indépendante
- peut **communiquer** avec les autres agents pour coopérer, négocier
  ou se coordonner

Pensez à une ruche : chaque abeille est autonome, elle prend ses
décisions seule, mais l'ensemble du système accomplit quelque chose de
beaucoup plus complexe qu'une seule abeille ne pourrait faire. Un SMA
fonctionne sur le même principe.

**Pourquoi plusieurs agents plutôt qu'un seul programme ?**

- **Décomposition** — un problème complexe est découpé en sous-problèmes,
  chacun géré par un agent spécialisé.
- **Parallélisme** — les agents travaillent en même temps, ce qui accélère
  le traitement.
- **Résilience** — si un agent tombe en panne, les autres continuent à
  fonctionner.
- **Distribution** — les agents peuvent tourner sur des machines
  différentes, réparties sur un réseau.

Les agents dans gAgent
-----------------------

Dans gAgent, chaque agent est une entité qui :

- tourne de façon **indépendante** dès son démarrage
- exécute une ou plusieurs tâches (**behaviours**) en parallèle
- communique avec les autres agents via des **messages FIPA ACL**

Les agents peuvent être sur la **même machine** (communication par
canaux locaux) ou sur des **machines différentes** (communication par
réseau). Du point de vue du code, cela ne change rien : on utilise
toujours les mêmes fonctions ``acl_send()`` et ``acl_receive()``.

.. code-block:: text

   Machine A                         Machine B
   ┌──────────────────────┐          ┌──────────────────────┐
   │  Agent Alice         │          │  Agent Bob           │
   │  Agent Carol         │◄────────►│  Agent Dave          │
   │                      │  réseau  │                      │
   └──────────────────────┘          └──────────────────────┘

La plateforme FIPA
-------------------

Dans un système multi-agents, les agents ont besoin d'une
**infrastructure commune** pour se trouver, s'identifier et se
coordonner. C'est le rôle de la **plateforme FIPA**.

La plateforme gAgent (``agentplatform``) est un service à lancer
avant les agents. Elle fournit deux services fondamentaux : l'AMS et
le DF.

.. code-block:: text

   ┌─────────────────────────────────────────────────────┐
   │                  gAgent Platform                    │
   │                                                     │
   │   ┌──────────────────┐   ┌────────────────────┐    │
   │   │  AMS             │   │  DF                │    │
   │   │  (annuaire des   │   │  (annuaire des     │    │
   │   │   agents)        │   │   services)        │    │
   │   └──────────────────┘   └────────────────────┘    │
   │                                                     │
   └─────────────────────────────────────────────────────┘
         ▲           ▲                 ▲           ▲
      Agent A     Agent B          Agent C     Agent D

L'AMS — qui sont les agents ?
------------------------------

L'**AMS** (Agent Management System) est le **registre officiel des
agents** actifs sur la plateforme. C'est l'équivalent d'un annuaire
téléphonique : chaque agent s'y inscrit à son démarrage et s'en
désinscrit à sa fin.

L'AMS permet de :

- **Savoir quels agents sont actifs** à un instant donné
- **Connaître l'état** de chaque agent (actif, suspendu, en attente...)
- **Contrôler les agents** à distance (suspendre, réveiller, supprimer)

Quand un agent gAgent démarre, il s'enregistre automatiquement dans
l'AMS sans que vous n'ayez à écrire la moindre ligne de code pour cela.
Quand il termine, il se désinscrit automatiquement.

.. code-block:: text

   Agent Alice démarre
      → s'enregistre dans l'AMS : "alice, actif"

   Agent Bob démarre
      → s'enregistre dans l'AMS : "bob, actif"

   [AMS] registre :
     alice  →  actif
     bob    →  actif

   Agent Alice termine
      → se désinscrit de l'AMS

   [AMS] registre :
     bob    →  actif

La plateforme n'est **pas obligatoire** pour faire fonctionner vos
agents. Sans elle, les agents tournent en mode autonome : ils
s'exécutent normalement, mais ne sont pas enregistrés et ne peuvent pas
être supervisés depuis l'extérieur.

Le DF — que font les agents ?
------------------------------

Le **DF** (Directory Facilitator) est l'**annuaire des services**
offerts par les agents. Là où l'AMS répond à la question *"qui est
actif ?"*, le DF répond à la question *"qui sait faire quoi ?"*.

Un agent qui offre un service — calcul, traduction, planification,
accès à une base de données — peut s'enregistrer dans le DF avec une
description de ce service. D'autres agents peuvent ensuite interroger
le DF pour trouver un agent capable de les aider.

.. code-block:: text

   Agent Alice s'enregistre dans le DF :
      type = "planification"
      ontologie = "logistique"

   Agent Carol s'enregistre dans le DF :
      type = "traduction"
      ontologie = "langues"

   Agent Bob interroge le DF :
      "qui fait de la planification logistique ?"
      → réponse : Alice

   Bob contacte directement Alice.

Cela permet de construire des systèmes **dynamiques** où les agents ne
se connaissent pas à l'avance : ils se découvrent au moment où ils en
ont besoin, via le DF.

Les outils de supervision
--------------------------

La plateforme gAgent fournit deux outils pour observer et contrôler
votre système en temps réel :

**agentmanager** — la console de gestion

.. code-block:: bash

   agentmanager list      # liste tous les agents actifs
   agentmanager watch     # affichage en temps réel, rafraîchi toutes les secondes
   agentmanager suspend alice   # suspendre l'agent alice
   agentmanager wake alice      # le réveiller
   agentmanager kill alice      # l'arrêter

**agentview** — la visualisation web

Un serveur web léger qui affiche une carte SVG de vos agents en temps
réel, accessible depuis un navigateur à l'adresse
``http://localhost:8080``.

Déploiement sur plusieurs machines
------------------------------------

Un système multi-agents prend tout son sens quand les agents sont
répartis sur plusieurs machines. gAgent supporte ce mode de
déploiement nativement.

Par défaut, les agents d'une même machine se trouvent via leur nom
(communication locale). Pour qu'un agent sur la **machine A** puisse
envoyer un message à un agent sur la **machine B**, il suffit de
définir une variable d'environnement indiquant l'adresse réseau de
l'agent destinataire :

.. code-block:: bash

   # Sur la machine B — l'agent "bob" écoute sur le réseau
   GAGENT_ENDPOINT_BOB=tcp://0.0.0.0:5555 ./mon_application

   # Sur la machine A — alice sait où trouver bob
   GAGENT_ENDPOINT_BOB=tcp://192.168.1.42:5555 ./mon_application_alice

Du côté du code, **rien ne change** : on utilise toujours
``acl_send("bob", message)`` exactement comme si bob était sur la même
machine.

.. code-block:: text

   Machine A (192.168.1.10)          Machine B (192.168.1.42)
   ┌──────────────────────┐          ┌──────────────────────┐
   │  Agent Alice         │          │  Agent Bob           │
   │                      │          │                      │
   │  acl_send("bob", m)  │─────────►│  acl_receive("bob")  │
   │                      │  TCP     │                      │
   └──────────────────────┘          └──────────────────────┘
   Code identique sur les deux machines — seule la variable
   d'environnement change.

Ce qu'il faut retenir
----------------------

- Un **agent** est une entité autonome avec ses propres tâches et sa
  propre logique.
- Les agents **communiquent** par messages FIPA ACL, localement ou sur
  le réseau.
- L'**AMS** sait quels agents sont actifs et dans quel état.
- Le **DF** sait quels services sont disponibles et qui les offre.
- La **plateforme** est optionnelle mais recommandée pour superviser
  votre système.
- Le code est le **même** que les agents soient sur une ou plusieurs
  machines.

Les tutoriels suivants vous montrent comment mettre tout cela en
pratique, étape par étape.
