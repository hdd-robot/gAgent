9 — Le protocole Subscribe-Notify
==================================

Le protocole **Subscribe-Notify** est le protocole d'**abonnement et de
notification**. Il répond à un besoin très courant : être prévenu
automatiquement chaque fois qu'un événement se produit, sans avoir à
demander à chaque fois.

La logique du protocole
------------------------

Pensez à une newsletter : vous vous abonnez une fois, et vous recevez
automatiquement chaque nouvel article sans avoir à en faire la demande.
C'est exactement le fonctionnement du Subscribe-Notify.

Un agent **abonné** (subscriber) dit à un agent **éditeur** (publisher)
"préviens-moi chaque fois que quelque chose change". L'éditeur envoie
alors des notifications automatiques à tous ses abonnés, à chaque
événement.

.. code-block:: text

   Abonné (Subscriber)              Éditeur (Publisher)
           │                               │
           │────── SUBSCRIBE ──────────────►│  "abonne-moi"
           │◄───── AGREE ───────────────────│  "abonnement accepté"
           │                               │
           │                          [événement]
           │◄───── INFORM ──────────────────│  notification 1
           │◄───── INFORM ──────────────────│  notification 2
           │◄───── INFORM ──────────────────│  notification 3
           │                               │
           │────── CANCEL ─────────────────►│  "je me désabonne"
           │◄───── INFORM ──────────────────│  dernier message (optionnel)

Ce qui distingue ce protocole des autres : **la conversation ne se
termine pas après un seul échange**. L'éditeur continue d'envoyer des
notifications jusqu'à ce que l'abonné envoie un ``CANCEL``.

Plusieurs abonnés simultanés
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Un éditeur peut avoir plusieurs abonnés en même temps. Chaque appel à
``notify()`` envoie une notification à tous les abonnés actifs.

.. code-block:: text

   Abonné A ─────SUBSCRIBE──────►
   Abonné B ─────SUBSCRIBE──────► Éditeur
   Abonné C ─────SUBSCRIBE──────►
                                  │
                             [événement]
                                  │
                    ◄──INFORM──── │ → A
                    ◄──INFORM──── │ → B
                    ◄──INFORM──── │ → C

Les deux rôles
---------------

- **L'abonné** (``SubscribeInitiator``) — s'abonne, reçoit les
  notifications, peut se désabonner
- **L'éditeur** (``SubscribeParticipant``) — gère les abonnements,
  publie les notifications quand l'état change

L'abonné — ``SubscribeInitiator``
-----------------------------------

L'abonné gère automatiquement l'envoi du ``SUBSCRIBE``, l'attente du
``AGREE``, et la réception des notifications. Vous n'implémentez que
les méthodes qui vous intéressent.

**Constructeur :**

.. code-block:: cpp

   SubscribeInitiator(ag, "mon-nom", "editeur", "sujet", "ontologie", timeout_ms)
   //                 │    │          │           │         │            │
   //                 │    │          │           │         │            └ délai pour recevoir le AGREE
   //                 │    │          │           │         └ optionnel
   //                 │    │          │           └ sujet de l'abonnement (contenu du SUBSCRIBE)
   //                 │    │          └ nom de l'éditeur
   //                 │    └ votre nom
   //                 └ this

**Méthodes à surcharger :**

.. list-table::
   :widths: 35 15 50
   :header-rows: 1

   * - Méthode
     - Obligatoire
     - Appelée quand...
   * - ``handleNotify(msg)``
     - Non
     - Une notification arrive — ``msg`` contient la valeur publiée
   * - ``handleRefuse(msg)``
     - Non
     - L'éditeur a refusé l'abonnement
   * - ``shouldCancel()``
     - Non
     - Appelée périodiquement — retourner ``true`` pour se désabonner

**Exemple — surveiller la température d'un capteur :**

.. code-block:: cpp

   #include <gagent/protocols/SubscribeNotify.hpp>
   using namespace gagent::protocols;

   class SurveillanceTemperature : public SubscribeInitiator {
       int  notifications_recues_ = 0;
   public:
       SurveillanceTemperature(Agent* ag)
           : SubscribeInitiator(
               ag,
               "moniteur",    // mon nom
               "capteur",     // l'éditeur auquel je m'abonne
               "temperature", // le sujet qui m'intéresse
               "meteo"        // ontologie
             )
       {}

       void handleNotify(const ACLMessage& msg) override {
           std::cout << "[Moniteur] Température : "
                     << msg.getContent() << "°C" << std::endl;
           notifications_recues_++;
       }

       void handleRefuse(const ACLMessage& msg) override {
           std::cout << "[Moniteur] Abonnement refusé : "
                     << msg.getContent() << std::endl;
       }

       // Se désabonner après 5 notifications
       bool shouldCancel() override {
           return notifications_recues_ >= 5;
       }
   };

L'éditeur — ``SubscribeParticipant``
--------------------------------------

L'éditeur gère la liste des abonnés et envoie les notifications. Vous
implémentez ``handleSubscribe()`` pour décider d'accepter ou refuser
un abonnement, puis appelez ``notify()`` quand l'état change.

**Méthodes à surcharger :**

.. list-table::
   :widths: 35 15 50
   :header-rows: 1

   * - Méthode
     - Obligatoire
     - Rôle
   * - ``handleSubscribe(msg)``
     - Non
     - Retourner ``true`` pour accepter, ``false`` pour refuser
   * - ``handleCancel(subscriber)``
     - Non
     - Appelée quand un abonné se désabonne — retourner un dernier message (ou ``""`` pour rien)

