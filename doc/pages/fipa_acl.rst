FIPA ACL
========

Le standard **FIPA ACL** (Agent Communication Language) définit un protocole
de communication entre agents basé sur des **performatives** et des
**s-expressions**.

Performatives
-------------

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Performative
     - Signification
   * - ``request``
     - Demande à l'agent récepteur d'effectuer une action
   * - ``inform``
     - Informe que la proposition est vraie
   * - ``query-ref``
     - Demande la valeur d'une expression
   * - ``cfp``
     - Call for Proposal (protocole de négociation)
   * - ``propose``
     - Répond à un CFP avec une proposition
   * - ``accept-proposal``
     - Accepte une proposition
   * - ``reject-proposal``
     - Rejette une proposition
   * - ``agree``
     - Accepte de réaliser une action
   * - ``refuse``
     - Refuse de réaliser une action
   * - ``failure``
     - Signale l'échec d'une action
   * - ``not-understood``
     - Signale que le message n'a pas été compris
   * - ``subscribe``
     - S'abonne à des notifications
   * - ``cancel``
     - Annule une demande précédente

Champs d'un message
-------------------

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Champ
     - Description
   * - ``:sender``
     - Identifiant de l'agent émetteur
   * - ``:receiver``
     - Identifiant(s) du ou des agents destinataires
   * - ``:content``
     - Contenu du message (expression dans le langage déclaré)
   * - ``:language``
     - Langage du contenu (``fipa-sl``, ``prolog``, etc.)
   * - ``:ontology``
     - Ontologie partagée entre émetteur et récepteur
   * - ``:protocol``
     - Protocole d'interaction utilisé
   * - ``:conversation-id``
     - Identifiant de conversation
   * - ``:reply-with``
     - Référence pour la réponse attendue
   * - ``:in-reply-to``
     - Référence du message auquel on répond
   * - ``:encoding``
     - Encodage du contenu

Usage en C++
------------

Créer un message
~~~~~~~~~~~~~~~~

.. code-block:: cpp

   #include <gagent/messaging/ACLMessage.hpp>
   using namespace gagent;

   ACLMessage msg(ACLMessage::Performative::REQUEST);
   msg.setSender    (AgentIdentifier{"alice"});
   msg.addReceiver  (AgentIdentifier{"bob"});
   msg.setContent   ("(action bob (query-time))");
   msg.setLanguage  ("fipa-sl");
   msg.setOntology  ("time-ontology");
   msg.setConversationId("conv-42");

Sérialiser
~~~~~~~~~~

.. code-block:: cpp

   std::string raw = msg.toString();
   // → (request
   //    :sender (agent-identifier :name alice)
   //    :receiver (set (agent-identifier :name bob))
   //    :content "(action bob (query-time))"
   //    :language fipa-sl
   //    :ontology time-ontology
   //    :conversation-id conv-42)

Parser
~~~~~~

.. code-block:: cpp

   auto opt = ACLMessage::parse(raw);
   if (opt) {
       ACLMessage& m = *opt;
       std::cout << m.getContent() << "\n";
   }

Créer une réponse
~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   // Crée automatiquement un message avec :in-reply-to rempli
   ACLMessage reply = msg.createReply(ACLMessage::Performative::INFORM);
   reply.setSender (AgentIdentifier{"bob"});
   reply.setContent("Il est 21:00:00");
