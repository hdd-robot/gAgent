/*
 * FipaAclDriver.cpp
 */

#include "FipaAclDriver.hpp"

namespace gagent {

std::optional<ACLMessage> FipaAclDriver::parse(const std::string& input)
{
    result      = ACLMessage{};
    parseError  = "";
    loc         = FipaAclParser::location_type{};

    if (!fipa_acl_parse_string(input, *this)) {
        return std::nullopt;
    }
    return result;
}

} // namespace gagent
