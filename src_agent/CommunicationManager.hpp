/**
 * @file CommunicationManager.hpp
 * @brief Manages different communication protocols (UDP, CORBA, Message Queues)
 * @author gAgent Project
 * @date 2025
 */

#ifndef COMMUNICATIONMANAGER_HPP_
#define COMMUNICATIONMANAGER_HPP_

#include <string>
#include <memory>
#include <map>
#include "ACLMessage.hpp"
#include "ErrorHandler.hpp"
#include "Logger.hpp"

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
    
    /**
     * @brief Send a message
     * @param message ACL message to send
     * @param destination Destination address
     * @return true if sent successfully
     */
    virtual bool send(const ACLMessage& message, const std::string& destination) = 0;
    
    /**
     * @brief Receive a message (non-blocking)
     * @param message Output parameter for received message
     * @return true if message received
     */
    virtual bool receive(ACLMessage& message) = 0;
    
    /**
     * @brief Check if channel is active
     */
    virtual bool isActive() const = 0;
    
    /**
     * @brief Close the channel
     */
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
    
    /**
     * @brief Register a communication channel
     * @param protocol Protocol type
     * @param channel Channel instance
     */
    void registerChannel(Protocol protocol, std::unique_ptr<CommunicationChannel> channel);
    
    /**
     * @brief Send message using specified protocol
     * @param protocol Protocol to use
     * @param message Message to send
     * @param destination Destination address
     * @return true if sent successfully
     */
    bool send(Protocol protocol, const ACLMessage& message, const std::string& destination);
    
    /**
     * @brief Receive message from any protocol
     * @param message Output parameter
     * @param protocol Output - which protocol received from
     * @return true if message received
     */
    bool receive(ACLMessage& message, Protocol& protocol);
    
    /**
     * @brief Get channel for protocol
     * @param protocol Protocol type
     * @return Pointer to channel or nullptr
     */
    CommunicationChannel* getChannel(Protocol protocol);
    
    /**
     * @brief Close all channels
     */
    void closeAll();
    
    // Delete copy/move
    CommunicationManager(const CommunicationManager&) = delete;
    CommunicationManager& operator=(const CommunicationManager&) = delete;
    
private:
    std::map<Protocol, std::unique_ptr<CommunicationChannel>> channels_;
};

} // namespace gagent

#endif /* COMMUNICATIONMANAGER_HPP_ */
