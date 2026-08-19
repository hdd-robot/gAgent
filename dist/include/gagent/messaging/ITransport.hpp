/*
 * ITransport.hpp — Interface abstraite de transport pour la messagerie FIPA ACL
 *
 * Permet de brancher n'importe quel mécanisme de transport (ZeroMQ, Email, HTTP…)
 * sans que les protocoles FIPA (ContractNet, Request, Subscribe) sachent comment
 * les messages voyagent physiquement.
 *
 * Implémentations fournies :
 *   ZmqTransport  — transport IPC/TCP ZeroMQ (défaut)
 *
 * Usage (injection dans un agent) :
 *   agent.setTransport(std::make_shared<MyEmailTransport>(config));
 */

#ifndef GAGENT_ITRANSPORT_HPP_
#define GAGENT_ITRANSPORT_HPP_

#include <gagent/messaging/ACLMessage.hpp>
#include <optional>
#include <string>
#include <memory>

namespace gagent {
namespace messaging {

class ITransport {
public:
    virtual ~ITransport() = default;

    /**
     * Prépare la réception pour l'agent <name>.
     * @return endpoint public (ex: "ipc:///tmp/acl_alice" ou "tcp://10.0.0.1:5555")
     */
    virtual std::string bind(const std::string& name) = 0;

    /**
     * Envoie un message ACL à l'agent <to>.
     * @return true si l'envoi a réussi
     */
    virtual bool send(const std::string& to, const ACLMessage& msg) = 0;

    /**
     * Attend un message pour l'agent <name> pendant au plus timeout_ms ms.
     * @return le message reçu, ou nullopt si timeout
     */
    virtual std::optional<ACLMessage> receive(const std::string& name,
                                               int timeout_ms = 5000) = 0;

    /**
     * Libère la queue de <name> et désenregistre l'endpoint.
     * À appeler dans takeDown() de l'agent.
     */
    virtual void unlink(const std::string& name) = 0;

    /**
     * Vide les queues d'envoi en attente (flush linger).
     */
    virtual void flush() = 0;
};

} // namespace messaging
} // namespace gagent

#endif /* GAGENT_ITRANSPORT_HPP_ */
