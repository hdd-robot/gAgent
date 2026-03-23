/*
 * Environnement.cpp
 */

#include <gagent/env/Environnement.hpp>
#include "../utils/udp_client_server.hpp"
#include <sstream>

#ifdef BUILD_GUI
#include "../gui/EnvironnementGui.hpp"
#endif

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
                auto it_list = list_attr.find(agent_id);
                if (it_list != list_attr.end())
                    it_list->second = m;
                else
                    list_attr.insert({agent_id, m});
            }
        } else {
            std::cout << "message non conforme : " << sbuffer << std::endl;
        }
    }
}

void Environnement::start(bool gui, unsigned int timer_val)
{
    init_env();
    link_attribut();

#ifdef BUILD_GUI
    auto* env_gui = new EnvironnementGui();
    env_gui->setEnvPtr(this);
    int   argc   = 1;
    char* argv[] = {const_cast<char*>("")};
    env_gui->createWindow(argc, argv, gui, timer_val);
#else
    (void)gui;
    (void)timer_val;
    std::thread(&Environnement::readDataFromQueueMsg, this).detach();
#endif
}

void Environnement::clear_nsap() {}

int Environnement::push_nsap() { return 0; }

int Environnement::pull_nsap() { return 0; }

std::map<int, std::string>* Environnement::get_nsaps()
{
    return nullptr;
}

void Environnement::make_agent()
{
    for (size_t i = 0; i < list_visual_agents.size(); i++)
        delete list_visual_agents[i];
    list_visual_agents.clear();

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
