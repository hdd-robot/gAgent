Messagerie entre agents
=======================

Architecture de communication
------------------------------

Chaque agent expose deux queues POSIX MQ :

.. list-table::
   :widths: 30 20 50
   :header-rows: 1

   * - Queue
     - Taille max
     - Usage
   * - ``/{8-char-id}``
     - 100 octets
     - Signaux de contrôle internes (lifecycle)
   * - ``/acl_{nom}``
     - 1024 octets
     - Messages FIPA ACL applicatifs

Helpers de messagerie
---------------------

Pour l'échange de messages ACL dans tes behaviours :

.. code-block:: cpp

   // Envoie un ACLMessage à l'agent "bob"
   acl_send("bob", msg);

   // Attend un message sur sa propre queue (timeout 5s)
   auto opt = acl_receive("alice", 5000);
   if (opt) {
       const ACLMessage& m = *opt;
       // traitement...
   }

Exemple complet : REQUEST ↔ INFORM
------------------------------------

Voir ``tests/two_agents_acl.cpp`` pour un exemple fonctionnel.

Alice envoie 4 requêtes à Bob qui répond avec l'heure courante :

.. code-block:: text

   [Alice → Bob] REQUEST : "Quelle heure est-il ? (msg 1/4)"
   [Bob ← Alice] request : "Quelle heure est-il ? (msg 1/4)"
   [Bob → Alice] INFORM  : "Il est 21:01:38"
   [Alice ← Bob] inform  : "Il est 21:01:38"
   ...

Pattern général d'un agent communicant
---------------------------------------

.. code-block:: cpp

   class SenderBehaviour : public Behaviour {
       int count_ = 0;
   public:
       SenderBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           ACLMessage req(ACLMessage::Performative::REQUEST);
           req.setSender  (AgentIdentifier{"sender"});
           req.addReceiver(AgentIdentifier{"receiver"});
           req.setContent ("ma requête");
           req.setConversationId("conv-" + std::to_string(count_));
           acl_send("receiver", req);
           ++count_;
           sleep(1);
       }

       bool done() override { return count_ >= 5; }
   };

   class ReceiverBehaviour : public Behaviour {
       int count_ = 0;
   public:
       ReceiverBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           auto opt = acl_receive("receiver", 5000);
           if (!opt) return;
           const ACLMessage& msg = *opt;

           // Répondre
           ACLMessage reply = msg.createReply(ACLMessage::Performative::INFORM);
           reply.setSender (AgentIdentifier{"receiver"});
           reply.setContent("ma réponse");
           acl_send(msg.getSender().name, reply);
           ++count_;
       }

       bool done() override { return count_ >= 5; }
   };

.. note::

   Nettoyage des queues dans ``takeDown()`` :

   .. code-block:: cpp

      void takeDown() override {
          mq_unlink("/acl_monagent");
      }
