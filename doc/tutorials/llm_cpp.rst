17 — Intégrer un agent LLM en C++
===================================

Côté C++, un agent LLM utilise ``PythonBehaviour`` à la place d'un
behaviour classique. Ce behaviour lance votre script Python en
sous-processus et orchestre la communication.

PythonBehaviour
----------------

.. code-block:: cpp

   #include <gagent/python/PythonBehaviour.hpp>
   using namespace gagent::python;

Constructeur :

.. code-block:: cpp

   PythonBehaviour(
       Agent*             ag,
       const std::string& my_name,       // nom ACL de cet agent
       const std::string& script_path,   // chemin vers le .py
       const std::string& system_prompt, // personnalité du LLM
       const std::string& model,         // "gpt-4o-mini", "llama3"…
       int                max_tokens,    // longueur max de réponse
       int                max_history,   // tours de conv mémorisés
       int                tick_ms        // intervalle de tick (ms)
   );

.. list-table::
   :widths: 25 75
   :header-rows: 1

   * - Paramètre
     - Description
   * - ``my_name``
     - Nom de la file ACL (``ipc:///tmp/acl_<my_name>``)
   * - ``script_path``
     - Chemin absolu ou relatif vers le script Python
   * - ``system_prompt``
     - Définit le rôle / la personnalité de l'agent pour le LLM
   * - ``model``
     - Transmis au script Python — utilisé pour choisir le modèle LLM
   * - ``max_tokens``
     - Limite la longueur des réponses générées
   * - ``max_history``
     - Nombre de tours de conversation conservés en mémoire
   * - ``tick_ms``
     - Fréquence à laquelle ``on_tick()`` est appelé en Python

Exemple complet
----------------

Deux agents : ``LLMAgent`` répond aux questions via un LLM,
``TesterAgent`` lui envoie trois questions et affiche les réponses.

.. code-block:: cpp

   #include <gagent/core/AgentCore.hpp>
   #include <gagent/core/Agent.hpp>
   #include <gagent/core/Behaviour.hpp>
   #include <gagent/messaging/AclMQ.hpp>
   #include <gagent/messaging/ACLMessage.hpp>
   #include <gagent/python/PythonBehaviour.hpp>
   #include <iostream>
   #include <thread>
   #include <chrono>

   using namespace gagent;
   using namespace gagent::messaging;
   using namespace gagent::python;

   // ── L'agent LLM ──────────────────────────────────────────────────────────────

   class LLMAgent : public Agent {
       std::string script_;
   public:
       explicit LLMAgent(const std::string& script) : script_(script) {}

       void setup() override {
           addBehaviour(new PythonBehaviour(
               this,
               "llm-agent",          // nom ACL
               script_,              // chemin vers le .py
               "Tu es un expert en systèmes multi-agents FIPA. "
               "Réponds en français, de façon concise.",
               "gpt-4o-mini",        // modèle
               200,                  // max_tokens
               10                    // max_history
           ));
       }

       void takeDown() override {
           acl_unlink("llm-agent");
       }
   };

   // ── L'agent testeur ───────────────────────────────────────────────────────────

   class TesterBehaviour : public Behaviour {
       std::vector<std::string> questions_ = {
           "Quel est le rôle d'un agent AMS dans FIPA ?",
           "Explique le protocole Contract Net en une phrase.",
           "Quelle est la différence entre un agent et un thread ?",
       };
       int  idx_     = 0;
       bool waiting_ = false;

   public:
       explicit TesterBehaviour(Agent* ag) : Behaviour(ag) {}

       void action() override {
           if (waiting_) {
               // Attendre la réponse (timeout 6 s)
               auto rep = acl_receive("tester", 6000);
               if (rep && rep->getPerformative() == ACLMessage::Performative::INFORM) {
                   std::cout << "[Tester] réponse : "
                             << rep->getContent() << std::endl;
               } else {
                   std::cout << "[Tester] timeout ou erreur" << std::endl;
               }
               waiting_ = false;
               idx_++;
           }

           if (idx_ >= (int)questions_.size()) {
               // Toutes les questions posées → arrêter le LLMAgent
               ACLMessage cancel(ACLMessage::Performative::CANCEL);
               cancel.setSender(AgentIdentifier{"tester"});
               acl_send("llm-agent", cancel);
               done_ = true;
               return;
           }

           std::this_thread::sleep_for(std::chrono::milliseconds(500));

           ACLMessage req(ACLMessage::Performative::REQUEST);
           req.setSender(AgentIdentifier{"tester"});
           req.setContent(questions_[idx_]);
           req.setConversationId("q-" + std::to_string(idx_));

           std::cout << "[Tester] question " << idx_ + 1
                     << " : " << questions_[idx_] << std::endl;

           acl_send("llm-agent", req);
           waiting_ = true;
       }

       void onEnd() override { this_agent->doDelete(); }
   };

   class TesterAgent : public Agent {
   public:
       void setup() override {
           addBehaviour(new TesterBehaviour(this));
       }
       void takeDown() override { acl_unlink("tester"); }
   };

   // ── main ─────────────────────────────────────────────────────────────────────

   int main(int argc, char* argv[]) {
       std::string script = (argc >= 2) ? argv[1] : "llm_agent.py";

       AgentCore::initAgentSystem();

       LLMAgent    llm(script);
       TesterAgent tester;

       llm.init();
       // Laisser le temps au script Python de démarrer
       std::this_thread::sleep_for(std::chrono::milliseconds(300));
       tester.init();

       AgentCore::syncAgentSystem();
       return 0;
   }

Lancer l'exemple
-----------------

.. code-block:: bash

   # Avec OpenAI
   export OPENAI_API_KEY=sk-...
   ./build/examples/llm_agent examples/llm_agent.py

   # Sans clé (mode echo — fonctionne sans LLM)
   ./build/examples/llm_agent examples/llm_agent.py

   # Avec Ollama (local)
   ollama run llama3
   ./build/examples/llm_agent examples/mon_agent_ollama.py

Résultat attendu (avec LLM) :

.. code-block:: text

   [Tester] question 1 : Quel est le rôle d'un agent AMS dans FIPA ?
   [Tester] réponse : L'AMS (Agent Management System) est l'annuaire
   central de la plateforme FIPA. Il gère le cycle de vie des agents :
   création, enregistrement, suspension, suppression.

   [Tester] question 2 : Explique le protocole Contract Net en une phrase.
   [Tester] réponse : Le Contract Net est un protocole de négociation
   où un coordinateur lance un appel d'offres (CFP), les candidats
   proposent ou refusent, et le coordinateur sélectionne le meilleur.

Robustesse de PythonBehaviour
-------------------------------

``PythonBehaviour`` gère les pannes du processus Python de façon transparente :

- Si Python meurt pendant une écriture → ``EPIPE`` détecté, behaviour terminé proprement
- Si Python ferme son ``stdout`` → ``read()`` retourne 0, behaviour terminé
- Si Python ne répond plus → le prochain tick vérifie que le processus est vivant via ``waitpid``
- Si le LLM lève une exception → le script Python l'attrape et retourne une action d'erreur,
  le processus C++ n'est pas impacté

Plusieurs agents LLM simultanés
---------------------------------

Vous pouvez faire tourner plusieurs agents LLM dans le même SMA, chacun
avec sa propre personnalité et son propre modèle :

.. code-block:: cpp

   LLMAgent expert  ("agents/expert.py",   "gpt-4o",      "Tu es un expert FIPA.");
   LLMAgent moderateur("agents/modo.py",   "gpt-4o-mini", "Tu es un modérateur.");
   LLMAgent local   ("agents/local.py",    "llama3",      "Tu es un assistant local.");

   expert.init();
   moderateur.init();
   local.init();

   AgentCore::syncAgentSystem();

Chaque script Python est un processus indépendant avec son propre
historique de conversation. Les agents communiquent entre eux via des
messages ACL comme n'importe quel autre agent gAgent.
