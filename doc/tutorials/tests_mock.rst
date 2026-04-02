Tester ses agents avec MockTransport
=====================================

Les tests habituels des agents gAgent démarrent de vrais processus,
ouvrent des sockets ZeroMQ et attendent des messages réseau. Cela
prend plusieurs secondes et peut échouer pour des raisons extérieures
au code (port occupé, timing réseau, etc.).

``MockTransport`` résout ce problème : c'est un transport **en mémoire**
qui permet de tester les protocoles FIPA sans aucune infrastructure
réseau, sans ``fork()``, et en quelques millisecondes.

Principe
---------

.. code-block:: text

   Test normal (ZMQ)                  Test avec MockTransport
   ─────────────────────────          ─────────────────────────
   fork() × 2 processus               Un seul processus
   Sockets ZeroMQ                     Files en mémoire (MockBus)
   Attente de timing réseau           Déterministe, instantané
   ~1-16 secondes par test            < 1 milliseconde par test

La mécanique est simple : ``MockBus`` est une boîte aux lettres
partagée entre tous les agents du test. Quand Alice envoie à Bob,
le message atterrit immédiatement dans la file de Bob. Bob peut
le lire au prochain appel à ``receive()``.

Les composants
---------------

.. code-block:: cpp

   #include <gagent/messaging/MockTransport.hpp>
   using namespace gagent::messaging;

   // Le bus partagé — une seule instance par test
   auto bus = std::make_shared<MockBus>();

   // Un transport par agent, tous branchés sur le même bus
   auto transport_alice = std::make_shared<MockTransport>(bus);
   auto transport_bob   = std::make_shared<MockTransport>(bus);

``MockBus`` expose aussi une méthode utilitaire pour les assertions :

.. code-block:: cpp

   // Combien de messages Bob a-t-il en attente ?
   int n = bus->pending("bob");

L'agent de test : ``StubAgent``
---------------------------------

Les agents normaux démarrent avec ``init()``, ce qui lance un ``fork()``.
Pour les tests, on crée un agent minimal sans fork :

.. code-block:: cpp

   // À déclarer dans votre fichier de test
   class StubAgent : public Agent {
   public:
       explicit StubAgent(std::shared_ptr<ITransport> t) {
           setTransport(std::move(t));
       }
       void setup() override {}  // pas de behaviours automatiques
   };

Piloter les state machines manuellement
-----------------------------------------

Sans threads ni event loop, vous appelez ``onStart()`` et ``action()``
vous-même dans l'ordre voulu. Chaque appel à ``action()`` fait avancer
la machine à états d'un pas.

.. code-block:: cpp

   // Créer les agents de test
   StubAgent alice(std::make_shared<MockTransport>(bus));
   StubAgent bob  (std::make_shared<MockTransport>(bus));

   // Instancier les behaviours directement
   MonRequester req(&alice, ...);
   MonServeur   srv(&bob,   ...);

   // Initialiser (équivalent de onStart() appelé par le framework)
   req.onStart();
   srv.onStart();

   // Piloter pas à pas
   req.action();   // Alice envoie REQUEST
   srv.action();   // Bob reçoit REQUEST, envoie INFORM
   req.action();   // Alice reçoit INFORM → handleInform()

   // Vérifier le résultat
   assert(req.done());

Exemple 1 — Protocole Request
-------------------------------

Test complet du flux REQUEST → INFORM :

