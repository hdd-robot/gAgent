5 — Envoyer et recevoir un message
====================================

Ce tutoriel présente les briques fondamentales de la communication entre
agents : comment créer un message, l'envoyer à un autre agent, et le
recevoir. Tout le reste — les protocoles, la négociation, les abonnements
— repose sur ces trois opérations.

Les includes nécessaires
-------------------------

.. code-block:: cpp

   #include <gagent/messaging/ACLMessage.hpp>
   #include <gagent/messaging/AgentIdentifier.hpp>
   #include <gagent/messaging/AclMQ.hpp>

   using namespace gagent;
   using namespace gagent::messaging;

Créer un message
-----------------

Un message FIPA ACL se construit en précisant sa **performative** puis
en renseignant ses champs un par un.

.. code-block:: cpp

   // 1. Créer le message avec sa performative
   ACLMessage msg(ACLMessage::Performative::INFORM);

   // 2. Préciser qui envoie et qui reçoit
   msg.setSender  (AgentIdentifier{"alice"});
   msg.addReceiver(AgentIdentifier{"bob"});

   // 3. Définir le contenu
   msg.setContent("la température est de 23°C");

   // Champs optionnels — utiles pour organiser les échanges
   msg.setConversationId("conv-001");   // relie les messages d'un même échange
   msg.setOntology("meteo");            // vocabulaire partagé
   msg.setLanguage("fipa-sl");          // langage du contenu

Seuls la performative et le contenu sont vraiment indispensables pour
un échange simple. Les autres champs deviennent utiles dès que plusieurs
conversations se déroulent en parallèle.

Préparer la réception avant d'envoyer
---------------------------------------

Avant de pouvoir recevoir des messages, l'agent doit **ouvrir sa boîte
de réception**. Cela se fait avec ``acl_bind()`` dans ``onStart()``.

.. code-block:: cpp

   void onStart() override {
       acl_bind("alice");   // ouvre la boîte de réception de l'agent "alice"
   }

.. warning::

   Appelez toujours ``acl_bind()`` dans ``onStart()``, **avant** le
   premier ``acl_receive()``. Si l'autre agent envoie un message avant
   que la boîte soit ouverte, le message est perdu.

Envoyer un message
-------------------

``acl_send()`` envoie un message à un agent identifié par son nom.

.. code-block:: cpp

   acl_send("bob", msg);

C'est tout. Que bob soit sur la même machine ou sur une machine distante,
la syntaxe est identique. Le framework se charge du transport.

``acl_send()`` retourne ``true`` si l'envoi a réussi, ``false`` sinon
(agent inexistant, réseau indisponible...).

.. code-block:: cpp

   if (!acl_send("bob", msg)) {
       std::cout << "Envoi échoué." << std::endl;
   }

Recevoir un message
--------------------

``acl_receive()`` attend qu'un message arrive dans la boîte de l'agent.
Elle prend deux paramètres : le nom de l'agent et un **timeout** en
millisecondes.

.. code-block:: cpp

   auto opt = acl_receive("alice", 5000);   // attend jusqu'à 5 secondes

   if (!opt) {
       // Aucun message reçu dans le délai imparti
       std::cout << "Timeout — aucun message." << std::endl;
       return;
   }

   // Un message est arrivé
   const ACLMessage& msg = *opt;
   std::cout << "Reçu de " << msg.getSender().name << std::endl;
   std::cout << "Contenu : " << msg.getContent() << std::endl;

``acl_receive()`` retourne un ``std::optional<ACLMessage>`` :

- **Un message** si quelque chose est arrivé avant le timeout
- **Vide** (``nullopt``) si le délai est écoulé sans message

Limitation — une seule file par nom d'agent
--------------------------------------------

.. warning::

   ``acl_bind("bob")`` ouvre **une seule** file de réception pour le nom
   ``bob``. Tous les behaviours de l'agent qui appellent
   ``acl_receive("bob", …)`` puisent dans cette même file, et **le premier
   thread réveillé emporte le message**, qu'il le concerne ou non. S'il ne le
   concerne pas, il l'écarte — et le message est perdu, car rien ne le remet
   dans la file.

Tant qu'un agent ne mène qu'une conversation à la fois, le comportement est
correct. Le problème apparaît avec un agent **multi-protocoles**. Exemple :
``bob`` est à la fois ``RequestParticipant`` (il attend des REQUEST) et
``ContractNetParticipant`` (il attend des CFP), les deux liés au nom ``bob``,
chacun dans son thread. Un CFP arrive :

1. Les deux threads attendent sur la même file.
2. Le thread Request se réveille le premier et récupère le CFP.
3. La performative n'est pas REQUEST, il abandonne le message
   (``Request.hpp``, ``RequestParticipant::action()``).
4. Le behaviour Contract Net ne verra jamais ce CFP.

Les protocoles filtrent de la même façon sur l'identifiant de conversation :
un message dont le ``conversation-id`` ne correspond pas est écarté, avec le
commentaire « pas pour nous » (``Request.hpp:136``, ``ContractNet.hpp:168``,
``SubscribeNotify.hpp:160``…). Le filtrage est correct du point de vue du
protocole, mais le message est déjà sorti de la file.

**Contournement** — donner un nom distinct à chaque protocole hébergé par
l'agent :

.. code-block:: cpp

   void setup() override {
       addBehaviour(new MonServeurRequest(this, "bob-req"));   // file « bob-req »
       addBehaviour(new MonParticipantCNP(this, "bob-cnp"));   // file « bob-cnp »
   }

