18 — AMS et DF : annuaires de la plateforme
=============================================

Dans un SMA FIPA, les agents ne se connaissent pas nécessairement à
l'avance. Deux services d'annuaire leur permettent de se trouver et de
s'identifier :

- **AMS** (*Agent Management System*) — le registre des agents. Il sait
  qui est vivant, quel est son PID, son adresse de messagerie, et son
  état.

- **DF** (*Directory Facilitator*) — l'annuaire des services. Un agent
  annonce ce qu'il sait faire ; les autres peuvent chercher "qui sait
  faire X ?".

.. code-block:: text

   ┌─────────────────────────────────────────────────────┐
   │                  agentplatform                      │
   │                                                     │
   │   ┌──────────┐              ┌──────────────────┐    │
   │   │   AMS    │              │       DF         │    │
   │   │ qui vit? │              │ qui fait quoi ?  │    │
   │   └──────────┘              └──────────────────┘    │
   └─────────────────────────────────────────────────────┘
         ▲  enregistrement automatique       ▲
         │  (géré par AgentCore)             │  registerService()
         │                                   │  (dans setup())
      Agent A                             Agent B

L'AMS est géré automatiquement par gAgent — vos agents s'y enregistrent
et s'en désenregistrent sans que vous n'ayez à le faire manuellement.
**Le DF, en revanche, c'est vous qui le renseignez.**

.. note::

   Chaque agent possède deux entrées dans l'AMS :

   - **Entrée interne** (nom aléatoire type ``ab12cd34``) — gérée
     automatiquement par ``AgentCore``, utilisée pour la supervision.

   - **Entrée de routage** (nom visible comme ``"alice"``) — créée par
     ``acl_bind("alice")`` dans votre ``setup()``. C'est cette entrée
     que ``acl_send("alice", msg)`` utilise pour résoudre l'endpoint ZMQ
     de destination, y compris en mode cluster.

La plateforme est-elle obligatoire ?
--------------------------------------

Non. Si ``agentplatform`` n'est pas lancé, les opérations AMS et DF
échouent silencieusement — votre SMA fonctionne en mode dégradé
(agents locaux uniquement, sans annuaire). La plateforme devient
indispensable dès que vous voulez :

- superviser votre système avec ``agentmanager``
- permettre à des agents de se découvrir dynamiquement via le DF
- déployer sur plusieurs machines

Le DF — Directory Facilitator
--------------------------------

Le DF répond à la question : *"je cherche un agent qui offre tel
service, qui l'est ?"*

**Déclarer un service dans ``setup()`` :**

.. code-block:: cpp

   #include <gagent/platform/DFClient.hpp>
   using namespace gagent::platform;

   class AgentPlanning : public Agent {
   public:
       void setup() override {
           DFClient df;
           df.registerService(
               "planificateur",   // nom de cet agent
               "planning",        // type de service
               "htn-planner",     // nom du service
               "fipa-sl",         // langage (optionnel)
               "logistics"        // ontologie (optionnel)
           );

           addBehaviour(new PlanningBehaviour(this));
       }

       void takeDown() override {
           DFClient df;
           df.deregisterAgent("planificateur");  // nettoyage
       }
   };

**Chercher un service depuis un autre agent :**

.. code-block:: cpp

   DFClient df;

   // Chercher par type
   auto services = df.search("planning");

   for (const auto& s : services) {
       std::cout << "Agent : " << s.agentName
                 << " — service : " << s.serviceName << std::endl;
   }

   // Chercher par type ET ontologie
   auto services2 = df.search("planning", "logistics");

**Exemple complet — découverte dynamique :**

Un agent coordinateur cherche dans le DF tous les agents de type
``"transport"`` et leur envoie un CFP.

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/platform/DFClient.hpp>
   #include <gagent/messaging/AclMQ.hpp>
   #include <gagent/messaging/ACLMessage.hpp>

   using namespace gagent;
   using namespace gagent::messaging;
   using namespace gagent::platform;

   class DecouverteBehaviour : public OneShotBehaviour {
   public:
       DecouverteBehaviour(Agent* ag) : OneShotBehaviour(ag) {}

       void action() override {
           DFClient df;
           auto transporteurs = df.search("transport");

           if (transporteurs.empty()) {
               std::cout << "[Coord] aucun transporteur trouvé." << std::endl;
               return;
           }

           std::cout << "[Coord] " << transporteurs.size()
                     << " transporteur(s) trouvé(s) :" << std::endl;

           ACLMessage cfp(ACLMessage::Performative::CFP);
           cfp.setSender(AgentIdentifier{"coordinateur"});
           cfp.setContent("livraison Paris→Lyon, 500kg");

           for (const auto& s : transporteurs) {
               std::cout << "  → " << s.agentName << std::endl;
               acl_send(s.agentName, cfp);
           }
       }
   };

   class AgentCoordinateur : public Agent {
   public:
       void setup() override {
           addBehaviour(new DecouverteBehaviour(this));
       }
   };

Déclarer plusieurs services
-----------------------------

Un agent peut déclarer plusieurs services de types différents :

.. code-block:: cpp

   void setup() override {
       DFClient df;

       df.registerService("mon-agent", "planning",   "htn-planner");
       df.registerService("mon-agent", "monitoring", "sensor-reader");
       df.registerService("mon-agent", "transport",  "road-carrier",
                          "fipa-sl", "logistics");
   }

L'AMS — consulter l'état des agents
--------------------------------------

L'AMS est principalement utilisé en interne par gAgent, mais vous
pouvez l'interroger directement si vous avez besoin de vérifier qu'un
agent est vivant avant de lui envoyer un message.

.. code-block:: cpp

   #include <gagent/platform/AMSClient.hpp>
   using namespace gagent::platform;

   AMSClient ams;

   // Vérifier qu'un agent est actif avant de lui écrire
   auto info = ams.lookup("planificateur");
   if (info && info->state == "active") {
       acl_send("planificateur", requete);
   }

   // Lister tous les agents enregistrés
   auto agents = ams.list();
   for (const auto& a : agents) {
       std::cout << a.name << " [" << a.state << "] pid=" << a.pid;
       if (!a.slave_ip.empty())
           std::cout << " slave=" << a.slave_ip;  // vide si agent local
       std::cout << std::endl;
   }

Résumé
-------

.. list-table::
   :widths: 20 40 40
   :header-rows: 1

   * - Service
     - Rôle
     - Votre action
   * - **AMS**
     - Registre des agents (nom, PID, endpoint ZMQ, état, machine)
     - Entrée interne : automatique. Entrée de routage :
       ``acl_bind("nom")`` dans ``setup()``,
       ``acl_unlink("nom")`` dans ``takeDown()``
   * - **DF**
     - Annuaire des services
     - ``registerService()`` dans ``setup()``,
       ``deregisterAgent()`` dans ``takeDown()``
   * - **DF search**
     - Découvrir des agents par compétence
     - ``df.search(type)`` ou ``df.search(type, ontologie)``