.. code-block:: cpp

   #include <gagent/messaging/MockTransport.hpp>
   #include <gagent/protocols/Request.hpp>
   #include <cassert>
   using namespace gagent::messaging;
   using namespace gagent::protocols;

   class StubAgent : public Agent {
   public:
       explicit StubAgent(std::shared_ptr<ITransport> t) { setTransport(std::move(t)); }
       void setup() override {}
   };

   int main() {
       auto bus = std::make_shared<MockBus>();
       StubAgent alice(std::make_shared<MockTransport>(bus));
       StubAgent bob  (std::make_shared<MockTransport>(bus));

       bool inform_recu = false;

       // Définir les comportements
       struct Client : RequestInitiator {
           bool& flag_;
           Client(Agent* ag, bool& f)
               : RequestInitiator(ag, "alice", "bob", "6*7", "math", 500)
               , flag_(f) {}
           void handleInform(const ACLMessage& msg) override {
               std::cout << "Résultat : " << msg.getContent() << "\n";
               flag_ = true;
           }
       };

       struct Serveur : RequestParticipant {
           Serveur(Agent* ag) : RequestParticipant(ag, "bob") {}
           ACLMessage handleRequest(const ACLMessage& req) override {
               auto r = req.createReply(ACLMessage::Performative::INFORM);
               r.setSender(AgentIdentifier{"bob"});
               r.setContent("42");
               return r;
           }
       };

       Client  client(&alice, inform_recu);
       Serveur serveur(&bob);

       client.onStart();   // bind("alice")
       serveur.onStart();  // bind("bob")

       client.action();    // envoie REQUEST à bob
       serveur.action();   // reçoit REQUEST, envoie INFORM à alice
       client.action();    // reçoit INFORM → handleInform()

       assert(inform_recu);         // alice a reçu la réponse
       assert(client.done());       // protocole terminé
       std::cout << "Test OK\n";
   }

Résultat :

.. code-block:: text

   Résultat : 42
   Test OK

Exemple 2 — AGREE + INFORM différé
-------------------------------------

Tester le flux REQUEST → AGREE → INFORM :

.. code-block:: cpp

   struct ServeurLent : RequestParticipant {
       ServeurLent(Agent* ag) : RequestParticipant(ag, "bob") {}

       bool prepareAgree(const ACLMessage&) override { return true; }

       ACLMessage handleRequest(const ACLMessage& req) override {
           // Pas de sleep en test — le MockTransport est synchrone
           auto r = req.createReply(ACLMessage::Performative::INFORM);
           r.setContent("traitement terminé");
           return r;
       }
   };

   bool agree_recu  = false;
   bool inform_recu = false;

   struct ClientAgreeable : RequestInitiator {
       bool& agree_;
       bool& inform_;
       ClientAgreeable(Agent* ag, bool& a, bool& i)
           : RequestInitiator(ag, "alice", "bob", "tâche", "", 500)
           , agree_(a), inform_(i) {}
       void handleAgree (const ACLMessage&) override { agree_  = true; }
       void handleInform(const ACLMessage&) override { inform_ = true; }
   };

   ClientAgreeable client(&alice, agree_recu, inform_recu);
   ServeurLent     serveur(&bob);

   client.onStart();
   serveur.onStart();

   client.action();   // envoie REQUEST
   serveur.action();  // reçoit REQUEST → envoie AGREE puis INFORM
   client.action();   // reçoit AGREE → handleAgree()
   client.action();   // reçoit INFORM → handleInform()

   assert(agree_recu && inform_recu);

Exemple 3 — Contract Net
--------------------------

Tester un appel d'offres complet avec deux participants :

