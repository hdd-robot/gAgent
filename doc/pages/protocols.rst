Protocoles d'interaction FIPA
==============================

Les protocoles d'interaction FIPA définissent des séquences standardisées
d'échanges ACL entre agents. Ils sont implémentés dans ``include/gagent/protocols/``
sous forme de ``Behaviour`` réutilisables.

Contract Net (FIPA SC00029H)
-----------------------------

Le **Contract Net Protocol** permet à un agent (initiateur) de déléguer une
tâche à l'agent le plus qualifié parmi plusieurs candidats (participants),
via un mécanisme d'enchère.

.. code-block:: text

   Initiateur          Participants
       │                    │
       │── CFP ────────────►│  "qui peut faire X ?"
       │                    │
       │◄── PROPOSE ────────│  offre avec contenu libre (ex: coût, délai)
       │◄── REFUSE ─────────│  refus (participant surchargé, hors périmètre…)
       │
       │  [selectProposals() — choisit le(s) gagnant(s)]
       │
       │── ACCEPT_PROPOSAL ►│  au(x) gagnant(s)
       │── REJECT_PROPOSAL ►│  aux autres
       │
       │◄── INFORM ─────────│  résultat de l'exécution
       │◄── FAILURE ─────────│  (si la tâche a échoué)

Inclusion
~~~~~~~~~

.. code-block:: cpp

   #include <gagent/protocols/ContractNet.hpp>
   using namespace gagent::protocols;

ContractNetInitiator
~~~~~~~~~~~~~~~~~~~~

Crée une sous-classe et surcharge les méthodes virtuelles :

.. code-block:: cpp

   class MonInitiateur : public ContractNetInitiator {
   public:
       MonInitiateur(Agent* ag,
                     const std::string& my_name,
                     const std::vector<AgentIdentifier>& participants)
           : ContractNetInitiator(
               ag,
               my_name,
               [](){
                   ACLMessage cfp(ACLMessage::Performative::CFP);
                   cfp.setContent("ma tâche");
                   cfp.setOntology("mon-ontologie");
                   return cfp;
               }(),
               participants,
               5000,   // délai offres (ms)
               10000)  // délai résultat (ms)
       {}

       // Sélectionne les agents à accepter parmi les propositions reçues
       std::vector<std::string> selectProposals(
               const std::vector<ACLMessage>& proposals) override
       {
           // Exemple : prendre la proposition avec le plus petit coût
           std::string best;
           int best_cost = INT_MAX;
           for (auto& p : proposals) {
               int cost = std::stoi(p.getContent());
               if (cost < best_cost) { best_cost = cost; best = p.getSender().name; }
           }
           return { best };
       }

       void handleInform(const ACLMessage& result) override {
           std::cout << "Résultat : " << result.getContent() << "\n";
           this_agent->doDelete();
       }

       void handleRefuse(const ACLMessage& msg) override {
           std::cout << msg.getSender().name << " a refusé\n";
       }
   };

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Méthode à surcharger
     - Description
   * - ``selectProposals(proposals)``
     - **Obligatoire.** Retourne les noms des agents à accepter.
   * - ``handleInform(msg)``
     - Résultat reçu du participant accepté.
   * - ``handleFailure(msg)``
     - Échec signalé par le participant.
   * - ``handleRefuse(msg)``
     - Refus d'un participant.

Paramètres du constructeur :

.. list-table::
   :header-rows: 1
   :widths: 30 70

   * - Paramètre
     - Description
   * - ``my_name``
     - Nom de l'initiateur — détermine la queue ``/acl_<my_name>``
   * - ``cfp``
     - Message CFP à envoyer (contenu déjà défini)
   * - ``participants``
     - Liste des ``AgentIdentifier`` destinataires
   * - ``proposal_timeout_ms``
     - Délai pour recevoir toutes les offres (défaut : 5000 ms)
   * - ``result_timeout_ms``
     - Délai pour recevoir le résultat d'exécution (défaut : 10000 ms)

ContractNetParticipant
~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: cpp

   class MonParticipant : public ContractNetParticipant {
   public:
       MonParticipant(Agent* ag, const std::string& my_name)
           : ContractNetParticipant(ag, my_name) {}

       // Évaluer le CFP et répondre PROPOSE ou REFUSE
       ACLMessage prepareProposal(const ACLMessage& cfp) override {
           if (surcharge_) {
               ACLMessage r(ACLMessage::Performative::REFUSE);
               r.setContent("surchargé");
               return r;
           }
           ACLMessage prop(ACLMessage::Performative::PROPOSE);
           prop.setContent(std::to_string(mon_tarif_));
           return prop;
       }

       // Exécuter la tâche acceptée et rapporter INFORM ou FAILURE
       ACLMessage executeTask(const ACLMessage& accept) override {
           // ... exécution ...
           ACLMessage result(ACLMessage::Performative::INFORM);
           result.setContent("terminé");
           this_agent->doDelete();
           return result;
       }
   };

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Méthode à surcharger
     - Description
   * - ``prepareProposal(cfp)``
     - **Obligatoire.** Retourne PROPOSE (offre) ou REFUSE (raison).
   * - ``executeTask(accept)``
     - **Obligatoire.** Exécute la tâche. Retourne INFORM ou FAILURE.

Le comportement se termine automatiquement (``done()`` = true) après :

- avoir envoyé un REFUSE
- avoir envoyé INFORM ou FAILURE suite à un ACCEPT
- avoir reçu un REJECT_PROPOSAL

Messagerie ACL — AclMQ
-----------------------

Les fonctions bas niveau sont disponibles séparément pour un usage direct :

.. code-block:: cpp

   #include <gagent/messaging/AclMQ.hpp>
   using namespace gagent::messaging;

   // Envoyer
   acl_send("bob", msg);

   // Recevoir avec timeout
   auto opt = acl_receive("alice", 5000);  // ms
   if (opt) { /* traiter *opt */ }

   // Libérer la queue en fin d'agent (dans takeDown())
   acl_unlink("alice");

Exemple complet
---------------

Voir ``tests/test_contract_net.cpp`` pour un exemple complet :
enchère de livraison avec 3 transporteurs et sélection du moins cher.

.. code-block:: bash

   cd build/tests
   ./test_contract_net
