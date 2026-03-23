%skeleton "lalr1.cc"
%require "3.2"

%define api.namespace {gagent}
%define api.parser.class {FipaAclParser}
%define api.token.constructor
%define api.value.type variant
%define parse.error verbose
%define parse.assert

%code requires {
#include <string>
#include <vector>
#include "ACLMessage.hpp"

namespace gagent {
    class FipaAclDriver;
}
}

%param { FipaAclDriver& driver }

%locations

%code {
#include "FipaAclDriver.hpp"

namespace gagent {
    FipaAclParser::symbol_type yylex(FipaAclDriver& driver);
}
}

/* ── Tokens ──────────────────────────────────────────────── */

%token END 0 "end of input"
%token LPAREN "("
%token RPAREN ")"

/* Performatives */
%token KW_ACCEPT_PROPOSAL   "accept-proposal"
%token KW_AGREE             "agree"
%token KW_CANCEL            "cancel"
%token KW_CFP               "cfp"
%token KW_CONFIRM           "confirm"
%token KW_DISCONFIRM        "disconfirm"
%token KW_FAILURE           "failure"
%token KW_INFORM            "inform"
%token KW_INFORM_IF         "inform-if"
%token KW_INFORM_REF        "inform-ref"
%token KW_NOT_UNDERSTOOD    "not-understood"
%token KW_PROPAGATE         "propagate"
%token KW_PROPOSE           "propose"
%token KW_PROXY             "proxy"
%token KW_QUERY_REF         "query-ref"
%token KW_REFUSE            "refuse"
%token KW_REJECT_PROPOSAL   "reject-proposal"
%token KW_REQUEST           "request"
%token KW_REQUEST_WHEN      "request-when"
%token KW_REQUEST_WHENEVER  "request-whenever"
%token KW_SUBSCRIBE         "subscribe"

/* Mots-clés structurels */
%token KW_AGENT_IDENTIFIER  "agent-identifier"
%token KW_SET               "set"
%token KW_SEQUENCE          "sequence"

/* Paramètres de message */
%token PARAM_SENDER         ":sender"
%token PARAM_RECEIVER       ":receiver"
%token PARAM_CONTENT        ":content"
%token PARAM_REPLY_WITH     ":reply-with"
%token PARAM_IN_REPLY_TO    ":in-reply-to"
%token PARAM_REPLY_BY       ":reply-by"
%token PARAM_LANGUAGE       ":language"
%token PARAM_ENCODING       ":encoding"
%token PARAM_ONTOLOGY       ":ontology"
%token PARAM_PROTOCOL       ":protocol"
%token PARAM_CONVERSATION_ID ":conversation-id"
%token PARAM_NAME           ":name"
%token PARAM_ADDRESSES      ":addresses"
%token PARAM_RESOLVERS      ":resolvers"

/* Valeurs */
%token <std::string> WORD   "word"
%token <std::string> STRING "string"

/* ── Types des non-terminaux ─────────────────────────────── */

%type <ACLMessage::Performative>       performative
%type <AgentIdentifier>                agent_identifier
%type <std::vector<AgentIdentifier>>   agent_list receiver_spec
%type <std::string>                    expression nested_exprs

%%

/* ── Grammaire FIPA ACL ──────────────────────────────────── */

start
    : message END
    ;

message
    : LPAREN performative params RPAREN
      { driver.result.setPerformative($2); }
    ;

