/**
 * @file Logger.hpp
 * @brief Centralized logging system for gAgent platform
 * @author gAgent Project
 * @date 2025
 */

#ifndef GAGENT_LOGGER_HPP_
#define GAGENT_LOGGER_HPP_

#include <iostream>
#include <fstream>
#include <sstream>
#include <mutex>
#include <memory>
#include <chrono>
#include <iomanip>

namespace gagent {

/**
 * @brief Log levels for severity classification
 */
enum class LogLevel {
    DEBUG,    ///< Debug information
    INFO,     ///< Informational messages
    WARNING,  ///< Warning messages
    ERROR,    ///< Error messages
    CRITICAL  ///< Critical errors
};

/**
 * @brief Thread-safe logger singleton
 */
class Logger {
public:
    static Logger& getInstance() {
        static Logger instance;
        return instance;
    }

    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        min_level_ = level;
    }

    bool setLogFile(const std::string& filename) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_file_.open(filename, std::ios::app);
        return log_file_.is_open();
    }

    void log(LogLevel level, const std::string& message,
             const std::string& source = "") {
        if (level < min_level_) {
            return;
        }

        std::lock_guard<std::mutex> lock(mutex_);

        std::ostringstream oss;
        oss << getTimestamp() << " "
            << "[" << levelToString(level) << "] ";

        if (!source.empty()) {
            oss << "[" << source << "] ";
        }

        oss << message << std::endl;

        std::string formatted = oss.str();
        std::cout << formatted;

        if (log_file_.is_open()) {
            log_file_ << formatted;
            log_file_.flush();
        }
    }

    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }

    // Convenience methods
    void debug(const std::string& msg, const std::string& src = "") {
        log(LogLevel::DEBUG, msg, src);
    }

    void info(const std::string& msg, const std::string& src = "") {
        log(LogLevel::INFO, msg, src);
    }

    void warning(const std::string& msg, const std::string& src = "") {
        log(LogLevel::WARNING, msg, src);
    }

    void error(const std::string& msg, const std::string& src = "") {
        log(LogLevel::ERROR, msg, src);
    }

    void critical(const std::string& msg, const std::string& src = "") {
        log(LogLevel::CRITICAL, msg, src);
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() : min_level_(LogLevel::INFO) {}

    ~Logger() {
        close();
    }

    std::string levelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG:    return "DEBUG";
            case LogLevel::INFO:     return "INFO ";
            case LogLevel::WARNING:  return "WARN ";
            case LogLevel::ERROR:    return "ERROR";
            case LogLevel::CRITICAL: return "CRIT ";
            default:                 return "?????";
        }
    }

    std::string getTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count();
        return oss.str();
    }

    std::mutex mutex_;
    LogLevel min_level_;
    std::ofstream log_file_;
};

// Macro helpers for convenient logging
#define LOG_DEBUG(msg) gagent::Logger::getInstance().debug(msg, __func__)
#define LOG_INFO(msg) gagent::Logger::getInstance().info(msg, __func__)
#define LOG_WARNING(msg) gagent::Logger::getInstance().warning(msg, __func__)
#define LOG_ERROR(msg) gagent::Logger::getInstance().error(msg, __func__)
#define LOG_CRITICAL(msg) gagent::Logger::getInstance().critical(msg, __func__)

} // namespace gagent

#endif /* GAGENT_LOGGER_HPP_ */
