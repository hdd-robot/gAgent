10 — Le SMA situé : qu'est-ce qu'un environnement ?
=====================================================

Dans un système multi-agent, les agents ne vivent pas dans le vide. Ils
agissent *quelque part*. Cet espace dans lequel les agents existent,
perçoivent et agissent s'appelle l'**environnement**.

Un SMA est dit **situé** (*situated*) quand ses agents ont une présence
physique ou symbolique dans un espace partagé — ils ont une position, des
capteurs, des effecteurs, et leurs actions ont un effet sur le monde qui
les entoure.

Le débat scientifique
----------------------

La notion d'environnement dans les SMA a longtemps divisé la communauté.
Deux courants s'affrontent.

**Le courant minimaliste**

Pour des chercheurs comme Russell & Norvig ou Wooldridge, un SMA peut
se réduire à un ensemble d'agents qui s'échangent des messages. L'état
du monde est *représenté* dans les messages eux-mêmes : un agent
communique sa position, son état, ses observations à ses voisins. Il
n'y a pas besoin d'un espace partagé distinct — la messagerie *est*
l'environnement.

Cette vision est séduisante par sa simplicité. Elle convient très bien
aux systèmes de traitement d'information distribués, aux orchestrateurs
de services, ou aux agents purement cognitifs.

**Le courant situé**

Pour d'autres chercheurs — Rodney Brooks, Jacques Ferber (auteur de
*Multi-Agent Systems*, 1999), ou Danny Weyns — l'environnement est une
entité de première classe dans le SMA, pas un détail d'implémentation.

Leurs arguments :

- Un agent sans environnement ne *perçoit* rien — il ne fait que
  réagir à des messages, sans ancrage dans la réalité.
- L'environnement assure un **couplage indirect** entre agents : deux
  agents peuvent interagir sans se connaître, simplement en modifiant
  l'espace partagé (comme les fourmis avec les phéromones).
- Il permet de modéliser des phénomènes **émergents** : des
  comportements complexes qui naissent de l'interaction locale entre
  agents et environnement, sans coordination centrale.
- Dans les applications robotiques, de simulation ou de jeux, un
  environnement explicite est incontournable.

**Et dans la pratique ?**

Les deux positions ont leur mérite selon le type de système. Un SMA
d'agents conversationnels ou de planification n'a pas besoin d'un
espace 2D. Un SMA de simulation de fourmis, de robots, ou d'agents
évoluant dans une carte en a absolument besoin.

Le positionnement de gAgent
-----------------------------

gAgent adopte une position pragmatique : **l'environnement est
optionnel**. Vous pouvez tout à fait créer un système multi-agent
purement messagerie, sans environnement. Mais si votre application
nécessite un espace partagé — positions, capteurs, visualisation en
temps réel — l'environnement est disponible nativement.

.. code-block:: text

   Sans environnement (SMA messagerie) :

       Agent A ──── ACLMessage ────► Agent B
       Agent B ──── ACLMessage ────► Agent C

   Avec environnement (SMA situé) :

       ┌─────────────────────────────────────────┐
       │             Environnement               │
       │                                         │
       │    Agent A ●      ● Agent B             │
       │         (x=10,y=5)  (x=80,y=30)        │
       │                  ● Agent C              │
       │               (x=45,y=60)               │
       └─────────────────────────────────────────┘

Dans le SMA situé, les agents ont une **position** dans l'espace. Ils
peuvent se déplacer, interagir avec leur voisinage, et leur état est
visible dans la visualisation en temps réel.

Ce que gAgent appelle un environnement
----------------------------------------

Dans gAgent, l'environnement joue trois rôles :

1. **Récepteur d'état** — il collecte les attributs de chaque agent
   (position, couleur, valeur…) publiés via la file de messages interne.

2. **Mémoire partagée** — il maintient une vue consolidée de l'état de
   tous les agents à un instant donné.

3. **Passerelle de visualisation** — il expose cet état via un socket
   Unix que ``agentview`` interroge en temps réel pour afficher la carte.

La suite de ce chapitre explique comment coder tout cela en pratique.
