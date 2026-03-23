/*
 * Environnement.cpp
 */

#include <gagent/env/Environnement.hpp>
#include "../utils/udp_client_server.hpp"
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>


namespace gagent {

static bool parseKeyValSeq(const std::string& input, std::map<std::string, std::string>& m)
{
    m.clear();
    std::istringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ';')) {
        auto sep = token.find(':');
        if (sep == std::string::npos) continue;
        m[token.substr(0, sep)] = token.substr(sep + 1);
    }
    return !m.empty();
}

Environnement::Environnement()
{
    udpMonitor = new udp_client_server::udp_client("127.0.0.1", 40013);
}

Environnement::~Environnement()
{
    delete udpMonitor;
}

void Environnement::readDataFromQueueMsg()
{
    const std::string mq_name = "/envqueuemsg";
    const int taille  = 1000;
    const int max_msg = 5;

    struct mq_attr attr;
    attr.mq_flags   = 0;
    attr.mq_maxmsg  = max_msg;
    attr.mq_msgsize = taille;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(mq_name.c_str(), O_RDONLY | O_CREAT, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open ");
        std::cout << "Error create Message Queue" << std::endl;
        return;
    }

    std::vector<char> buffer(taille);
    std::map<std::string, std::string> m;

    while (true) {
        int ret = mq_receive(mq, buffer.data(), taille, nullptr);
        if (ret < 0) {
            perror("mq_receive ");
            continue;
        }

        std::string sbuffer(buffer.data(), ret);
        if (parseKeyValSeq(sbuffer, m)) {
            auto it_attr = m.find(this->id);
            if (it_attr != m.end()) {
                const std::string& agent_id = it_attr->second;
                std::lock_guard<std::mutex> lk(env_mutex_);
                list_attr[agent_id] = m;
            }
        } else {
            std::cout << "message non conforme : " << sbuffer << std::endl;
        }
    }
}

void Environnement::start()
{
    init_env();
    link_attribut();
    std::thread(&Environnement::readDataFromQueueMsg, this).detach();
    event_loop();   // bloquant — garde le processus enfant en vie
}

// ── Helpers internes ──────────────────────────────────────────────────────────

