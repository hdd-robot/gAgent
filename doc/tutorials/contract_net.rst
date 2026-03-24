8 — Le Contract Net
====================

Le protocole **Contract Net** est le protocole de **négociation et
d'attribution de tâches** entre agents. Il répond à une question très
concrète : *j'ai quelque chose à faire faire, qui peut le faire, et
lequel vais-je choisir ?*

La logique du Contract Net
---------------------------

Imaginez un chef de projet qui a une mission urgente à déléguer. Il ne
va pas choisir un prestataire au hasard — il va lancer un appel d'offres,
comparer les réponses, et retenir la meilleure proposition.

C'est exactement le fonctionnement du Contract Net :

1. Un agent **coordinateur** lance un appel d'offres (``CFP``)
2. Plusieurs agents **candidats** reçoivent l'appel et décident de
   proposer (``PROPOSE``) ou de refuser (``REFUSE``)
3. Le coordinateur compare toutes les propositions et **sélectionne**
   le ou les gagnants
4. Il envoie un ``ACCEPT_PROPOSAL`` au(x) gagnant(s) et un
   ``REJECT_PROPOSAL`` aux autres
5. Le ou les gagnants **exécutent la tâche** et renvoient le résultat
   (``INFORM``) ou signalent un échec (``FAILURE``)

Le déroulement complet
-----------------------

.. code-block:: text

   Coordinateur       Candidat A      Candidat B      Candidat C
        │                  │               │               │
        │──── CFP ─────────►───────────────►───────────────►
        │              "qui peut livrer Paris→Lyon ?"
        │                  │               │               │
        │◄─── PROPOSE ──────│               │               │  "120€"
        │◄─── PROPOSE ──────────────────────│               │  "95€"
        │◄─── REFUSE ───────────────────────────────────────│  "indisponible"
        │
        │  [sélection : B est le moins cher]
        │
        │──── ACCEPT ───────────────────────►  "tu as le contrat"
        │──── REJECT ───────│                               │
        │              "une meilleure offre a été retenue"
        │                   │               │
        │◄─── INFORM ───────────────────────│  "livraison effectuée"
        │

Cas particuliers
~~~~~~~~~~~~~~~~~

**Tous les candidats refusent** — personne n'est disponible.
Le coordinateur ne sélectionne personne, il n'envoie aucun ACCEPT.
La négociation se termine sans résultat.

.. code-block:: text

   Coordinateur       Candidat A      Candidat B
        │                  │               │
        │──── CFP ─────────►───────────────►
        │◄─── REFUSE ───────│               │
        │◄─── REFUSE ───────────────────────│
        │
        │  [aucune proposition → négociation terminée]

**Le gagnant échoue dans l'exécution** — il avait promis mais n'a pas
pu tenir.

.. code-block:: text

   Coordinateur       Candidat A
        │                  │
        │──── ACCEPT ───────►
        │◄─── FAILURE ──────│  "véhicule en panne"

**Plusieurs gagnants** — le coordinateur peut accepter plusieurs
propositions à la fois si la tâche peut être répartie.

Les deux rôles
--------------

Le protocole implique toujours deux types d'agents :

- **L'initiateur** (``ContractNetInitiator``) — celui qui lance l'appel
  d'offres et choisit
- **Le participant** (``ContractNetParticipant``) — celui qui répond à
  l'appel et exécute si retenu

Chaque rôle est un behaviour à ajouter dans ``setup()``.

L'initiateur — ``ContractNetInitiator``
-----------------------------------------

L'initiateur gère toute la mécanique du protocole automatiquement.
Vous n'implémentez que deux choses :

**1. ``selectProposals()``** — la logique de sélection

C'est ici que vous décidez qui retenir parmi toutes les propositions
reçues. Vous recevez la liste des messages ``PROPOSE`` et vous retournez
les noms des agents retenus.

**2. ``handleInform()``** — traiter le résultat

Appelé quand le gagnant a terminé sa tâche et envoie son résultat.

.. code-block:: cpp

   #include <gagent/protocols/ContractNet.hpp>
   using namespace gagent::protocols;
   using namespace gagent::messaging;

   class CoordinateurLivraison : public ContractNetInitiator {
   public:
       CoordinateurLivraison(Agent* ag, std::vector<AgentIdentifier> candidats)
           : ContractNetInitiator(
               ag,
               "coordinateur",          // mon nom
               preparerCFP(),           // le message CFP
               candidats,               // à qui l'envoyer
               5000,                    // attendre les offres 5 secondes max
               10000                    // attendre le résultat 10 secondes max
             )
       {}

       // Choisir le moins cher parmi les propositions
       std::vector<std::string> selectProposals(
               const std::vector<ACLMessage>& propositions) override
       {
           if (propositions.empty()) {
               std::cout << "Aucune proposition reçue." << std::endl;
               return {};
           }

           // Trouver la proposition avec le prix le plus bas
           const ACLMessage* meilleure = &propositions[0];
           for (const auto& prop : propositions) {
               int prix = std::stoi(prop.getContent());
               if (prix < std::stoi(meilleure->getContent()))
                   meilleure = &prop;
           }

           std::cout << "Gagnant : " << meilleure->getSender().name
                     << " pour " << meilleure->getContent() << "€" << std::endl;
           return { meilleure->getSender().name };
       }

       // Traiter le résultat de la livraison
       void handleInform(const ACLMessage& msg) override {
           std::cout << "Résultat : " << msg.getContent() << std::endl;
       }

       // Optionnel : traiter les refus
       void handleRefuse(const ACLMessage& msg) override {
           std::cout << msg.getSender().name << " est indisponible." << std::endl;
       }

   private:
       static ACLMessage preparerCFP() {
           ACLMessage cfp(ACLMessage::Performative::CFP);
           cfp.setContent("livraison Paris→Lyon, 500kg");
           return cfp;
       }
   };

   class AgentCoordinateur : public Agent {
       std::vector<AgentIdentifier> candidats_;
   public:
       AgentCoordinateur(std::vector<AgentIdentifier> c)
           : candidats_(std::move(c)) {}

       void setup() override {
           addBehaviour(new CoordinateurLivraison(this, candidats_));
       }
   };

