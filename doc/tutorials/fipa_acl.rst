4 — La communication FIPA ACL
==============================

Avant d'écrire la moindre ligne de code pour faire communiquer des
agents, il est important de comprendre *comment* ils se parlent et
*pourquoi* ce système a été conçu ainsi. Ce tutoriel explique les
concepts fondamentaux de la communication FIPA ACL.

Pourquoi un langage de communication entre agents ?
----------------------------------------------------

Quand deux personnes se parlent, elles ne font pas que s'échanger des
mots. Elles savent instinctivement si l'autre **informe**, **demande**,
**propose**, **refuse** ou **accepte** quelque chose. Ce contexte — ce
qu'on appelle l'**intention** derrière les mots — est aussi important
que les mots eux-mêmes.

Entre agents informatiques, c'est pareil. Si un agent envoie le message
``"42"`` à un autre, cela ne veut rien dire sans contexte : est-ce une
réponse ? Une question ? Une erreur ? Un identifiant ?

**FIPA ACL** (Agent Communication Language) est un standard international
qui résout ce problème. Il définit un format de message structuré où
chaque message porte explicitement son intention. Les agents n'ont plus
à deviner — le message dit clairement ce qu'il est.

.. note::

   FIPA (Foundation for Intelligent Physical Agents) est un organisme
   de standardisation qui a défini dans les années 2000 des protocoles
   de communication pour les systèmes multi-agents. Ces standards sont
   aujourd'hui utilisés dans des domaines variés : robotique, simulation,
   systèmes distribués, intelligence artificielle.

La notion de performative
--------------------------

Le mot clé du langage FIPA ACL est la **performative**. C'est
l'étiquette qui indique l'intention du message — ce que l'émetteur
*fait* en envoyant ce message.

Le terme vient de la linguistique : un **énoncé performatif** est une
phrase qui accomplit une action par le seul fait d'être prononcée.
Dire "Je te promets" *est* une promesse. Dire "Je t'informe" *est* un
acte d'information.

En FIPA ACL, chaque message commence par sa performative :

.. code-block:: text

   (inform               ← "je t'informe que..."
    :sender   alice
    :receiver bob
    :content  "la température est de 23°C"
   )

   (request              ← "je te demande de..."
    :sender   alice
    :receiver bob
    :content  "ouvre la fenêtre"
   )

La performative n'est pas une convention arbitraire — elle engage
l'émetteur. Un agent qui envoie un ``inform`` affirme que l'information
est vraie. Un agent qui envoie un ``request`` attend réellement une
action en retour.

Structure d'un message
-----------------------