static std::string current_timestamp()
{
    auto now = std::chrono::system_clock::now();
    auto t   = std::chrono::system_clock::to_time_t(now);
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(
                   now.time_since_epoch()) % 1000;
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%Y-%m-%dT%H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

// ── NSAP — pile de snapshots ──────────────────────────────────────────────────

/**
 * Empile l'état courant de tous les agents.
 * @return numéro de séquence du snapshot créé (taille de la pile)
 */
int Environnement::push_nsap()
{
    std::lock_guard<std::mutex> lk(env_mutex_);
    int seq = nsap_seq_++;
    list_snaps[seq]   = list_attr;
    nsap_index_[seq]  = current_timestamp();
    std::cout << "[nsap] push #" << seq
              << " (" << list_attr.size() << " agents) @ "
              << nsap_index_[seq] << "\n";
    return seq + 1;   // taille de la pile
}

/**
 * Restaure le snapshot le plus récent (LIFO) et le retire de la pile.
 * @return taille restante de la pile, -1 si la pile était vide
 */
int Environnement::pull_nsap()
{
    std::lock_guard<std::mutex> lk(env_mutex_);
    if (list_snaps.empty()) {
        std::cerr << "[nsap] pull : pile vide\n";
        return -1;
    }
    // Le plus récent = clé maximale (séquence la plus élevée)
    auto it = std::prev(list_snaps.end());
    int  seq = it->first;
    list_attr = it->second;
    list_snaps.erase(it);
    nsap_index_.erase(seq);
    std::cout << "[nsap] pull #" << seq
              << " restauré (" << list_attr.size() << " agents)"
              << ", reste " << list_snaps.size() << " snap(s)\n";
    return static_cast<int>(list_snaps.size());
}

/**
 * Vide toute la pile de snapshots sans modifier l'état courant.
 */
void Environnement::clear_nsap()
{
    std::lock_guard<std::mutex> lk(env_mutex_);
    list_snaps.clear();
    nsap_index_.clear();
    nsap_seq_ = 0;
    std::cout << "[nsap] pile vidée\n";
}

/**
 * Retourne l'index de la pile : { numéro_séquence → timestamp }.
 * Le pointeur est valide jusqu'au prochain push/pull/clear.
 */
std::map<int, std::string>* Environnement::get_nsaps()
{
    return &nsap_index_;
}

void Environnement::make_agent()
{
    for (size_t i = 0; i < list_visual_agents.size(); i++)
        delete list_visual_agents[i];
    list_visual_agents.clear();

    std::lock_guard<std::mutex> lk(env_mutex_);
    for (auto& [agent_id, attrs] : list_attr) {
        auto* v = new VisualAgent();
        for (auto& [k, val_str] : attrs) {
            if      (k == id)     { v->id      = val_str; }
            else if (k == name)   { v->name    = val_str; }
            else if (k == pos_x)  { try { v->pos_x  = std::stof(val_str); } catch (...) {} }
            else if (k == pos_y)  { try { v->pos_y  = std::stof(val_str); } catch (...) {} }
            else if (k == val)    { v->val     = val_str; }
            else if (k == shape)  { v->shape   = val_str; }
            else if (k == color)  { v->color   = val_str; }
            else if (k == pattern){ v->pattern = val_str; }
            else if (k == size)   { try { v->size   = std::stof(val_str); } catch (...) {} }
            else if (k == size_x) { try { v->size_x = std::stof(val_str); } catch (...) {} }
            else if (k == size_y) { try { v->size_y = std::stof(val_str); } catch (...) {} }
            else if (k == size_z) { try { v->size_z = std::stof(val_str); } catch (...) {} }
            else                  { v->vals.push_back(val_str); }
        }
        if (v->id.empty()) { delete v; continue; }
        if (v->shape.empty()) v->shape = "circle";
        if (v->size  == 0)    v->size  = 5.0f;
        list_visual_agents.push_back(v);
    }
}

int Environnement::sendMsgMonitor(std::string msg)
{
    msg = "Environnement -> " + msg;
    udpMonitor->send(msg.c_str(), BUFLEN);
    return 0;
}

// ── Env socket server ─────────────────────────────────────────────────────────

static std::string env_readline_fd(int fd)
{
    std::string line;
    char c;
    while (::read(fd, &c, 1) == 1) {
        if (c == '\n') break;
        line += c;
    }
    return line;
}

static std::string env_json_escape(const std::string& s)
{
    std::string out;
    for (char c : s) {
        if      (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else                out += c;
    }
    return out;
}

void Environnement::serve(const std::string& socket_path)
{
    ::unlink(socket_path.c_str());

    int srv = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (srv < 0) { perror("env serve socket"); return; }

    struct sockaddr_un addr{};
    addr.sun_family = AF_UNIX;
    std::strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);

    if (::bind(srv, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("env serve bind"); ::close(srv); return;
    }
    ::listen(srv, 8);

    while (true) {
        int cli = ::accept(srv, nullptr, nullptr);
        if (cli < 0) continue;

        std::string cmd = env_readline_fd(cli);
        if (!cmd.empty() && cmd.back() == '\r') cmd.pop_back();

        std::string response;

        if (cmd == "GET_AGENTS") {
            std::ostringstream js;
            js << "{\"width\":" << map_width
               << ",\"height\":" << map_height
               << ",\"agents\":[";

            std::lock_guard<std::mutex> lk(env_mutex_);
            bool first = true;
            for (auto& [agent_id, attrs] : list_attr) {
                auto get = [&](const std::string& key) -> std::string {
                    auto it = attrs.find(key);
                    return it != attrs.end() ? it->second : "";
                };
                auto getf = [&](const std::string& key, float def) -> float {
                    auto it = attrs.find(key);
                    if (it == attrs.end()) return def;
                    try { return std::stof(it->second); } catch (...) { return def; }
                };

                if (!first) js << ",";
                first = false;
                js << "{"
                   << "\"id\":\""      << env_json_escape(get(id))      << "\","
                   << "\"name\":\""    << env_json_escape(get(name))    << "\","
                   << "\"shape\":\""   << env_json_escape(get(shape))   << "\","
                   << "\"color\":\""   << env_json_escape(get(color))   << "\","
                   << "\"pattern\":\"" << env_json_escape(get(pattern)) << "\","
                   << "\"x\":"         << getf(pos_x,  0.0f)            << ","
                   << "\"y\":"         << getf(pos_y,  0.0f)            << ","
                   << "\"size_x\":"    << getf(size_x, 1.0f)            << ","
                   << "\"size_y\":"    << getf(size_y, 1.0f)            << ","
                   << "\"size_z\":"    << getf(size_z, 1.0f)            << ","
                   << "\"size\":"      << getf(size,   1.0f)            << ","
                   << "\"val\":\""     << env_json_escape(get(val))     << "\""
                   << "}";
            }
            js << "]}";
            response = js.str();
        }
        else if (cmd == "GET_NSAP") {
            std::lock_guard<std::mutex> lk(env_mutex_);
            std::ostringstream js;
            js << "{\"count\":" << nsap_index_.size() << ",\"snaps\":[";
            bool first = true;
            for (auto& [seq, ts] : nsap_index_) {
                if (!first) js << ",";
                first = false;
                js << "{\"seq\":" << seq
                   << ",\"timestamp\":\"" << env_json_escape(ts) << "\"}";
            }
            js << "]}";
            response = js.str();
        }
        else {
            response = "{\"error\":\"unknown command\"}";
        }

        response += "\n";
        ::write(cli, response.c_str(), response.size());
        ::close(cli);
    }
}

} // namespace gagent
