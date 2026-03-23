/*
 * llm_agent.cpp — Démonstration du PythonBehaviour avec un LLM
 *
 * Deux agents :
 *   LLMAgent    — délègue sa logique à llm_agent.py (OpenAI ou echo)
 *   TesterAgent — envoie des questions et affiche les réponses
 *
 * Lancement :
 *   export OPENAI_API_KEY=sk-...          # optionnel
 *   ./llm_agent
 */

#include <gagent/core/AgentCore.hpp>
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/env/Environnement.hpp>
#include <gagent/messaging/AclMQ.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/python/PythonBehaviour.hpp>

#include <iostream>
#include <thread>
#include <chrono>
#include <filesystem>

using namespace gagent;
using namespace gagent::messaging;
using namespace gagent::python;

// ── Environnement minimal ─────────────────────────────────────────────────────

class DummyEnv : public Environnement {
public:
    void init_env()      override {}
    void link_attribut() override {}
    void event_loop()    override {}
};

// ── Comportement du testeur ───────────────────────────────────────────────────

class TesterBehaviour : public Behaviour {
    std::vector<std::string> questions_ = {
        "Quel est le rôle d'un agent AMS dans FIPA ?",
        "Explique le protocole Contract Net en une phrase.",
        "Quelle est la différence entre un agent et un thread ?",
    };
    int  idx_    = 0;
    bool done_   = false;
    bool waiting_ = false;

public:
    explicit TesterBehaviour(Agent* ag) : Behaviour(ag) {}

    void action() override {
        if (waiting_) {
            // Attend la réponse du LLM
            auto opt = acl_receive("tester", 6000);
            if (opt && opt->getPerformative() == ACLMessage::Performative::INFORM) {
                std::cout << "\n[Tester] réponse reçue :\n"
                          << "  " << opt->getContent() << "\n\n";
            } else {
                std::cout << "[Tester] timeout ou réponse inattendue\n";
            }
            waiting_ = false;
            ++idx_;
        }

        if (idx_ >= (int)questions_.size()) {
            // Toutes les questions posées → demander l'arrêt du LLM agent
            ACLMessage cancel(ACLMessage::Performative::CANCEL);
            cancel.setSender(AgentIdentifier{"tester"});
            acl_send("llm-agent", cancel);
            done_ = true;
            return;
        }

        // Pause entre questions
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        ACLMessage req(ACLMessage::Performative::REQUEST);
        req.setSender(AgentIdentifier{"tester"});
        req.setContent(questions_[idx_]);
        req.setConversationId("q-" + std::to_string(idx_));
        req.setReplyWith("reply-" + std::to_string(idx_));

        std::cout << "[Tester] question " << idx_ + 1 << "/" << questions_.size()
                  << " : " << questions_[idx_] << "\n";

        acl_send("llm-agent", req);
        waiting_ = true;
    }

    bool done() override { return done_; }

    void onEnd() override {
        this_agent->doDelete();
    }
};

// ── Agents ────────────────────────────────────────────────────────────────────

class LLMAgentC : public Agent {
    std::string script_path_;
public:
    explicit LLMAgentC(const std::string& script)
        : script_path_(script) {}

    void setup() override {
        std::cout << "[LLMAgent] démarrage → " << script_path_ << "\n";
        addBehaviour(new PythonBehaviour(this, "llm-agent", script_path_, 300));
    }
    void takeDown() override { acl_unlink("llm-agent"); }
};

class TesterAgentC : public Agent {
public:
    void setup() override {
        std::cout << "[Tester] démarrage\n";
        addBehaviour(new TesterBehaviour(this));
    }
    void takeDown() override { acl_unlink("tester"); }
};

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    // Chemin vers llm_agent.py (même répertoire que ce binaire, ou argument)
    std::string script;
    if (argc >= 2) {
        script = argv[1];
    } else {
        // Cherche dans le répertoire du binaire ou dans examples/
        namespace fs = std::filesystem;
        fs::path bin_dir = fs::path(argv[0]).parent_path();
        std::vector<fs::path> candidates = {
            bin_dir / "llm_agent.py",
            bin_dir / "../examples/llm_agent.py",
            "examples/llm_agent.py",
            "llm_agent.py",
        };
        for (auto& p : candidates)
            if (fs::exists(p)) { script = p.string(); break; }
    }

    if (script.empty()) {
        std::cerr << "Erreur : llm_agent.py introuvable.\n"
                  << "Usage : " << argv[0] << " [chemin/vers/llm_agent.py]\n";
        return 1;
    }

    std::cout << "======================================================\n"
              << " gAgent — PythonBehaviour + LLM\n"
              << "======================================================\n\n";

    // Nettoyage
    acl_unlink("llm-agent");
    acl_unlink("tester");

    AgentCore::initAgentSystem();

    DummyEnv env;
    AgentCore::initEnvironnementSystem(env);

    LLMAgentC  llm_agent(script);
    TesterAgentC tester;

    llm_agent.init();
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    tester.init();

    AgentCore::syncAgentSystem();
    AgentCore::syncAgentSystem();

    std::cout << "\n=== Terminé ===\n";
    return 0;
}