performative
    : KW_ACCEPT_PROPOSAL   { $$ = ACLMessage::Performative::ACCEPT_PROPOSAL; }
    | KW_AGREE             { $$ = ACLMessage::Performative::AGREE; }
    | KW_CANCEL            { $$ = ACLMessage::Performative::CANCEL; }
    | KW_CFP               { $$ = ACLMessage::Performative::CFP; }
    | KW_CONFIRM           { $$ = ACLMessage::Performative::CONFIRM; }
    | KW_DISCONFIRM        { $$ = ACLMessage::Performative::DISCONFIRM; }
    | KW_FAILURE           { $$ = ACLMessage::Performative::FAILURE; }
    | KW_INFORM            { $$ = ACLMessage::Performative::INFORM; }
    | KW_INFORM_IF         { $$ = ACLMessage::Performative::INFORM_IF; }
    | KW_INFORM_REF        { $$ = ACLMessage::Performative::INFORM_REF; }
    | KW_NOT_UNDERSTOOD    { $$ = ACLMessage::Performative::NOT_UNDERSTOOD; }
    | KW_PROPAGATE         { $$ = ACLMessage::Performative::PROPAGATE; }
    | KW_PROPOSE           { $$ = ACLMessage::Performative::PROPOSE; }
    | KW_PROXY             { $$ = ACLMessage::Performative::PROXY; }
    | KW_QUERY_REF         { $$ = ACLMessage::Performative::QUERY_REF; }
    | KW_REFUSE            { $$ = ACLMessage::Performative::REFUSE; }
    | KW_REJECT_PROPOSAL   { $$ = ACLMessage::Performative::REJECT_PROPOSAL; }
    | KW_REQUEST           { $$ = ACLMessage::Performative::REQUEST; }
    | KW_REQUEST_WHEN      { $$ = ACLMessage::Performative::REQUEST_WHEN; }
    | KW_REQUEST_WHENEVER  { $$ = ACLMessage::Performative::REQUEST_WHENEVER; }
    | KW_SUBSCRIBE         { $$ = ACLMessage::Performative::SUBSCRIBE; }
    ;

params
    : %empty
    | params param
    ;

param
    : PARAM_SENDER agent_identifier
      { driver.result.setSender(std::move($2)); }
    | PARAM_RECEIVER receiver_spec
      { for (auto& r : $2) driver.result.addReceiver(std::move(r)); }
    | PARAM_CONTENT expression
      { driver.result.setContent($2); }
    | PARAM_LANGUAGE expression
      { driver.result.setLanguage($2); }
    | PARAM_ENCODING expression
      { driver.result.setEncoding($2); }
    | PARAM_ONTOLOGY expression
      { driver.result.setOntology($2); }
    | PARAM_PROTOCOL WORD
      { driver.result.setProtocol($2); }
    | PARAM_CONVERSATION_ID expression
      { driver.result.setConversationId($2); }
    | PARAM_REPLY_WITH expression
      { driver.result.setReplyWith($2); }
    | PARAM_IN_REPLY_TO expression
      { driver.result.setInReplyTo($2); }
    | PARAM_REPLY_BY expression
      { /* datetime — extensible plus tard */ }
    ;

receiver_spec
    : agent_identifier
      { $$ = std::vector<AgentIdentifier>{std::move($1)}; }
    | LPAREN KW_SET agent_list RPAREN
      { $$ = std::move($3); }
    ;

agent_list
    : agent_identifier
      { $$ = std::vector<AgentIdentifier>{std::move($1)}; }
    | agent_list agent_identifier
      { $1.push_back(std::move($2)); $$ = std::move($1); }
    ;

agent_identifier
    : LPAREN KW_AGENT_IDENTIFIER PARAM_NAME WORD agent_id_params RPAREN
      { $$ = AgentIdentifier(std::move($4)); }
    ;

agent_id_params
    : %empty
    | agent_id_params agent_id_param
    ;

agent_id_param
    : PARAM_ADDRESSES LPAREN KW_SEQUENCE url_list RPAREN
    | PARAM_RESOLVERS LPAREN KW_SEQUENCE resolver_list RPAREN
    ;

url_list
    : %empty
    | url_list WORD
    | url_list STRING
    ;

resolver_list
    : %empty
    | resolver_list agent_identifier
    ;

expression
    : WORD                              { $$ = $1; }
    | STRING                            { $$ = $1; }
    | LPAREN nested_exprs RPAREN        { $$ = "(" + $2 + ")"; }
    ;

nested_exprs
    : %empty                            { $$ = ""; }
    | nested_exprs expression           { $$ = $1.empty() ? $2 : $1 + " " + $2; }
    ;

%%

void gagent::FipaAclParser::error(const location_type& loc, const std::string& msg)
{
    driver.parseError = msg + " [ligne " + std::to_string(loc.begin.line)
                      + ", col " + std::to_string(loc.begin.column) + "]";
}
