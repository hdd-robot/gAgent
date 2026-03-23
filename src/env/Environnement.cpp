/*
 * Environnement.cpp
 */

#include <gagent/env/Environnement.hpp>
#include "../utils/udp_client_server.hpp"
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>


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

} // namespace gagent
