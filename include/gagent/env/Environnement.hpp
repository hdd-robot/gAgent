/*
 * Environnement.hpp
 */

#ifndef GAGENT_ENVIRONNEMENT_HPP_
#define GAGENT_ENVIRONNEMENT_HPP_

#include <map>
#include <string>
#include <iterator>
#include <iostream>
#include <vector>
#include <mutex>
#include <mqueue.h>
#include <thread>
#include <gagent/env/VisualAgent.hpp>

namespace udp_client_server {
    class udp_client;
}

#define BUFLEN 1024

namespace gagent {

class Environnement {
public:
    Environnement();
    virtual ~Environnement();

    virtual void start() final;

    virtual void clear_nsap() final;
    virtual int  push_nsap()  final;
    virtual int  pull_nsap()  final;
    virtual std::map<int, std::string>* get_nsaps() final;

    virtual void init_env()      {}
    virtual void link_attribut() {}
    virtual void event_loop()    {}

    virtual void link_id(std::string v)     final { id     = v; }
    virtual void link_name(std::string v)   final { name   = v; }
    virtual void link_pos_x(std::string v)  final { pos_x  = v; }
    virtual void link_pos_y(std::string v)  final { pos_y  = v; }
    virtual void link_color(std::string v)  final { color  = v; }
    virtual void link_shape(std::string v)  final { shape  = v; }
    virtual void link_size(std::string v)   final { size   = v; }
    virtual void link_size_x(std::string v) final { size_x = v; }
    virtual void link_size_y(std::string v) final { size_y = v; }
    virtual void link_size_z(std::string v) final { size_z = v; }
    virtual void link_val(std::string v)    final { val    = v; }

    virtual void make_agent()        final;
    virtual void readDataFromQueueMsg();

    int sendMsgMonitor(std::string msg);

    std::string id      = "id";
    std::string name    = "name";
    std::string shape   = "shape";
    std::string size    = "size";
    std::string size_x  = "size_x";
    std::string size_y  = "size_y";
    std::string size_z  = "size_z";
    std::string color   = "color";
    std::string pos_x   = "pos_x";
    std::string pos_y   = "pos_y";
    std::string val     = "val";
    std::string pattern = "pattern";
    std::vector<std::string> vals;

    std::vector<VisualAgent*> list_visual_agents;

    int map_width  = 600;
    int map_height = 300;

private:
    std::map<std::string, std::map<std::string, std::string>> list_attr;
    std::map<int, std::map<std::string, std::map<std::string, std::string>>> list_snaps;
    std::map<int, std::string> nsap_index_;
    int nsap_seq_ = 0;
    std::mutex env_mutex_;
    udp_client_server::udp_client* udpMonitor = nullptr;
};

} // namespace gagent

#endif /* GAGENT_ENVIRONNEMENT_HPP_ */
