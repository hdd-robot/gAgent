/**
 * @file Logger.hpp
 * @brief Centralized logging system for gAgent platform
 *
 * Two output modes (independent, both can be active simultaneously):
 *
 *   1. Text log  — human-readable, controlled by setLogFile() / LOG_* macros
 *   2. JSON Lines — structured, one JSON object per line, activated by the
 *                   GAGENT_LOG environment variable (path to .jsonl file)
 *
 * JSON line format:
 *   {"ts":"2026-03-24T10:00:00.123Z","event":"acl_send","from":"alice","to":"bob",...}
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
#include <cstdlib>
#include <initializer_list>
#include <utility>
#include <string>

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

    // ── Text log ─────────────────────────────────────────────────────────────

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
        if (json_file_.is_open()) {
            json_file_.close();
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

    // ── JSON Lines structured log ─────────────────────────────────────────────

    /**
     * Emit one JSON line.
     *
     * Activated by the GAGENT_LOG env var (set at process start).
     * If GAGENT_LOG is not set the call is a no-op.
     *
     * Usage:
     *   Logger::getInstance().logJson("acl_send", {
     *       {"from",  "alice"},
     *       {"to",    "bob"},
     *       {"perf",  "request"},
     *       {"content", "hello"},
     *   });
     *
     * Output (one line):
     *   {"ts":"2026-03-24T10:00:00.123Z","event":"acl_send","from":"alice",...}
     */
    void logJson(const std::string& event,
                 std::initializer_list<std::pair<const char*, std::string>> fields)
    {
        if (!json_enabled_) return;

        std::lock_guard<std::mutex> lock(mutex_);

        std::ostringstream oss;
        oss << "{\"ts\":\"" << getTimestampISO() << "\""
            << ",\"event\":\"" << jsonEscape(event) << "\"";
        for (auto& [k, v] : fields) {
            oss << ",\"" << k << "\":\"" << jsonEscape(v) << "\"";
        }
        oss << "}\n";

        json_file_ << oss.str();
        json_file_.flush();
    }

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() : min_level_(LogLevel::INFO) {
        // Activate JSON logging if GAGENT_LOG env var is set
        const char* path = std::getenv("GAGENT_LOG");
        if (path && path[0] != '\0') {
            json_file_.open(path, std::ios::app);
            json_enabled_ = json_file_.is_open();
        }
    }

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

    /** ISO 8601 UTC timestamp with millisecond precision. */
    std::string getTimestampISO() const {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;

        std::ostringstream oss;
        oss << std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S")
            << "." << std::setfill('0') << std::setw(3) << ms.count() << "Z";
        return oss.str();
    }

    /** Escape a string for JSON (backslash, double-quote, control chars). */
    static std::string jsonEscape(const std::string& s) {
        std::string out;
        out.reserve(s.size());
        for (unsigned char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (c < 0x20) {
                        // control character
                        char buf[8];
                        snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    } else {
                        out += static_cast<char>(c);
                    }
            }
        }
        return out;
    }

    std::mutex    mutex_;
    LogLevel      min_level_;
    std::ofstream log_file_;

    bool          json_enabled_ = false;
    std::ofstream json_file_;
};

// Macro helpers for convenient logging
#define LOG_DEBUG(msg) gagent::Logger::getInstance().debug(msg, __func__)
#define LOG_INFO(msg) gagent::Logger::getInstance().info(msg, __func__)
#define LOG_WARNING(msg) gagent::Logger::getInstance().warning(msg, __func__)
#define LOG_ERROR(msg) gagent::Logger::getInstance().error(msg, __func__)
#define LOG_CRITICAL(msg) gagent::Logger::getInstance().critical(msg, __func__)

// Macro for JSON structured logging
#define LOG_JSON(event, ...) gagent::Logger::getInstance().logJson(event, {__VA_ARGS__})

} // namespace gagent

#endif /* GAGENT_LOGGER_HPP_ */