Chaque behaviour possède alors sa propre file et plus rien n'est perdu. En
contrepartie, l'agent expose plusieurs noms, ce qui s'écarte du principe FIPA
« un agent = un nom » : les interlocuteurs doivent savoir à quel alias écrire.

.. note::

   La correction de fond consiste à introduire une boîte aux lettres au niveau
   de l'agent : un lecteur unique draine la file, et les behaviours réclament
   les messages **correspondant à un critère** (conversation-id, performative,
   protocole, expéditeur) — l'équivalent du ``MessageTemplate`` de JADE. Ce qui
   ne correspond pas reste en boîte pour un autre behaviour. Ce chantier n'est
   pas engagé : il modifie l'API publique de réception.

Lire le contenu d'un message reçu
-----------------------------------

.. code-block:: cpp

   const ACLMessage& msg = *opt;

   // Qui a envoyé ce message ?
   std::string expediteur = msg.getSender().name;

   // Quelle est la performative ?
   auto perf = msg.getPerformative();
   if (perf == ACLMessage::Performative::INFORM) { /* ... */ }
   if (perf == ACLMessage::Performative::REQUEST) { /* ... */ }

   // Quel est le contenu ?
   std::string contenu = msg.getContent();

   // À quelle conversation appartient ce message ?
   std::string conv = msg.getConversationId();

Répondre à un message
----------------------

La méthode ``createReply()`` crée automatiquement une réponse en
reprenant les champs de la conversation (``conversation-id``,
``in-reply-to``, destinataire inversé). C'est le moyen recommandé de
répondre plutôt que de construire un nouveau message à la main.

.. code-block:: cpp

   auto opt = acl_receive("bob", 5000);
   if (!opt) return;

   const ACLMessage& demande = *opt;

   // Créer une réponse INFORM
   ACLMessage reponse = demande.createReply(ACLMessage::Performative::INFORM);
   reponse.setSender (AgentIdentifier{"bob"});
   reponse.setContent("voici ma réponse");

   // Envoyer la réponse à l'expéditeur original
   acl_send(demande.getSender().name, reponse);

Exemple complet — deux agents qui se parlent
---------------------------------------------

Voici un exemple minimal : Alice envoie un message à Bob, Bob lui
répond, Alice affiche la réponse.

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/core/AgentCore.hpp>
   #include <gagent/messaging/ACLMessage.hpp>
   #include <gagent/messaging/AgentIdentifier.hpp>
   #include <gagent/messaging/AclMQ.hpp>
   #include <iostream>

   using namespace gagent;
   using namespace gagent::messaging;

   // ── Bob : reçoit une question et répond ──────────────────────────────────

   class BobBehaviour : public Behaviour {
   public:
       BobBehaviour(Agent* ag) : Behaviour(ag) {}

       void onStart() override {
           acl_bind("bob");
       }

       void action() override {
           auto opt = acl_receive("bob", 5000);
           if (!opt) return;

           const ACLMessage& question = *opt;
           std::cout << "[Bob] Reçu de " << question.getSender().name
                     << " : " << question.getContent() << std::endl;

           ACLMessage reponse = question.createReply(ACLMessage::Performative::INFORM);
           reponse.setSender (AgentIdentifier{"bob"});
           reponse.setContent("42");
           acl_send(question.getSender().name, reponse);
       }

       bool done() override { return false; }   // tourne en continu
   };

   class BobAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new BobBehaviour(this));
       }
   };

   // ── Alice : envoie une question et attend la réponse ─────────────────────

   class AliceBehaviour : public Behaviour {
       bool done_ = false;
   public:
       AliceBehaviour(Agent* ag) : Behaviour(ag) {}

       void onStart() override {
           acl_bind("alice");
       }

       void action() override {
           // Envoyer la question
           ACLMessage question(ACLMessage::Performative::REQUEST);
           question.setSender  (AgentIdentifier{"alice"});
           question.addReceiver(AgentIdentifier{"bob"});
           question.setContent ("quelle est la réponse à tout ?");
           question.setConversationId("conv-001");
           acl_send("bob", question);

           // Attendre la réponse (5 secondes max)
           auto opt = acl_receive("alice", 5000);
           if (!opt) {
               std::cout << "[Alice] Pas de réponse." << std::endl;
           } else {
               std::cout << "[Alice] Réponse de " << opt->getSender().name
                         << " : " << opt->getContent() << std::endl;
           }
           done_ = true;
       }

       bool done() override { return done_; }
   };

   class AliceAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new AliceBehaviour(this));
       }
   };

   // ── main ─────────────────────────────────────────────────────────────────

   int main() {
       AgentCore::initAgentSystem();

       BobAgent   bob;
       AliceAgent alice;

       bob.init();
       alice.init();

       AgentCore::syncAgentSystem();  // attend que tous les agents terminent
       return 0;
   }

Résultat :

.. code-block:: text

   [Bob] Reçu de alice : quelle est la réponse à tout ?
   [Alice] Réponse de bob : 42

Résumé des fonctions
---------------------

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Fonction
     - Rôle
   * - ``acl_bind("nom")``
     - Ouvre la boîte de réception — à appeler dans ``onStart()``
   * - ``acl_send("destinataire", msg)``
     - Envoie un message à un agent
   * - ``acl_receive("nom", timeout_ms)``
     - Attend un message, retourne vide si timeout
   * - ``msg.createReply(performative)``
     - Crée une réponse en reprenant les champs de conversation
