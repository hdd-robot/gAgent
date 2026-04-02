/*
 * ZmqTransport.hpp — Implémentation ZeroMQ de ITransport
 *
 * Délègue simplement aux fonctions acl_bind / acl_send / acl_receive /
 * acl_unlink / acl_flush définies dans AclMQ.hpp.
 *
 * C'est le transport par défaut instancié par Agent.
 */

#ifndef GAGENT_ZMQTRANSPORT_HPP_
#define GAGENT_ZMQTRANSPORT_HPP_

#include <gagent/messaging/ITransport.hpp>
#include <gagent/messaging/AclMQ.hpp>

namespace gagent {
namespace messaging {

class ZmqTransport : public ITransport {
public:
    std::string bind(const std::string& name) override {
        return acl_bind(name);
    }

    bool send(const std::string& to, const ACLMessage& msg) override {
        return acl_send(to, msg);
    }

    std::optional<ACLMessage> receive(const std::string& name,
                                      int timeout_ms = 5000) override {
        return acl_receive(name, timeout_ms);
    }

    void unlink(const std::string& name) override {
        acl_unlink(name);
    }

    void flush() override {
        acl_flush();
    }
};

} // namespace messaging
} // namespace gagent

#endif /* GAGENT_ZMQTRANSPORT_HPP_ */
