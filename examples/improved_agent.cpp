/**
 * @example improved_agent.cpp
 * @brief Improved agent example with modern C++ practices
 * 
 * This example demonstrates:
 * - Smart pointer usage
 * - Logging system integration
 * - Error handling
 * - Custom behaviours
 * - Environment visualization
 */

#include <iostream>
#include <memory>
#include <string>

#include "../src_agent/AgentCore.hpp"
#include "../src_agent/Environnement.hpp"
#include "../src_agent/Agent.hpp"
#include "../src_agent/Behaviour.hpp"
#include "../src_agent/Logger.hpp"
#include "../src_agent/ErrorHandler.hpp"

using namespace gagent;

/**
 * @brief Custom environment with improved initialization
 */
class ImprovedEnvironment : public Environnement {
public:
    void init_env() override {
        LOG_INFO("Environment initialized");
    }

    void link_attribut() override {
        link_id("id");
        link_name("name");
        link_pos_x("x");
        link_pos_y("y");
        link_color("color");
        link_size("size");
        link_shape("shape");
        LOG_DEBUG("Attributes linked");
    }

    void event_loop() override {
        // Update logic here
    }
};

/**
 * @brief Ticker behaviour with logging
 */
class LoggingTickerBehaviour : public TickerBehaviour {
public:
    explicit LoggingTickerBehaviour(Agent* ag, int interval_ms = 1000)
        : TickerBehaviour(ag, interval_ms), counter_(0) {
        LOG_INFO("LoggingTickerBehaviour created with " + 
                 std::to_string(interval_ms) + "ms interval");
    }

    void onStart() override {
        LOG_INFO("Behaviour started");
    }

    void onTick() override {
        try {
            counter_++;
            
            // Update agent attributes
            this_agent->setAttribut("x", std::to_string(counter_ * 10));
            this_agent->setAttribut("y", std::to_string(counter_ * 5));
            this_agent->setAttribut("status", "active");
            
            // Notify environment
            this_agent->attributUpdated();
            
            // Log every 10 ticks
            if (counter_ % 10 == 0) {
                LOG_INFO("Tick count: " + std::to_string(counter_));
            }
            
            // Stop after 100 ticks for demo
            if (counter_ >= 100) {
                LOG_INFO("Reached 100 ticks, requesting shutdown");
                this_agent->doDelete();
            }
            
        } catch (const std::exception& e) {
            ErrorHandler::handleException(e, "LoggingTickerBehaviour::onTick");
        }
    }

    void onEnd() override {
        LOG_INFO("Behaviour ended after " + std::to_string(counter_) + " ticks");
    }

private:
    int counter_;
};

/**
 * @brief Message handling behaviour
 */
class MessageHandlerBehaviour : public CyclicBehaviour {
public:
    explicit MessageHandlerBehaviour(Agent* ag) : CyclicBehaviour(ag) {}

    void onStart() override {
        LOG_INFO("Message handler started");
    }

    void action() override {
        try {
            // Check for incoming messages
            // In a real implementation, this would process the message queue
            
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
        } catch (const std::exception& e) {
            ErrorHandler::handleException(e, "MessageHandlerBehaviour::action");
        }
    }

    void onEnd() override {
        LOG_INFO("Message handler stopped");
    }
};

/**
 * @brief Improved agent implementation
 */
class ImprovedAgent : public Agent {
public:
    ImprovedAgent() {
        LOG_INFO("ImprovedAgent constructor called");
    }

    ~ImprovedAgent() override {
        LOG_INFO("ImprovedAgent destructor called");
    }

    void setup() override {
        try {
            LOG_INFO("Agent setup starting");
            
            // Set agent attributes
            addAttribut("id");
            addAttribut("name");
            addAttribut("x");
            addAttribut("y");
            addAttribut("status");
            addAttribut("color");
            addAttribut("shape");
            
            setAttribut("name", "ImprovedAgent-001");
            setAttribut("color", "blue");
            setAttribut("shape", "circle");
            setAttribut("status", "initializing");
            
            // Add behaviours
            addBehaviour(new LoggingTickerBehaviour(this, 500));  // 500ms ticks
            addBehaviour(new MessageHandlerBehaviour(this));
            
            LOG_INFO("Agent setup completed successfully");
            
        } catch (const std::exception& e) {
            ErrorHandler::handleException(e, "ImprovedAgent::setup");
            throw InitializationException("Failed to setup agent");
        }
    }

    void takeDown() override {
        try {
            LOG_INFO("Agent takeDown starting");
            
            setAttribut("status", "shutting_down");
            attributUpdated();
            
            // Cleanup resources here
            
            LOG_INFO("Agent takeDown completed");
            
        } catch (const std::exception& e) {
            ErrorHandler::handleException(e, "ImprovedAgent::takeDown");
        }
    }
};

/**
 * @brief Main function demonstrating improved agent usage
 */
int main(int argc, char* argv[]) {
    try {
        // Initialize logging
        Logger::getInstance().setLogLevel(LogLevel::DEBUG);
        Logger::getInstance().setLogFile("improved_agent.log");
        
        LOG_INFO("=== Starting Improved Agent Demo ===");
        
        // Initialize agent system
        AgentCore::initAgentSystem();
        LOG_INFO("Agent system initialized");
        
        // Create and configure environment
        ImprovedEnvironment env;
        bool use_gui = false;  // Set to true to enable Qt GUI
        
        if (argc > 1 && std::string(argv[1]) == "--gui") {
            use_gui = true;
            LOG_INFO("GUI mode enabled");
        }
        
        AgentCore::initEnvironnementSystem(env, use_gui, 500);
        LOG_INFO("Environment system initialized");
        
        // Create and initialize agent
        LOG_INFO("Creating agent...");
        ImprovedAgent agent;
        agent.init();
        
        // Wait for agent to complete
        AgentCore::syncAgentSystem();
        
        LOG_INFO("=== Agent Demo Completed ===");
        
        return EXIT_SUCCESS;
        
    } catch (const AgentException& e) {
        LOG_CRITICAL(std::string("Agent exception: ") + e.what());
        return EXIT_FAILURE;
        
    } catch (const std::exception& e) {
        LOG_CRITICAL(std::string("Unexpected error: ") + e.what());
        return EXIT_FAILURE;
        
    } catch (...) {
        LOG_CRITICAL("Unknown error occurred");
        return EXIT_FAILURE;
    }
}
