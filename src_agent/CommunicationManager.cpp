/**
 * @file CommunicationManager.cpp
 * @brief Implementation of communication manager
 */

#include "CommunicationManager.hpp"
#include <mqueue.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

namespace gagent {

//=============================================================================
// UDPChannel Implementation
//=============================================================================

class UDPChannel::Impl {
public:
    int socket_fd = -1;
    std::string address;
    int port;
    bool active = false;
};

UDPChannel::UDPChannel(const std::string& address, int port)
    : impl_(new Impl()) {
    impl_->address = address;
    impl_->port = port;
    
    // Create UDP socket
    impl_->socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (impl_->socket_fd < 0) {
        throw CommunicationException("Failed to create UDP socket");
    }
    
    // Set non-blocking
    int flags = fcntl(impl_->socket_fd, F_GETFL, 0);
    fcntl(impl_->socket_fd, F_SETFL, flags | O_NONBLOCK);
    
    // Bind socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(address.c_str());
    
    if (bind(impl_->socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        ::close(impl_->socket_fd);
        throw CommunicationException("Failed to bind UDP socket");
    }
    
    impl_->active = true;
    LOG_INFO("UDP channel initialized on " + address + ":" + std::to_string(port));
}

UDPChannel::~UDPChannel() {
    close();
}

bool UDPChannel::send(const ACLMessage& message, const std::string& destination) {
    if (!impl_->active) return false;
    
    try {
        // TODO: Serialize ACLMessage to string
        std::string msg_str = "ACL message"; // Placeholder
        
        // Parse destination (format: "host:port")
        size_t colon_pos = destination.find(':');
        if (colon_pos == std::string::npos) {
            LOG_ERROR("Invalid destination format: " + destination);
            return false;
        }
        
        std::string host = destination.substr(0, colon_pos);
        int port = std::stoi(destination.substr(colon_pos + 1));
        
        struct sockaddr_in dest_addr;
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(port);
        dest_addr.sin_addr.s_addr = inet_addr(host.c_str());
        
        ssize_t sent = sendto(impl_->socket_fd, msg_str.c_str(), msg_str.length(),
                             0, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
        
        return sent >= 0;
        
    } catch (const std::exception& e) {
        LOG_ERROR(std::string("UDP send failed: ") + e.what());
        return false;
    }
}

bool UDPChannel::receive(ACLMessage& message) {
    if (!impl_->active) return false;
    
    char buffer[4096];
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    
    ssize_t received = recvfrom(impl_->socket_fd, buffer, sizeof(buffer) - 1,
                               0, (struct sockaddr*)&sender_addr, &addr_len);
    
    if (received > 0) {
        buffer[received] = '\0';
        // TODO: Parse buffer into ACLMessage
        LOG_DEBUG("UDP message received");
        return true;
    }
    
    return false;
}

bool UDPChannel::isActive() const {
    return impl_->active;
}

void UDPChannel::close() {
    if (impl_->socket_fd >= 0) {
        ::close(impl_->socket_fd);
        impl_->socket_fd = -1;
    }
    impl_->active = false;
    LOG_INFO("UDP channel closed");
}

//=============================================================================
// CORBAChannel Implementation (stub for now)
//=============================================================================

class CORBAChannel::Impl {
public:
    bool active = false;
    // TODO: Add CORBA ORB and object references
};

CORBAChannel::CORBAChannel(const std::string& nameserver_ior)
    : impl_(new Impl()) {
    // TODO: Initialize CORBA ORB
    LOG_INFO("CORBA channel initialized");
    impl_->active = true;
}

CORBAChannel::~CORBAChannel() {
    close();
}

bool CORBAChannel::send(const ACLMessage& message, const std::string& destination) {
    // TODO: Implement CORBA message sending
    LOG_WARNING("CORBA send not yet implemented");
    return false;
}

bool CORBAChannel::receive(ACLMessage& message) {
    // TODO: Implement CORBA message receiving
    return false;
}

bool CORBAChannel::isActive() const {
    return impl_->active;
}

void CORBAChannel::close() {
    // TODO: Shutdown CORBA ORB
    impl_->active = false;
    LOG_INFO("CORBA channel closed");
}

//=============================================================================
// MQChannel Implementation
//=============================================================================

class MQChannel::Impl {
public:
    mqd_t mq = -1;
    std::string queue_name;
    bool active = false;
};

MQChannel::MQChannel(const std::string& queue_name)
    : impl_(new Impl()) {
    impl_->queue_name = queue_name;
    
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 4096;
    attr.mq_curmsgs = 0;
    
    impl_->mq = mq_open(queue_name.c_str(), O_RDWR | O_CREAT | O_NONBLOCK, 
                        0666, &attr);
    
    if (impl_->mq == (mqd_t)-1) {
        throw CommunicationException("Failed to open message queue: " + queue_name);
    }
    
    impl_->active = true;
    LOG_INFO("Message queue opened: " + queue_name);
}

MQChannel::~MQChannel() {
    close();
}

bool MQChannel::send(const ACLMessage& message, const std::string& destination) {
    if (!impl_->active) return false;
    
    // TODO: Serialize ACLMessage
    std::string msg_str = "ACL message";
    
    if (mq_send(impl_->mq, msg_str.c_str(), msg_str.length(), 0) == 0) {
        return true;
    }
    
    return false;
}

bool MQChannel::receive(ACLMessage& message) {
    if (!impl_->active) return false;
    
    char buffer[4096];
    ssize_t received = mq_receive(impl_->mq, buffer, sizeof(buffer), nullptr);
    
    if (received > 0) {
        buffer[received] = '\0';
        // TODO: Parse into ACLMessage
        return true;
    }
    
    return false;
}

bool MQChannel::isActive() const {
    return impl_->active;
}

void MQChannel::close() {
    if (impl_->mq != (mqd_t)-1) {
        mq_close(impl_->mq);
        mq_unlink(impl_->queue_name.c_str());
        impl_->mq = -1;
    }
    impl_->active = false;
    LOG_INFO("Message queue closed: " + impl_->queue_name);
}

//=============================================================================
// CommunicationManager Implementation
//=============================================================================

CommunicationManager::CommunicationManager() {
    LOG_INFO("Communication manager initialized");
}

CommunicationManager::~CommunicationManager() {
    closeAll();
}

void CommunicationManager::registerChannel(Protocol protocol, 
                                          std::unique_ptr<CommunicationChannel> channel) {
    channels_[protocol] = std::move(channel);
    LOG_DEBUG("Channel registered for protocol " + std::to_string(static_cast<int>(protocol)));
}

bool CommunicationManager::send(Protocol protocol, const ACLMessage& message, 
                               const std::string& destination) {
    auto it = channels_.find(protocol);
    if (it == channels_.end() || !it->second) {
        LOG_ERROR("Channel not found for protocol");
        return false;
    }
    
    return it->second->send(message, destination);
}

bool CommunicationManager::receive(ACLMessage& message, Protocol& protocol) {
    // Try receiving from all channels
    for (auto& pair : channels_) {
        if (pair.second && pair.second->receive(message)) {
            protocol = pair.first;
            return true;
        }
    }
    return false;
}

CommunicationChannel* CommunicationManager::getChannel(Protocol protocol) {
    auto it = channels_.find(protocol);
    return (it != channels_.end()) ? it->second.get() : nullptr;
}

void CommunicationManager::closeAll() {
    for (auto& pair : channels_) {
        if (pair.second) {
            pair.second->close();
        }
    }
    channels_.clear();
    LOG_INFO("All communication channels closed");
}

} // namespace gagent
