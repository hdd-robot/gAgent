/*
 * AgentFactory.hpp — Fabrique d'agents et serveur de migration FIPA
 *
 * Permet la migration d'agents entre nœuds d'un cluster gAgent.
 *
 * Côté destination — enregistrer les types au démarrage :
 *   AgentFactory::instance().registerType("MonAgent",
 *       []() -> Agent* { return new MonAgent(); });
 *   // Puis démarrer le serveur de migration dans un thread :
 *   std::thread([]{ AgentFactory::instance().startMigrationServer(); }).detach();
 *
 * Côté agent migrant :
 *   virtual std::string agentTypeName() const override { return "MonAgent"; }
 *   // dans un behaviour :
 *   doMove(MigrationTarget{"192.168.1.10"});
 */

#ifndef GAGENT_AGENTFACTORY_HPP_
#define GAGENT_AGENTFACTORY_HPP_

#include <functional>
#include <map>
#include <string>

namespace gagent {

class Agent;

class AgentFactory {
public:
    using Creator = std::function<Agent*()>;

    static AgentFactory& instance();

    // Enregistre un constructeur pour un type d'agent donné.
    // À appeler dans main() avant startMigrationServer().
    void registerType(const std::string& type_name, Creator creator);

    // Crée une instance du type donné, ou nullptr si non enregistré.
    Agent* create(const std::string& type_name) const;

    // Démarre le serveur TCP de migration (boucle bloquante).
    // Appeler dans un std::thread séparé.
    // port == -1  →  PlatformConfig::instance().migrationPort()
    void startMigrationServer(int port = -1);

private:
    AgentFactory()  = default;
    std::map<std::string, Creator> registry_;
};

} // namespace gagent

#endif /* GAGENT_AGENTFACTORY_HPP_ */
