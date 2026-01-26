/**
 * @file BehaviourFactory.hpp
 * @brief Factory pattern for creating Behaviour instances
 * @author gAgent Project
 * @date 2025
 */

#ifndef BEHAVIOURFACTORY_HPP_
#define BEHAVIOURFACTORY_HPP_

#include <memory>
#include <string>
#include <functional>
#include <map>
#include "Behaviour.hpp"
#include "ErrorHandler.hpp"

namespace gagent {

/**
 * @brief Factory for creating Behaviour instances
 * 
 * Implements the Factory pattern to create behaviour objects dynamically.
 * Supports registration of custom behaviour types.
 * 
 * Example usage:
 * @code
 * auto factory = BehaviourFactory::getInstance();
 * factory.registerBehaviour("MyBehaviour", 
 *     [](Agent* ag) { return std::make_unique<MyBehaviour>(ag); });
 * 
 * auto behaviour = factory.create("MyBehaviour", agent);
 * @endcode
 */
class BehaviourFactory {
public:
    using BehaviourCreator = std::function<std::unique_ptr<Behaviour>(Agent*)>;

    /**
     * @brief Get singleton instance
     * @return Reference to factory instance
     */
    static BehaviourFactory& getInstance() {
        static BehaviourFactory instance;
        return instance;
    }

    /**
     * @brief Register a new behaviour type
     * @param name Type name identifier
     * @param creator Function to create the behaviour
     */
    void registerBehaviour(const std::string& name, BehaviourCreator creator) {
        creators_[name] = creator;
    }

    /**
     * @brief Create a behaviour instance
     * @param name Type name
     * @param agent Agent owner
     * @return Unique pointer to created behaviour
     * @throw BehaviourException if type not found
     */
    std::unique_ptr<Behaviour> create(const std::string& name, Agent* agent) {
        auto it = creators_.find(name);
        if (it == creators_.end()) {
            throw BehaviourException("Unknown behaviour type: " + name);
        }
        return it->second(agent);
    }

    /**
     * @brief Check if behaviour type is registered
     * @param name Type name
     * @return true if registered
     */
    bool isRegistered(const std::string& name) const {
        return creators_.find(name) != creators_.end();
    }

    // Delete copy/move
    BehaviourFactory(const BehaviourFactory&) = delete;
    BehaviourFactory& operator=(const BehaviourFactory&) = delete;

private:
    BehaviourFactory() = default;
    std::map<std::string, BehaviourCreator> creators_;
};

} // namespace gagent

#endif /* BEHAVIOURFACTORY_HPP_ */
