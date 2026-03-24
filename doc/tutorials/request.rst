7 — Le protocole Request
=========================

Le protocole **Request** est le plus simple et le plus courant des
protocoles FIPA. Il modélise une situation du quotidien : un agent
demande à un autre d'effectuer quelque chose et attend sa réponse.

La logique du protocole
------------------------

Pensez à un client qui passe une commande à un prestataire. Le client
formule sa demande, le prestataire peut accepter et livrer, refuser, ou
signaler un problème. Le protocole Request formalise exactement ce
scénario.

.. code-block:: text

   Demandeur                        Exécuteur
       │                                │
       │──────── REQUEST ───────────────►│  "fais ceci"
       │                                │
       │◄─────── INFORM ─────────────────│  succès, voici le résultat
       │   ou    REFUSE                  │  refus, voici pourquoi
       │   ou    AGREE + INFORM          │  accepté, résultat à venir
       │   ou    AGREE + FAILURE         │  accepté, mais échec en cours d'exécution

Le cas le plus fréquent est **REQUEST → INFORM** : la demande aboutit
directement. Les autres cas permettent de gérer les situations plus
complexes.

Quand utiliser AGREE ?
~~~~~~~~~~~~~~~~~~~~~~~

Quand le traitement prend du temps, l'exécuteur envoie d'abord un
``AGREE`` pour dire "j'ai reçu ta demande, je m'en occupe" puis, une
fois le travail terminé, envoie l'``INFORM`` avec le résultat.

Sans ``AGREE``, le demandeur ne saurait pas si l'exécuteur a bien reçu
la demande ou si elle s'est perdue en chemin.

Les deux rôles
---------------

- **L'initiateur** (``RequestInitiator``) — celui qui formule la demande
  et gère les différentes réponses possibles
- **Le participant** (``RequestParticipant``) — celui qui reçoit les
  demandes, les traite et renvoie une réponse

L'initiateur — ``RequestInitiator``
-------------------------------------

L'initiateur envoie la requête et attend la réponse. Vous n'implémentez
que les méthodes correspondant aux réponses qui vous intéressent.

**Constructeur :**

.. code-block:: cpp

   RequestInitiator(ag, "mon-nom", "nom-cible", "contenu", "ontologie", timeout_ms)
   //               │    │          │             │           │            │
   //               │    │          │             │           │            └ délai max (ms)
   //               │    │          │             │           └ optionnel
   //               │    │          │             └ ce que vous demandez
   //               │    │          └ à qui vous envoyez la demande
   //               │    └ votre nom
   //               └ this (l'agent)

**Méthodes à surcharger :**

.. list-table::
   :widths: 35 15 50
   :header-rows: 1

   * - Méthode
     - Obligatoire
     - Appelée quand...
   * - ``handleInform(msg)``
     - Non
     - La demande a abouti — ``msg`` contient le résultat
   * - ``handleRefuse(msg)``
     - Non
     - La demande a été refusée — ``msg`` contient la raison
   * - ``handleFailure(msg)``
     - Non
     - L'exécuteur avait accepté mais a échoué
   * - ``handleAgree(msg)``
     - Non
     - L'exécuteur a accepté la demande (résultat à venir)
   * - ``handleTimeout()``
     - Non
     - Aucune réponse dans le délai imparti

**Exemple — demander un calcul :**

.. code-block:: cpp

   #include <gagent/protocols/Request.hpp>
   using namespace gagent::protocols;

   class DemandeCalcul : public RequestInitiator {
   public:
       DemandeCalcul(Agent* ag)
           : RequestInitiator(
               ag,
               "alice",       // mon nom
               "calculateur", // à qui je demande
               "6 * 7",       // ce que je demande
               "",            // ontologie (ici vide)
               5000           // attendre 5 secondes max
             )
       {}

       void handleInform(const ACLMessage& msg) override {
           std::cout << "[Alice] Résultat : " << msg.getContent() << std::endl;
       }

       void handleRefuse(const ACLMessage& msg) override {
           std::cout << "[Alice] Refusé : " << msg.getContent() << std::endl;
       }

       void handleTimeout() override {
           std::cout << "[Alice] Pas de réponse." << std::endl;
       }
   };

Le participant — ``RequestParticipant``
-----------------------------------------

Le participant tourne en continu, écoute les demandes entrantes, et pour
chacune appelle ``handleRequest()`` pour construire la réponse.

Vous n'implémentez qu'**une seule méthode** : ``handleRequest()``. Elle
reçoit la demande et doit retourner un message de réponse — soit
``INFORM`` avec le résultat, soit ``REFUSE`` ou ``FAILURE`` selon le cas.

.. code-block:: cpp

   class ServiceCalcul : public RequestParticipant {
   public:
       ServiceCalcul(Agent* ag)
           : RequestParticipant(ag, "calculateur")
       {}

       ACLMessage handleRequest(const ACLMessage& req) override {
           std::string demande = req.getContent();  // ex : "6 * 7"

           // Traiter la demande
           int resultat = 42;  // votre logique ici

           // Retourner le résultat
           ACLMessage reponse = req.createReply(ACLMessage::Performative::INFORM);
           reponse.setSender(AgentIdentifier{"calculateur"});
           reponse.setContent(std::to_string(resultat));
           return reponse;
       }
   };

