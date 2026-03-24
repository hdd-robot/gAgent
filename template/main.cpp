#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>
#include <gagent/core/AgentCore.hpp>
#include <iostream>
#include <unistd.h>

using namespace gagent;

// Un behaviour est une tâche répétée par l'agent.
// action() est appelé en boucle tant que done() retourne false.
class HelloBehaviour : public Behaviour {
    int count_ = 0;
public:
    HelloBehaviour(Agent* ag) : Behaviour(ag) {}

    void action() override {
        std::cout << "[HelloAgent] Hello ! (" << ++count_ << "/5)" << std::endl;
        sleep(1);
    }

    bool done() override { return count_ >= 5; }
};

// L'agent déclare ses behaviours dans setup().
class HelloAgent : public Agent {
public:
    void setup() override {
        addBehaviour(new HelloBehaviour(this));
    }
};

int main() {
    // Initialise le système (gestionnaires de signaux, etc.)
    AgentCore::initAgentSystem();

    // Crée et lance l'agent dans un processus enfant (fork)
    HelloAgent agent;
    agent.init();

    // Attend que l'agent ait terminé
    AgentCore::syncAgentSystem();
    return 0;
}