Le participant — ``ContractNetParticipant``
--------------------------------------------

Le participant attend un CFP, décide s'il propose ou refuse, et si
retenu, exécute la tâche. Vous implémentez deux méthodes :

**1. ``prepareProposal()``** — décider de proposer ou refuser

Vous recevez le CFP et vous retournez soit un message ``PROPOSE`` avec votre
offre, soit un message ``REFUSE`` avec la raison.

**2. ``executeTask()``** — exécuter la tâche si retenu

Appelé uniquement si votre PROPOSE a été accepté. Vous effectuez le travail
et vous retournez un ``INFORM`` avec le résultat, ou un ``FAILURE`` si
quelque chose a mal tourné.

.. code-block:: cpp

   class TransporteurAgent : public ContractNetParticipant {
       std::string nom_;
       int         tarif_;
       bool        disponible_;
   public:
       TransporteurAgent(Agent* ag, std::string nom, int tarif, bool dispo)
           : ContractNetParticipant(ag, nom)
           , nom_(nom), tarif_(tarif), disponible_(dispo)
       {}

       ACLMessage prepareProposal(const ACLMessage& cfp) override {
           if (!disponible_) {
               // Je ne peux pas — je refuse
               ACLMessage refus(ACLMessage::Performative::REFUSE);
               refus.setContent("véhicule indisponible");
               return refus;
           }

           // Je peux — je propose mon tarif
           ACLMessage proposition(ACLMessage::Performative::PROPOSE);
           proposition.setContent(std::to_string(tarif_));
           return proposition;
       }

       ACLMessage executeTask(const ACLMessage& accept) override {
           std::cout << nom_ << " : livraison en cours..." << std::endl;
           sleep(2);  // simulation du travail

           ACLMessage resultat(ACLMessage::Performative::INFORM);
           resultat.setContent("livraison effectuée");
           return resultat;
       }
   };

   class AgentTransporteur : public Agent {
       std::string nom_;
       int         tarif_;
       bool        disponible_;
   public:
       AgentTransporteur(std::string nom, int tarif, bool dispo)
           : nom_(nom), tarif_(tarif), disponible_(dispo) {}

       void setup() override {
           addBehaviour(new TransporteurAgent(this, nom_, tarif_, disponible_));
       }
   };

Assembler le tout dans ``main()``
-----------------------------------

.. code-block:: cpp

   int main() {
       AgentCore::initAgentSystem();

       // Les trois candidats
       AgentTransporteur a("transporteur-a", 120, true);
       AgentTransporteur b("transporteur-b",  95, true);
       AgentTransporteur c("transporteur-c",   0, false);  // indisponible

       // Le coordinateur connaît les noms des candidats
       std::vector<AgentIdentifier> candidats = {
           {"transporteur-a"},
           {"transporteur-b"},
           {"transporteur-c"}
       };
       AgentCoordinateur coordinateur(candidats);

       // Lancement — l'ordre n'a pas d'importance
       a.init();
       b.init();
       c.init();
       coordinateur.init();

       // Attendre la fin de tous les agents
       AgentCore::syncAgentSystem();

       return 0;
   }

Résultat attendu :

.. code-block:: text

   transporteur-c est indisponible.
   Gagnant : transporteur-b pour 95€
   transporteur-b : livraison en cours...
   Résultat : livraison effectuée

Les timeouts
-------------

Le coordinateur attend les propositions pendant un délai configurable.
Si un candidat ne répond pas à temps, il est simplement ignoré — la
sélection se fait avec les propositions reçues.

.. code-block:: cpp

   ContractNetInitiator(
       ag,
       "coordinateur",
       cfp,
       candidats,
       5000,    // attendre les propositions 5 secondes max
       10000    // attendre le résultat 10 secondes max
   )

De même, le participant attend le CFP pendant un délai. Si aucun CFP
n'arrive, il se termine seul.

.. code-block:: cpp

   ContractNetParticipant(
       ag,
       "transporteur-a",
       8000    // attendre un CFP 8 secondes max
   )

.. tip::

   Configurez le timeout des participants plus long que le timeout
   des propositions de l'initiateur. Cela laisse le temps à l'initiateur
   d'envoyer le CFP avant que les participants ne s'éteignent.

Résumé des méthodes
--------------------

**Initiateur** (``ContractNetInitiator``) :

.. list-table::
   :widths: 35 15 50
   :header-rows: 1

   * - Méthode
     - Obligatoire
     - Rôle
   * - ``selectProposals(propositions)``
     - Oui
     - Choisir les gagnants parmi les PROPOSE reçus
   * - ``handleInform(msg)``
     - Non
     - Traiter le résultat de la tâche (INFORM)
   * - ``handleFailure(msg)``
     - Non
     - Traiter un échec d'exécution (FAILURE)
   * - ``handleRefuse(msg)``
     - Non
     - Être notifié des refus (REFUSE)

**Participant** (``ContractNetParticipant``) :

.. list-table::
   :widths: 35 15 50
   :header-rows: 1

   * - Méthode
     - Obligatoire
     - Rôle
   * - ``prepareProposal(cfp)``
     - Oui
     - Retourner un PROPOSE ou un REFUSE en réponse au CFP
   * - ``executeTask(accept)``
     - Oui
     - Exécuter la tâche et retourner un INFORM ou un FAILURE