Pour refuser une demande :

.. code-block:: cpp

   ACLMessage handleRequest(const ACLMessage& req) override {
       if (/* condition de refus */) {
           ACLMessage refus = req.createReply(ACLMessage::Performative::REFUSE);
           refus.setSender(AgentIdentifier{"calculateur"});
           refus.setContent("opération non supportée");
           return refus;
       }
       // sinon traiter normalement...
   }

Cas avec AGREE — traitement long
----------------------------------

Quand le traitement prend du temps, le participant peut envoyer un
``AGREE`` immédiatement, puis envoyer l'``INFORM`` une fois le travail
terminé. Cela se fait manuellement dans le behaviour du participant :

.. code-block:: cpp

   class ServiceLent : public Behaviour {
       enum class Etat { ATTENTE, TRAITEMENT } etat_ = Etat::ATTENTE;
       ACLMessage demande_en_cours_;
   public:
       ServiceLent(Agent* ag) : Behaviour(ag) {}

       void onStart() override {
           acl_bind("service-lent");
       }

       void action() override {
           if (etat_ == Etat::ATTENTE) {
               auto opt = acl_receive("service-lent", 500);
               if (!opt || opt->getPerformative() != ACLMessage::Performative::REQUEST)
                   return;

               demande_en_cours_ = *opt;

               // Confirmer immédiatement
               ACLMessage agree = demande_en_cours_.createReply(ACLMessage::Performative::AGREE);
               agree.setSender(AgentIdentifier{"service-lent"});
               acl_send(demande_en_cours_.getSender().name, agree);

               etat_ = Etat::TRAITEMENT;

           } else {
               // Simuler un traitement long
               sleep(3);

               ACLMessage inform = demande_en_cours_.createReply(ACLMessage::Performative::INFORM);
               inform.setSender(AgentIdentifier{"service-lent"});
               inform.setContent("traitement terminé");
               acl_send(demande_en_cours_.getSender().name, inform);

               etat_ = Etat::ATTENTE;
           }
       }

       bool done() override { return false; }
   };

Exemple complet
----------------

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/core/AgentCore.hpp>
   #include <gagent/protocols/Request.hpp>
   #include <iostream>

   using namespace gagent;
   using namespace gagent::protocols;
   using namespace gagent::messaging;

   // ── Le service : répond aux demandes de calcul ───────────────────────────

   class ServiceCalcul : public RequestParticipant {
   public:
       ServiceCalcul(Agent* ag) : RequestParticipant(ag, "calculateur") {}

       ACLMessage handleRequest(const ACLMessage& req) override {
           std::cout << "[Calculateur] Demande reçue : "
                     << req.getContent() << std::endl;

           ACLMessage rep = req.createReply(ACLMessage::Performative::INFORM);
           rep.setSender(AgentIdentifier{"calculateur"});
           rep.setContent("42");
           return rep;
       }
   };

   class AgentCalculateur : public Agent {
   public:
       void setup() override {
           addBehaviour(new ServiceCalcul(this));
       }
   };

   // ── Le client : envoie une demande et traite la réponse ──────────────────

   class DemandeCalcul : public RequestInitiator {
   public:
       DemandeCalcul(Agent* ag)
           : RequestInitiator(ag, "alice", "calculateur", "6 * 7", "", 5000) {}

       void handleInform(const ACLMessage& msg) override {
           std::cout << "[Alice] Résultat : " << msg.getContent() << std::endl;
       }

       void handleRefuse(const ACLMessage& msg) override {
           std::cout << "[Alice] Refusé : " << msg.getContent() << std::endl;
       }

       void handleTimeout() override {
           std::cout << "[Alice] Pas de réponse dans les délais." << std::endl;
       }
   };

   class AgentAlice : public Agent {
   public:
       void setup() override {
           addBehaviour(new DemandeCalcul(this));
       }
   };

   // ── main ─────────────────────────────────────────────────────────────────

   int main() {
       AgentCore::initAgentSystem();

       AgentCalculateur calculateur;
       AgentAlice       alice;

       calculateur.init();
       alice.init();

       AgentCore::syncAgentSystem();  // attend que tous les agents terminent
       return 0;
   }

Résultat :

.. code-block:: text

   [Calculateur] Demande reçue : 6 * 7
   [Alice] Résultat : 42

Résumé
-------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Élément
     - Rôle
   * - ``RequestInitiator``
     - Envoie la demande, gère toutes les réponses possibles via des callbacks
   * - ``RequestParticipant``
     - Écoute les demandes en continu, retourne une réponse via ``handleRequest()``
   * - ``handleRequest()``
     - Seule méthode obligatoire côté participant — retourner INFORM, REFUSE ou FAILURE
   * - ``handleInform()``
     - Côté initiateur — la demande a abouti
   * - ``handleRefuse()``
     - Côté initiateur — la demande a été refusée
   * - ``handleTimeout()``
     - Côté initiateur — pas de réponse dans le délai
