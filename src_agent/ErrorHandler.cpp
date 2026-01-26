/**
 * @file ErrorHandler.cpp
 * @brief Implementation of error handling utilities
 */

#include "ErrorHandler.hpp"
#include "Logger.hpp"
#include <cerrno>
#include <cstring>

namespace gagent {

bool ErrorHandler::handleException(const std::exception& e, 
                                   const std::string& context) {
    std::string message = context + ": " + e.what();
    Logger::getInstance().error(message);
    return false;
}

void ErrorHandler::logSystemError(const std::string& context) {
    std::string error = context + ": " + std::strerror(errno);
    Logger::getInstance().error(error);
}

} // namespace gagent
