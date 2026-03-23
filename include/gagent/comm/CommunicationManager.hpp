/**
 * @file CommunicationManager.hpp
 * @brief Manages different communication protocols (UDP, CORBA, Message Queues)
 * @author gAgent Project
 * @date 2025
 */

#ifndef GAGENT_COMMUNICATIONMANAGER_HPP_
#define GAGENT_COMMUNICATIONMANAGER_HPP_

#include <string>
#include <memory>
#include <map>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/utils/ErrorHandler.hpp>
#include <gagent/utils/Logger.hpp>

namespace gagent {

/**
 * @brief Communication protocol types
 */
enum class Protocol {
    UDP,       ///< UDP communication
    CORBA,     ///< CORBA/IIOP
    MQ,        ///< POSIX Message Queues
    TCP        ///< TCP/IP sockets
};

/**
 * @brief Abstract base for communication channels
 */
class CommunicationChannel {
public:
    virtual ~CommunicationChannel() = default;

    virtual bool send(const ACLMessage& message, const std::string& destination) = 0;
    virtual bool receive(ACLMessage& message) = 0;
    virtual bool isActive() const = 0;
    virtual void close() = 0;
};

/**
 * @brief UDP communication channel
 */
class UDPChannel : public CommunicationChannel {
public:
    UDPChannel(const std::string& address, int port);
    ~UDPChannel() override;

    bool send(const ACLMessage& message, const std::string& destination) override;
    bool receive(ACLMessage& message) override;
    bool isActive() const override;
    void close() override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief CORBA communication channel
 */
class CORBAChannel : public CommunicationChannel {
public:
    CORBAChannel(const std::string& nameserver_ior);
    ~CORBAChannel() override;

    bool send(const ACLMessage& message, const std::string& destination) override;
    bool receive(ACLMessage& message) override;
    bool isActive() const override;
    void close() override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Message Queue communication channel
 */
class MQChannel : public CommunicationChannel {
public:
    explicit MQChannel(const std::string& queue_name);
    ~MQChannel() override;

    bool send(const ACLMessage& message, const std::string& destination) override;
    bool receive(ACLMessage& message) override;
    bool isActive() const override;
    void close() override;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

/**
 * @brief Manages multiple communication channels
 */
class CommunicationManager {
public:
    CommunicationManager();
    ~CommunicationManager();

    void registerChannel(Protocol protocol, std::unique_ptr<CommunicationChannel> channel);
    bool send(Protocol protocol, const ACLMessage& message, const std::string& destination);
    bool receive(ACLMessage& message, Protocol& protocol);
    CommunicationChannel* getChannel(Protocol protocol);
    void closeAll();

    CommunicationManager(const CommunicationManager&) = delete;
    CommunicationManager& operator=(const CommunicationManager&) = delete;

private:
    std::map<Protocol, std::unique_ptr<CommunicationChannel>> channels_;
};

} // namespace gagent

#endif /* GAGENT_COMMUNICATIONMANAGER_HPP_ */