.. code-block:: cpp

   #include <gagent/protocols/ContractNet.hpp>
   using namespace gagent::protocols;

   // Manager : prend l'offre la moins chère
   struct Manager : ContractNetInitiator {
       std::string& gagnant_;
       Manager(Agent* ag, std::vector<AgentIdentifier> parts, std::string& g)
           : ContractNetInitiator(ag, "manager", make_cfp(), parts, 500, 500)
           , gagnant_(g) {}

       std::vector<std::string> selectProposals(
               const std::vector<ACLMessage>& propositions) override {
           auto best = std::min_element(propositions.begin(), propositions.end(),
               [](const ACLMessage& a, const ACLMessage& b) {
                   return std::stoi(a.getContent()) < std::stoi(b.getContent());
               });
           gagnant_ = best->getSender().name;
           return { gagnant_ };
       }

       static ACLMessage make_cfp() {
           ACLMessage cfp(ACLMessage::Performative::CFP);
           cfp.setContent("tâche");
           return cfp;
       }
   };

   // Worker : propose un coût fixe
   struct Worker : ContractNetParticipant {
       int cout_;
       Worker(Agent* ag, const std::string& nom, int c)
           : ContractNetParticipant(ag, nom, 500), cout_(c) {}
       ACLMessage prepareProposal(const ACLMessage&) override {
           ACLMessage p(ACLMessage::Performative::PROPOSE);
           p.setContent(std::to_string(cout_));
           return p;
       }
       ACLMessage executeTask(const ACLMessage&) override {
           ACLMessage r(ACLMessage::Performative::INFORM);
           r.setContent("fait");
           return r;
       }
   };

   // ── Test ──────────────────────────────────────────────────────────────────

   auto bus = std::make_shared<MockBus>();
   StubAgent ag_manager(std::make_shared<MockTransport>(bus));
   StubAgent ag_bob    (std::make_shared<MockTransport>(bus));
   StubAgent ag_carol  (std::make_shared<MockTransport>(bus));

   std::string gagnant;
   std::vector<AgentIdentifier> parts = {
       AgentIdentifier{"bob"}, AgentIdentifier{"carol"}
   };

   Manager manager(&ag_manager, parts, gagnant);
   Worker  bob    (&ag_bob,   "bob",   10);  // coût 10
   Worker  carol  (&ag_carol, "carol",  5);  // coût 5  ← moins chère

   manager.onStart(); bob.onStart(); carol.onStart();

   manager.action();  // envoie CFP à bob et carol
   bob.action();      // reçoit CFP → envoie PROPOSE(10)
   carol.action();    // reçoit CFP → envoie PROPOSE(5)
   manager.action();  // reçoit PROPOSE de bob  (1/2)
   manager.action();  // reçoit PROPOSE de carol (2/2)
   manager.action();  // HANDLE_PROPOSALS → ACCEPT carol, REJECT bob
   bob.action();      // reçoit REJECT → done
   carol.action();    // reçoit ACCEPT → execute → envoie INFORM
   manager.action();  // reçoit INFORM → handleInform → state=DONE
   manager.action();  // DONE → done_ = true

   assert(gagnant == "carol");   // carol sélectionnée (moins chère)
   assert(manager.done());
   assert(bob.done());
   assert(carol.done());

Bonnes pratiques
-----------------

**Nommer les agents différemment par test**

Pour éviter les collisions entre tests qui tournent en parallèle, préfixez
les noms avec un identifiant de test :

.. code-block:: cpp

   // Dans test_calcul
   RequestInitiator(ag, "test1-alice", "test1-bob", ...)

   // Dans test_refus
   RequestInitiator(ag, "test2-alice", "test2-bob", ...)

**Vérifier les messages en attente**

Si un test échoue, ``bus->pending()`` permet de savoir si des messages
n'ont pas été consommés :

.. code-block:: cpp

   if (bus->pending("bob") > 0) {
       std::cerr << "Bob a des messages non lus !\n";
   }

**Pas de sleep() dans les tests MockTransport**

Les ``sleep()`` ou ``sleep_for()`` n'ont aucun sens avec MockTransport
car les messages arrivent instantanément. Supprimez-les dans vos
``handleRequest()`` lors des tests.

Résumé
-------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Élément
     - Rôle
   * - ``MockBus``
     - Bus en mémoire partagé entre les agents du test
   * - ``MockTransport``
     - Transport sans réseau — ``receive()`` retourne immédiatement
   * - ``StubAgent``
     - Agent minimal sans fork ni event loop
   * - ``onStart()`` / ``action()``
     - Piloter les state machines manuellement, pas à pas
   * - ``bus->pending(name)``
     - Vérifier les messages non consommés dans les assertions
