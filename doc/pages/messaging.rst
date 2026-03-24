Messagerie entre agents
=======================

Architecture de communication
------------------------------

Chaque agent expose un **socket ZeroMQ PULL** lié à un endpoint IPC :

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Endpoint
     - Usage
   * - ``ipc:///tmp/acl_<nom>``
     - Messages FIPA ACL applicatifs (transport local par défaut)
   * - ``GAGENT_ENDPOINT_<NOM>=tcp://host:port``
     - Override pour déploiement multi-machine

Les messages sont envoyés via des sockets PUSH persistants (``PushCache``),
un par destination, réutilisés entre les appels pour éviter le coût
de reconnexion.

.. note::

   Le canal de contrôle interne de chaque agent (signaux de lifecycle)
   utilise toujours une **queue POSIX MQ** ``/{8-char-id}``. Seule la
   messagerie FIPA ACL applicative passe par ZeroMQ.

Helpers de messagerie
---------------------

Pour l'échange de messages ACL dans tes behaviours :

.. code-block:: cpp

   #include <gagent/messaging/AclMQ.hpp>
   using namespace gagent::messaging;

   // Pré-bind le socket PULL dans onStart() avant d'envoyer
   void onStart() override {
       acl_bind("monagent");
   }

   // Envoie un ACLMessage à l'agent "bob"
   acl_send("bob", msg);

   // Attend un message sur sa propre queue (timeout 5s)
   auto opt = acl_receive("monagent", 5000);
   if (opt) {
       const ACLMessage& m = *opt;
       // traitement...
   }

.. warning::

   Toujours appeler ``acl_bind(nom)`` dans ``onStart()`` **avant** tout
   ``acl_receive()``. Cela garantit que le socket PULL est lié avant que
   l'autre agent n'essaie de s'y connecter.

Exemple complet : REQUEST ↔ INFORM
------------------------------------

Voir ``tests/two_agents_acl.cpp`` pour un exemple fonctionnel.

Alice envoie 3 requêtes à Bob qui répond à chaque fois :

.. code-block:: text

   [Alice → Bob] REQUEST : "cycle 1"
   [Bob → Alice] INFORM  : "ok:1"
   [Alice → Bob] REQUEST : "cycle 2"
   [Bob → Alice] INFORM  : "ok:2"
   [Alice → Bob] REQUEST : "cycle 3"
   [Bob → Alice] INFORM  : "ok:3"

Pattern général d'un agent communicant
---------------------------------------

.. code-block:: cpp

   class ReceiverBehaviour : public Behaviour {
       int count_ = 0;
   public:
       void onStart() override {
           acl_bind("receiver");   // lie le socket PULL
       }

       void action() override {
           auto opt = acl_receive("receiver", 5000);
           if (!opt) return;
           const ACLMessage& msg = *opt;

           ACLMessage reply = msg.createReply(ACLMessage::Performative::INFORM);
           reply.setSender (AgentIdentifier{"receiver"});
           reply.setContent("ma réponse");
           acl_send(msg.getSender().name, reply);
           ++count_;
       }

       bool done() override { return count_ >= 5; }
   };

   class SenderBehaviour : public Behaviour {
       int count_ = 0;
   public:
       void onStart() override {
           acl_bind("sender");
       }

       void action() override {
           ACLMessage req(ACLMessage::Performative::REQUEST);
           req.setSender  (AgentIdentifier{"sender"});
           req.addReceiver(AgentIdentifier{"receiver"});
           req.setContent ("ma requête");
           req.setConversationId("conv-" + std::to_string(count_));
           acl_send("receiver", req);

           auto opt = acl_receive("sender", 5000);
           if (opt) ++count_;
       }

       bool done() override { return count_ >= 5; }
   };

Déploiement multi-machine
--------------------------

Par défaut le transport est IPC (local). Pour connecter des agents sur
des machines distinctes, définir la variable d'environnement avant de
lancer l'agent destinataire :

.. code-block:: bash

   # Machine B — agent "bob" écoute sur TCP
   GAGENT_ENDPOINT_BOB=tcp://0.0.0.0:5555 ./mon_app

   # Machine A — alice envoie à bob sur la machine B
   GAGENT_ENDPOINT_BOB=tcp://192.168.1.42:5555 ./mon_app_alice

.. note::

   Le nom dans la variable d'environnement est en **majuscules** :
   ``GAGENT_ENDPOINT_<NOM_EN_MAJUSCULES>``.

Thread-safety
-------------

``PushCache`` est thread-safe : plusieurs behaviours d'un même agent
peuvent appeler ``acl_send()`` vers la même destination simultanément
sans risque de corruption. Le mutex est maintenu pour toute la durée
de l'envoi ZeroMQ.
