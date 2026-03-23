/*
 * AclMQ.hpp — Utilitaires POSIX MQ pour la messagerie FIPA ACL
 *
 * Chaque agent dispose d'une queue dédiée : /acl_<nom>
 * Les messages sont sérialisés au format FIPA ACL (s-expressions).
 *
 * Usage :
 *   #include <gagent/messaging/AclMQ.hpp>
 *   using namespace gagent::messaging;
 *
 *   acl_send("bob", msg);
 *   auto opt = acl_receive("alice", 5000);
 */

#ifndef GAGENT_ACLMQ_HPP_
#define GAGENT_ACLMQ_HPP_

#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/utils/Logger.hpp>
#include <mqueue.h>
#include <optional>
#include <string>
#include <vector>
#include <cerrno>
#include <ctime>

namespace gagent {
namespace messaging {

static constexpr int ACL_MQ_MSGSIZE = 1024;
static constexpr int ACL_MQ_MAXMSG  = 10;

inline std::string acl_qname(const std::string& agent_name)
{
    return "/acl_" + agent_name;
}

/**
 * Envoie un ACLMessage dans la queue /acl_<to>.
 * Crée la queue si elle n'existe pas encore.
 * @return true si envoi réussi
 */
inline bool acl_send(const std::string& to, const ACLMessage& msg)
{
    std::string raw = msg.toString();
    if ((int)raw.size() >= ACL_MQ_MSGSIZE) return false;

    struct mq_attr attr{};
    attr.mq_maxmsg  = ACL_MQ_MAXMSG;
    attr.mq_msgsize = ACL_MQ_MSGSIZE;

    mqd_t mq = mq_open(acl_qname(to).c_str(), O_WRONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) return false;

    int r = mq_send(mq, raw.c_str(), raw.size(), 0);
    mq_close(mq);
    if (r == 0) {
        std::string recv = msg.getReceivers().empty() ? to
                         : msg.getReceivers()[0].name;
        LOG_JSON("acl_send",
            {"from",  msg.getSender().name},
            {"to",    recv},
            {"perf",  ACLMessage::performativeToString(msg.getPerformative())},
            {"conv",  msg.getConversationId()},
            {"content", msg.getContent().substr(0, 120)}
        );
    }
    return r == 0;
}

/**
 * Reçoit un ACLMessage depuis la queue /acl_<my_name>.
 * Bloquant jusqu'à timeout_ms.
 * @return nullopt si timeout ou erreur
 */
inline std::optional<ACLMessage> acl_receive(const std::string& my_name,
                                              int timeout_ms = 5000)
{
    struct mq_attr attr{};
    attr.mq_maxmsg  = ACL_MQ_MAXMSG;
    attr.mq_msgsize = ACL_MQ_MSGSIZE;

    mqd_t mq = mq_open(acl_qname(my_name).c_str(), O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) return std::nullopt;

    struct timespec ts{};
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += timeout_ms / 1000;
    ts.tv_nsec += static_cast<long>(timeout_ms % 1000) * 1000000LL;
    if (ts.tv_nsec >= 1000000000LL) { ++ts.tv_sec; ts.tv_nsec -= 1000000000LL; }

    std::vector<char> buf(ACL_MQ_MSGSIZE);
    ssize_t n = mq_timedreceive(mq, buf.data(), ACL_MQ_MSGSIZE, nullptr, &ts);
    mq_close(mq);

    if (n <= 0) return std::nullopt;
    auto result = ACLMessage::parse(std::string(buf.data(), static_cast<size_t>(n)));
    if (result) {
        LOG_JSON("acl_recv",
            {"to",    my_name},
            {"from",  result->getSender().name},
            {"perf",  ACLMessage::performativeToString(result->getPerformative())},
            {"conv",  result->getConversationId()},
            {"content", result->getContent().substr(0, 120)}
        );
    }
    return result;
}

/**
 * Supprime la queue /acl_<name> du système.
 * À appeler dans takeDown() de l'agent.
 */
inline void acl_unlink(const std::string& name)
{
    mq_unlink(acl_qname(name).c_str());
}

} // namespace messaging
} // namespace gagent

#endif /* GAGENT_ACLMQ_HPP_ */
