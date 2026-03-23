/**
 * @file ErrorHandler.hpp
 * @brief Centralized error handling for gAgent platform
 * @author gAgent Project
 * @date 2025
 */

#ifndef GAGENT_ERRORHANDLER_HPP_
#define GAGENT_ERRORHANDLER_HPP_

#include <string>
#include <exception>
#include <stdexcept>

namespace gagent {

/**
 * @brief Base exception class for gAgent errors
 */
class AgentException : public std::runtime_error {
public:
    explicit AgentException(const std::string& message)
        : std::runtime_error(message) {}
};

/**
 * @brief Exception for initialization failures
 */
class InitializationException : public AgentException {
public:
    explicit InitializationException(const std::string& message)
        : AgentException("Initialization failed: " + message) {}
};

/**
 * @brief Exception for communication errors
 */
class CommunicationException : public AgentException {
public:
    explicit CommunicationException(const std::string& message)
        : AgentException("Communication error: " + message) {}
};

/**
 * @brief Exception for message parsing errors
 */
class MessageParsingException : public AgentException {
public:
    explicit MessageParsingException(const std::string& message)
        : AgentException("Message parsing failed: " + message) {}
};

/**
 * @brief Exception for behavior errors
 */
class BehaviourException : public AgentException {
public:
    explicit BehaviourException(const std::string& message)
        : AgentException("Behaviour error: " + message) {}
};

/**
 * @brief Exception for lifecycle management errors
 */
class LifecycleException : public AgentException {
public:
    explicit LifecycleException(const std::string& message)
        : AgentException("Lifecycle error: " + message) {}
};

/**
 * @brief Error handler utility functions
 */
class ErrorHandler {
public:
    /**
     * @brief Handle exception and log error
     * @param e Exception to handle
     * @param context Context information
     * @return false to indicate failure
     */
    static bool handleException(const std::exception& e,
                               const std::string& context);

    /**
     * @brief Log system error with errno
     * @param context Context information
     */
    static void logSystemError(const std::string& context);

private:
    ErrorHandler() = delete;
};

} // namespace gagent

#endif /* GAGENT_ERRORHANDLER_HPP_ */