**Méthodes disponibles :**

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Méthode
     - Rôle
   * - ``notify(contenu)``
     - Envoie une notification à **tous** les abonnés actifs
   * - ``notifyOne(nom, contenu)``
     - Envoie une notification à **un seul** abonné
   * - ``subscriberCount()``
     - Retourne le nombre d'abonnés actifs

**Exemple — capteur qui publie la température toutes les secondes :**

.. code-block:: cpp

   class CapteurTemperature : public SubscribeParticipant {
       int valeur_ = 20;
   public:
       CapteurTemperature(Agent* ag)
           : SubscribeParticipant(ag, "capteur")
       {}

       bool handleSubscribe(const ACLMessage& msg) override {
           std::cout << "[Capteur] Nouvel abonné : "
                     << msg.getSender().name << std::endl;
           return true;  // accepter tous les abonnements
       }

       std::string handleCancel(const std::string& subscriber) override {
           std::cout << "[Capteur] " << subscriber
                     << " s'est désabonné." << std::endl;
           return "";  // pas de dernier message
       }

       void action() override {
           // Traiter les abonnements/désabonnements entrants
           SubscribeParticipant::action();

           // Publier une nouvelle valeur chaque seconde
           if (subscriberCount() > 0) {
               valeur_++;
               notify(std::to_string(valeur_));
           }

           sleep(1);
       }
   };

.. note::

   Dans cet exemple, ``action()`` appelle d'abord
   ``SubscribeParticipant::action()`` pour traiter les nouveaux
   abonnements, puis publie la notification. C'est le moyen de combiner
   la gestion des abonnements avec votre propre logique de publication.

Refuser un abonnement
----------------------

Retournez simplement ``false`` dans ``handleSubscribe()`` :

.. code-block:: cpp

   bool handleSubscribe(const ACLMessage& msg) override {
       if (/* condition */) {
           return false;  // l'abonné recevra un REFUSE
       }
       return true;
   }

Exemple complet
----------------

.. code-block:: cpp

   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/core/AgentCore.hpp>
   #include <gagent/protocols/SubscribeNotify.hpp>
   #include <iostream>

   using namespace gagent;
   using namespace gagent::protocols;

   // ── Le capteur : publie la température toutes les secondes ───────────────

   class CapteurBehaviour : public SubscribeParticipant {
       int temp_ = 20;
   public:
       CapteurBehaviour(Agent* ag) : SubscribeParticipant(ag, "capteur") {}

       bool handleSubscribe(const ACLMessage& msg) override {
           std::cout << "[Capteur] " << msg.getSender().name
                     << " s'est abonné." << std::endl;
           return true;
       }

       void action() override {
           SubscribeParticipant::action();  // gérer abonnements/désabonnements

           if (subscriberCount() > 0) {
               temp_++;
               notify(std::to_string(temp_));
           }
           sleep(1);
       }
   };

   class AgentCapteur : public Agent {
   public:
       void setup() override {
           addBehaviour(new CapteurBehaviour(this));
       }
   };

   // ── Le moniteur : s'abonne et affiche 5 notifications puis se désabonne ──

   class MoniteurBehaviour : public SubscribeInitiator {
       int count_ = 0;
   public:
       MoniteurBehaviour(Agent* ag)
           : SubscribeInitiator(ag, "moniteur", "capteur", "temperature") {}

       void handleNotify(const ACLMessage& msg) override {
           std::cout << "[Moniteur] Température : "
                     << msg.getContent() << "°C" << std::endl;
           count_++;
       }

       bool shouldCancel() override { return count_ >= 5; }
   };

   class AgentMoniteur : public Agent {
   public:
       void setup() override {
           addBehaviour(new MoniteurBehaviour(this));
       }
   };

   // ── main ─────────────────────────────────────────────────────────────────

   int main() {
       AgentCore::initAgentSystem();

       AgentCapteur  capteur;
       AgentMoniteur moniteur;

       capteur.init();
       moniteur.init();

       AgentCore::syncAgentSystem();  // attend que tous les agents terminent
       return 0;
   }

Résultat :

.. code-block:: text

   [Capteur] moniteur s'est abonné.
   [Moniteur] Température : 21°C
   [Moniteur] Température : 22°C
   [Moniteur] Température : 23°C
   [Moniteur] Température : 24°C
   [Moniteur] Température : 25°C

Résumé
-------

.. list-table::
   :widths: 35 65
   :header-rows: 1

   * - Élément
     - Rôle
   * - ``SubscribeInitiator``
     - S'abonne, reçoit les notifications, peut se désabonner
   * - ``SubscribeParticipant``
     - Gère les abonnements, publie les notifications
   * - ``handleNotify(msg)``
     - Côté abonné — appelée à chaque notification reçue
   * - ``shouldCancel()``
     - Côté abonné — retourner ``true`` pour se désabonner
   * - ``handleSubscribe(msg)``
     - Côté éditeur — accepter (``true``) ou refuser (``false``) un abonnement
   * - ``notify(contenu)``
     - Côté éditeur — envoyer une notification à tous les abonnés
   * - ``subscriberCount()``
     - Côté éditeur — nombre d'abonnés actifs
