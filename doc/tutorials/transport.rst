Le transport de messagerie
===========================

Par défaut, gAgent achemine les messages FIPA ACL via **ZeroMQ** (IPC
local ou TCP en cluster). Mais les protocoles FIPA eux-mêmes ne savent
pas comment les messages voyagent : ils passent par une interface
abstraite appelée ``ITransport``.

Cela signifie que vous pouvez remplacer ZeroMQ par un autre mécanisme —
email, HTTP, mémoire partagée, ou un simple simulateur de test — sans
toucher à vos agents ni à vos protocoles.

L'interface ``ITransport``
---------------------------

.. code-block:: cpp

   class ITransport {
   public:
       // Prépare la réception pour cet agent — retourne l'endpoint
       virtual std::string bind(const std::string& name) = 0;

       // Envoie un message ACL à l'agent <to>
       virtual bool send(const std::string& to, const ACLMessage& msg) = 0;

       // Attend un message pendant au plus timeout_ms ms (nullopt = timeout)
       virtual std::optional<ACLMessage> receive(const std::string& name,
                                                  int timeout_ms) = 0;

       // Libère la queue de <name>
       virtual void unlink(const std::string& name) = 0;

       // Vide les envois en attente
       virtual void flush() = 0;
   };

Le transport par défaut : ``ZmqTransport``
--------------------------------------------

Sans configuration particulière, chaque agent utilise ``ZmqTransport``
qui s'appuie sur ZeroMQ. Vous n'avez rien à faire — c'est transparent.

.. code-block:: cpp

   class MonAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new MonBehaviour(this));
           // ZmqTransport est instancié automatiquement
       }
   };

Injecter un transport personnalisé
------------------------------------

Pour changer de transport, appelez ``setTransport()`` **avant** ``init()`` :

.. code-block:: cpp

   MonAgent agent;
   agent.setTransport(std::make_shared<MonTransportCustom>());
   agent.init();

Les protocoles (Request, ContractNet, Subscribe-Notify) utiliseront
automatiquement le transport injecté, sans aucune modification de leur
code.

Écrire son propre transport
-----------------------------

Il suffit d'implémenter les 5 méthodes de l'interface. Voici un exemple
de **transport par fichier** (un fichier par agent, pour déboguer) :

.. code-block:: cpp

   #include <gagent/messaging/ITransport.hpp>
   #include <fstream>
   #include <filesystem>
   using namespace gagent::messaging;

   class FileTransport : public ITransport {
   public:
       std::string bind(const std::string& name) override {
           // Crée le fichier de queue si nécessaire
           std::string path = "/tmp/queue_" + name + ".acl";
           std::ofstream(path, std::ios::app); // crée si absent
           return "file://" + path;
       }

       bool send(const std::string& to, const ACLMessage& msg) override {
           std::ofstream f("/tmp/queue_" + to + ".acl", std::ios::app);
           if (!f) return false;
           f << msg.toString() << "\n---\n";
           return true;
       }

       std::optional<ACLMessage> receive(const std::string& name,
                                          int timeout_ms) override {
           std::string path = "/tmp/queue_" + name + ".acl";
           // Lecture simplifiée — dans un vrai transport, gérer le timeout
           std::ifstream f(path);
           if (!f) return std::nullopt;
           std::string content, line;
           while (std::getline(f, line)) {
               if (line == "---") break;
               content += line + "\n";
           }
           if (content.empty()) return std::nullopt;
           return ACLMessage::parse(content);
       }

       void unlink(const std::string& name) override {
           std::filesystem::remove("/tmp/queue_" + name + ".acl");
       }

       void flush() override {}
   };

.. note::

   Cet exemple est volontairement simplifié. Un transport de production
   devrait gérer la concurrence, le timeout réel, et les erreurs d'E/S.

Cas d'usage : transport par email
-----------------------------------

Pour des systèmes d'agents asynchrones (agents qui ne sont pas
simultanément actifs), un transport par email peut être pertinent.
Le protocole FIPA Request fonctionnerait alors sur des délais de minutes
ou d'heures plutôt que de millisecondes.

.. code-block:: cpp

   class EmailTransport : public ITransport {
   public:
       EmailTransport(const std::string& smtp_server,
                      const std::string& domain)
           : smtp_(smtp_server), domain_(domain) {}

       std::string bind(const std::string& name) override {
           return name + "@" + domain_;
       }

       bool send(const std::string& to, const ACLMessage& msg) override {
           return smtp_.sendMail(
               to + "@" + domain_,
               "FIPA-ACL",           // sujet
               msg.toString()        // corps
           );
       }

       std::optional<ACLMessage> receive(const std::string& name,
                                          int timeout_ms) override {
           auto raw = imap_.fetch(name + "@" + domain_, timeout_ms);
           if (!raw) return std::nullopt;
           return ACLMessage::parse(*raw);
       }

       void unlink(const std::string&) override {}
       void flush()                    override {}

   private:
       SmtpClient smtp_;
       ImapClient imap_;
       std::string domain_;
   };

   // Usage :
   MyAgent agent;
   agent.setTransport(
       std::make_shared<EmailTransport>("smtp.example.com", "agents.example.com")
   );
   agent.init();

Adapter les timeouts aux transports lents
------------------------------------------

Un transport email ou HTTP a des latences de secondes ou de minutes.
Les timeouts par défaut des protocoles (5-10 secondes) seraient trop
courts. Ajustez-les dans les constructeurs :

.. code-block:: cpp

   // Request avec timeout de 10 minutes
   class MonInitiateur : public RequestInitiator {
   public:
       MonInitiateur(Agent* ag)
           : RequestInitiator(ag, "alice", "bob", "analyse-rapport",
                              "rapports", 600000)  // 10 minutes
       {}
   };

   // ContractNet avec 1h pour collecter les offres
   class MonAppelOffre : public ContractNetInitiator {
   public:
       MonAppelOffre(Agent* ag, std::vector<AgentIdentifier> fournisseurs)
           : ContractNetInitiator(ag, "acheteur",
                                  cfp,
                                  fournisseurs,
                                  3600000,  // 1h pour les propositions
                                  3600000)  // 1h pour les résultats
       {}
   };

Résumé
-------

.. list-table::
   :widths: 30 70
   :header-rows: 1

   * - Élément
     - Rôle
   * - ``ITransport``
     - Interface abstraite — 5 méthodes à implémenter
   * - ``ZmqTransport``
     - Implémentation ZeroMQ (par défaut, transparente)
   * - ``MockTransport``
     - Transport en mémoire pour les tests unitaires (voir :doc:`tests_mock`)
   * - ``setTransport()``
     - Injecter un transport avant ``init()``
   * - ``transport()``
     - Accéder au transport depuis un behaviour
