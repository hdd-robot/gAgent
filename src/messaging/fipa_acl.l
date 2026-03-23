%{
#include "FipaAclDriver.hpp"
#include "fipa_acl_parser.hpp"

/* Declaration dans le namespace pour que la definition qualifiee soit valide en C++ */
namespace gagent {
    FipaAclParser::symbol_type yylex(FipaAclDriver& driver);
}

#define YY_DECL \
    gagent::FipaAclParser::symbol_type \
    gagent::yylex(gagent::FipaAclDriver& driver)

using tok = gagent::FipaAclParser;

#define YY_USER_ACTION driver.loc.columns(yyleng);
%}

%option noyywrap nounput noinput batch

%%

[ \t\r]+        { driver.loc.step(); }
"\n"+           { driver.loc.lines(yyleng); driver.loc.step(); }

"("             { return tok::make_LPAREN(driver.loc); }
")"             { return tok::make_RPAREN(driver.loc); }

":sender"           { return tok::make_PARAM_SENDER(driver.loc); }
":receiver"         { return tok::make_PARAM_RECEIVER(driver.loc); }
":content"          { return tok::make_PARAM_CONTENT(driver.loc); }
":reply-with"       { return tok::make_PARAM_REPLY_WITH(driver.loc); }
":in-reply-to"      { return tok::make_PARAM_IN_REPLY_TO(driver.loc); }
":reply-by"         { return tok::make_PARAM_REPLY_BY(driver.loc); }
":language"         { return tok::make_PARAM_LANGUAGE(driver.loc); }
":encoding"         { return tok::make_PARAM_ENCODING(driver.loc); }
":ontology"         { return tok::make_PARAM_ONTOLOGY(driver.loc); }
":protocol"         { return tok::make_PARAM_PROTOCOL(driver.loc); }
":conversation-id"  { return tok::make_PARAM_CONVERSATION_ID(driver.loc); }
":name"             { return tok::make_PARAM_NAME(driver.loc); }
":addresses"        { return tok::make_PARAM_ADDRESSES(driver.loc); }
":resolvers"        { return tok::make_PARAM_RESOLVERS(driver.loc); }

"accept-proposal"   { return tok::make_KW_ACCEPT_PROPOSAL(driver.loc); }
"agree"             { return tok::make_KW_AGREE(driver.loc); }
"cancel"            { return tok::make_KW_CANCEL(driver.loc); }
"cfp"               { return tok::make_KW_CFP(driver.loc); }
"confirm"           { return tok::make_KW_CONFIRM(driver.loc); }
"disconfirm"        { return tok::make_KW_DISCONFIRM(driver.loc); }
"failure"           { return tok::make_KW_FAILURE(driver.loc); }
"inform-if"         { return tok::make_KW_INFORM_IF(driver.loc); }
"inform-ref"        { return tok::make_KW_INFORM_REF(driver.loc); }
"inform"            { return tok::make_KW_INFORM(driver.loc); }
"not-understood"    { return tok::make_KW_NOT_UNDERSTOOD(driver.loc); }
"propagate"         { return tok::make_KW_PROPAGATE(driver.loc); }
"propose"           { return tok::make_KW_PROPOSE(driver.loc); }
"proxy"             { return tok::make_KW_PROXY(driver.loc); }
"query-ref"         { return tok::make_KW_QUERY_REF(driver.loc); }
"refuse"            { return tok::make_KW_REFUSE(driver.loc); }
"reject-proposal"   { return tok::make_KW_REJECT_PROPOSAL(driver.loc); }
"request-when"      { return tok::make_KW_REQUEST_WHEN(driver.loc); }
"request-whenever"  { return tok::make_KW_REQUEST_WHENEVER(driver.loc); }
"request"           { return tok::make_KW_REQUEST(driver.loc); }
"subscribe"         { return tok::make_KW_SUBSCRIBE(driver.loc); }

"agent-identifier"  { return tok::make_KW_AGENT_IDENTIFIER(driver.loc); }
"set"               { return tok::make_KW_SET(driver.loc); }
"sequence"          { return tok::make_KW_SEQUENCE(driver.loc); }

\"(\\.|[^"\\])*\" {
    std::string raw(yytext + 1, yyleng - 2);
    std::string result;
    result.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            ++i;
            switch (raw[i]) {
                case 'n':  result += '\n'; break;
                case 't':  result += '\t'; break;
                case '"':  result += '"';  break;
                case '\\': result += '\\'; break;
                default:   result += raw[i]; break;
            }
        } else {
            result += raw[i];
        }
    }
    return tok::make_STRING(result, driver.loc);
}

[a-zA-Z][a-zA-Z0-9_\-\.@:/]* {
    return tok::make_WORD(std::string(yytext), driver.loc);
}

. {
    throw gagent::FipaAclParser::syntax_error(
        driver.loc,
        std::string("unexpected character: '") + yytext + "'");
}

<<EOF>> { return tok::make_END(driver.loc); }

%%

bool fipa_acl_parse_string(const std::string& input, gagent::FipaAclDriver& driver)
{
    YY_BUFFER_STATE buf = yy_scan_string(input.c_str());
    gagent::FipaAclParser parser(driver);
    bool ok = false;
    try {
        ok = (parser.parse() == 0) && driver.parseError.empty();
    } catch (const gagent::FipaAclParser::syntax_error& e) {
        driver.parseError = e.what();
    }
    yy_delete_buffer(buf);
    return ok;
}