Un message FIPA ACL est composé de **champs** qui décrivent
précisément la communication :

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Champ
     - Rôle
   * - Performative
     - L'intention du message (``inform``, ``request``, etc.)
   * - ``:sender``
     - Qui envoie le message
   * - ``:receiver``
     - À qui le message est destiné
   * - ``:content``
     - Le contenu du message (ce qu'on veut dire)
   * - ``:conversation-id``
     - Un identifiant pour relier les messages d'un même échange
   * - ``:reply-with``
     - Une référence que le destinataire devra inclure dans sa réponse
   * - ``:in-reply-to``
     - La référence du message auquel on répond
   * - ``:language``
     - Le langage utilisé pour exprimer le contenu
   * - ``:ontology``
     - Le vocabulaire partagé pour interpréter le contenu

Les champs ``:conversation-id``, ``:reply-with`` et ``:in-reply-to``
sont particulièrement importants : ils permettent de **relier les
messages entre eux** et de suivre une conversation qui s'étale sur
plusieurs échanges.

Les performatives disponibles
-------------------------------

gAgent implémente les performatives les plus utilisées du standard FIPA.
Voici chacune d'elles expliquée avec un exemple du quotidien.

``INFORM`` — informer
~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "Je vous annonce quelque chose. Je garantis que
c'est vrai."

**Analogie :** Un météorologue qui annonce "Il fera 28°C demain."

**Usage :** Transmettre un résultat, une mise à jour d'état, une
notification.

.. code-block:: text

   Alice → Bob
   (inform
    :content "le capteur 3 mesure 87% d'humidité"
   )

``REQUEST`` — demander
~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "Je vous demande d'effectuer une action. J'attends
que vous la fassiez (ou que vous m'expliquiez pourquoi vous ne pouvez pas)."

**Analogie :** Un manager qui dit à un employé "Prépare le rapport pour
demain."

**Usage :** Déclencher une action chez un autre agent.

.. code-block:: text

   Alice → Bob
   (request
    :content "calcule l'itinéraire vers Paris"
   )

``AGREE`` — accepter
~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "J'ai reçu votre demande, j'accepte de la traiter.
Le résultat viendra plus tard."

**Analogie :** Un employé qui répond "Bien reçu, je m'en occupe."

**Usage :** Répondre à un ``request`` quand l'action prend du temps.
L'agent confirme qu'il a accepté la mission avant de renvoyer le résultat.

.. code-block:: text

   Bob → Alice  (en réponse au request)
   (agree
    :content "je calcule l'itinéraire, patience..."
   )

``REFUSE`` — refuser
~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "J'ai reçu votre demande, mais je ne peux pas ou
ne veux pas la traiter. Je vous explique pourquoi."

**Analogie :** Un employé qui répond "Je ne peux pas faire ça, je suis
déjà sur un autre projet."

**Usage :** Répondre à un ``request`` ou à un ``cfp`` qu'on ne peut pas
honorer.

.. code-block:: text

   Bob → Alice  (en réponse au request)
   (refuse
    :content "itinéraire impossible : destination inconnue"
   )

``FAILURE`` — échec
~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "J'avais accepté votre demande, j'ai essayé, mais
j'ai échoué. Voici ce qui s'est passé."

**Analogie :** Un livreur qui appelle pour dire "J'ai essayé de livrer
le colis mais l'adresse n'existe pas."

**Usage :** Signaler qu'une action qui avait été acceptée n'a pas pu
aboutir.

.. code-block:: text

   Bob → Alice  (après un agree)
   (failure
    :content "service de cartographie indisponible"
   )

``CFP`` — appel à propositions
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "J'ai une tâche à faire réaliser. Qui peut la
faire, et à quelles conditions ?"

**Analogie :** Un acheteur qui publie un appel d'offres : "Je cherche
un prestataire pour rénover mon bureau."

**Usage :** Premier message du protocole Contract Net. L'agent initiateur
envoie un CFP à plusieurs agents et attend leurs propositions.

.. code-block:: text

   Coordinateur → [Transporteur A, Transporteur B, Transporteur C]
   (cfp
    :content "livraison Paris→Lyon, 500kg, pour demain"
   )

``PROPOSE`` — proposer
~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "En réponse à votre appel d'offres, voici ce que
je propose."

**Analogie :** Un prestataire qui répond à l'appel d'offres : "Je peux
le faire pour 850€, délai 3 semaines."

**Usage :** Réponse à un ``cfp``. Contient les conditions de la
proposition.

.. code-block:: text

   Transporteur A → Coordinateur
   (propose
    :content "je peux livrer pour 120€"
   )

``ACCEPT_PROPOSAL`` — accepter une proposition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "Votre proposition est retenue. Vous avez le contrat."

**Analogie :** L'acheteur qui rappelle le prestataire choisi : "On
retient votre offre."

**Usage :** Envoyé par l'initiateur du Contract Net au gagnant de
l'appel d'offres.

.. code-block:: text

   Coordinateur → Transporteur A
   (accept-proposal
    :content "proposition retenue, procède à la livraison"
   )

``REJECT_PROPOSAL`` — rejeter une proposition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "Votre proposition n'a pas été retenue."

**Analogie :** L'acheteur qui envoie un mail aux prestataires non
retenus : "Nous avons choisi une autre offre."

**Usage :** Envoyé par l'initiateur du Contract Net aux agents dont
la proposition n'a pas été sélectionnée.

.. code-block:: text

   Coordinateur → Transporteur B
   (reject-proposal
    :content "une meilleure offre a été retenue"
   )

``SUBSCRIBE`` — s'abonner
~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "Je veux être prévenu chaque fois qu'un
événement particulier se produit de votre côté."

**Analogie :** S'abonner à une newsletter : "Envoyez-moi un mail chaque
fois que vous publiez un article."

**Usage :** Premier message du protocole Subscribe-Notify. L'agent
abonné recevra des ``inform`` automatiques à chaque mise à jour.

.. code-block:: text

   Moniteur → Capteur
   (subscribe
    :content "notifie-moi à chaque changement de température"
   )

``CANCEL`` — annuler
~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "Annulez la demande que je vous avais faite
précédemment."

**Analogie :** Se désabonner d'une newsletter.

**Usage :** Mettre fin à un abonnement Subscribe-Notify, ou annuler
une demande en cours.

.. code-block:: text

   Moniteur → Capteur
   (cancel
    :content "fin de la surveillance"
   )

``NOT_UNDERSTOOD`` — non compris
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

**Ce que ça signifie :** "J'ai reçu votre message mais je n'ai pas
compris ce que vous attendiez de moi."

**Analogie :** "Pardon, je n'ai pas bien saisi votre question."

**Usage :** Réponse à tout message dont le contenu ou la performative
ne peut pas être interprété.

.. code-block:: text

   Bob → Alice
   (not-understood
    :content "performative inconnue dans ce contexte"
   )

Comment les performatives s'enchaînent
---------------------------------------

Les performatives ne s'utilisent pas isolément — elles s'enchaînent
selon des **protocoles** définis par le standard FIPA. Chaque protocole
décrit le dialogue complet entre agents : qui parle en premier, quelles
réponses sont possibles, comment ça se termine.

**Protocole Request** — le plus simple :

.. code-block:: text

   Alice                          Bob
     │                             │
     │──── REQUEST ───────────────►│
     │                             │  (Bob traite la demande)
     │◄─── INFORM ─────────────────│  succès
     │  ou REFUSE                  │  refus
     │  ou FAILURE                 │  échec après acceptation
     │  ou AGREE + INFORM          │  acceptation différée

**Protocole Contract Net** — la négociation :

.. code-block:: text

   Coordinateur          Agent A        Agent B        Agent C
        │                   │              │              │
        │──── CFP ──────────►──────────────►──────────────►
        │                   │              │              │
        │◄─── PROPOSE ───────│              │              │
        │◄─── PROPOSE ───────────────────── │              │
        │◄─── REFUSE ────────────────────────────────────── │
        │                   │              │              │
        │  (sélection du meilleur)
        │                   │              │
        │──── ACCEPT ────────►              │
        │──── REJECT ───────────────────────►

**Protocole Subscribe-Notify** — l'abonnement :

.. code-block:: text

   Moniteur                       Capteur
     │                              │
     │──── SUBSCRIBE ──────────────►│
     │◄─── AGREE ───────────────────│  abonnement accepté
     │                              │
     │◄─── INFORM ──────────────────│  notification 1
     │◄─── INFORM ──────────────────│  notification 2
     │◄─── INFORM ──────────────────│  notification 3
     │                              │
     │──── CANCEL ─────────────────►│  fin de l'abonnement

Ce qu'il faut retenir
----------------------

- Chaque message a une **performative** qui dit clairement ce qu'il est.
- Les messages sont reliés entre eux par un **identifiant de
  conversation** — les agents savent à quel échange appartient chaque
  message.
- Les performatives s'enchaînent selon des **protocoles** standardisés
  — les tutoriels suivants montrent comment les utiliser concrètement.
- Un agent n'est jamais obligé de répondre avec une performative
  spécifique — s'il ne comprend pas, il répond ``NOT_UNDERSTOOD``.
  S'il ne peut pas, il répond ``REFUSE``.

Dans les tutoriels suivants, vous allez voir comment envoyer et recevoir
ces messages en C++ avec gAgent, et comment utiliser les protocoles
prêts à l'emploi fournis par le framework.
