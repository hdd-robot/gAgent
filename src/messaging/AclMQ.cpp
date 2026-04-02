/*
 * AclMQ.cpp — Définitions des singletons de transport ZeroMQ
 *
 * Ces symboles sont volontairement définis ici (dans libgagent.so) et non
 * dans le header, afin qu'il n'existe qu'une seule instance de PullCache,
 * PushCache et zmq_ctx() par processus, même quand le header est inclus
 * à la fois par libgagent.so et par le binaire appelant.
 */

#include <gagent/messaging/AclMQ.hpp>
#include <unistd.h>

namespace gagent {
namespace messaging {

void* zmq_ctx()
{
    static void* ctx     = nullptr;
    static pid_t ctx_pid = 0;

    pid_t cur = ::getpid();
    if (ctx_pid != cur) {
        if (ctx) zmq_ctx_destroy(ctx);
        ctx     = zmq_ctx_new();
        ctx_pid = cur;
    }
    return ctx;
}

PullCache& PullCache::instance()
{
    static PullCache inst;
    return inst;
}

PushCache& PushCache::instance()
{
    static PushCache inst;
    return inst;
}

} // namespace messaging
} // namespace gagent
