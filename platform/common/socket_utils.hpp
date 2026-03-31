/*
 * socket_utils.hpp — utilitaires bas niveau pour les sockets AMS/DF
 */

#ifndef GAGENT_PLATFORM_SOCKET_UTILS_HPP_
#define GAGENT_PLATFORM_SOCKET_UTILS_HPP_

#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

namespace gagent {
namespace platform {

/* Lit une ligne jusqu'à '\n' en minimisant les appels système.
 * Utilise MSG_PEEK pour repérer le '\n' puis un seul read(). */
inline std::string readline_fd(int fd)
{
    std::string line;
    char buf[4096];

    while (true) {
        ssize_t n = ::recv(fd, buf, sizeof(buf), MSG_PEEK);
        if (n <= 0) break;

        char* nl = static_cast<char*>(memchr(buf, '\n', static_cast<size_t>(n)));
        size_t to_read = nl ? static_cast<size_t>(nl - buf + 1)
                            : static_cast<size_t>(n);

        ssize_t r = ::read(fd, buf, to_read);
        if (r <= 0) break;

        if (nl) {
            line.append(buf, static_cast<size_t>(r) - 1);  // sans '\n'
            break;
        }
        line.append(buf, static_cast<size_t>(r));
    }
    return line;
}

/* Écrit une chaîne sur le fd. */
inline void write_str(int fd, const std::string& s)
{
    ::write(fd, s.c_str(), s.size());
}

} // namespace platform
} // namespace gagent

#endif
