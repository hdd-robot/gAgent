/*
 * Args.hpp — Parsing des arguments CLI et configuration réseau
 */

#ifndef GAGENT_PLATFORM_ARGS_HPP_
#define GAGENT_PLATFORM_ARGS_HPP_

#include <string>
#include <iostream>

class Args {
public:
    Args();

    static int getAppName(int argc, char** argv);
    static int argsUsageAgentManager (int argc, char** argv, Args*);
    static int argsUsageAgentPlatform(int argc, char** argv, Args*);
    static int argsUsageAgentMonitor (int argc, char** argv, Args*);

    const std::string& getIpAdrMng()     const { return ipAdrMng_; }
    const std::string& getPortMng()      const { return portMng_;  }
    const std::string& getIpAdrPlt()     const { return ipAdrPlt_; }
    const std::string& getPortPlt()      const { return portPlt_;  }
    const std::string& getIpAdrMon()     const { return ipAdrMon_; }
    const std::string& getPortMon()      const { return portMon_;  }

    // Multi-machine
    bool               isMaster()        const { return master_; }
    bool               isSlave()         const { return !slave_addr_.empty(); }
    const std::string& slaveAddr()       const { return slave_addr_; }  // "ip:port"
    const std::string& localIP()         const { return local_ip_; }    // --ip override
    int                controlPort()     const { return control_port_; }
    int                basePort()        const { return base_port_; }

    void setIpAdrMng(const std::string& v) { ipAdrMng_ = v; }
    void setPortMng (const std::string& v) { portMng_  = v; }
    void setIpAdrPlt(const std::string& v) { ipAdrPlt_ = v; }
    void setPortPlt (const std::string& v) { portPlt_  = v; }
    void setIpAdrMon(const std::string& v) { ipAdrMon_ = v; }
    void setPortMon (const std::string& v) { portMon_  = v; }

    static constexpr int AGENT_MANAGER  = 10;
    static constexpr int AGENT_PLATFORM = 20;
    static constexpr int AGENT_MONITOR  = 30;

private:
    std::string ipAdrPlt_ = "127.0.0.1";
    std::string portPlt_  = "40011";
    std::string ipAdrMng_ = "127.0.0.1";
    std::string portMng_  = "40012";
    std::string ipAdrMon_ = "127.0.0.1";
    std::string portMon_  = "40013";

    bool        master_       = false;
    std::string slave_addr_;      // ex: "192.168.1.10:40011"
    std::string local_ip_;        // override via --ip
    int         control_port_ = 40015;
    int         base_port_    = 50000;
};

#endif /* GAGENT_PLATFORM_ARGS_HPP_ */
