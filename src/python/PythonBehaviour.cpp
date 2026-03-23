#include <gagent/python/PythonBehaviour.hpp>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>
#include <cerrno>

#include <iostream>
#include <cstring>

namespace gagent {
namespace python {

/* ------------------------------------------------------------------ */
/* Constructeur / destructeur                                           */
/* ------------------------------------------------------------------ */

PythonBehaviour::PythonBehaviour(Agent*             ag,
                                 const std::string& my_name,
                                 const std::string& script_path,
                                 const std::string& system_prompt,
                                 const std::string& model,
                                 int                max_tokens,
                                 int                max_history,
                                 int                tick_ms)
    : Behaviour(ag)
    , my_name_(my_name)
    , script_path_(script_path)
    , system_prompt_(system_prompt)
    , model_(model)
    , max_tokens_(max_tokens)
    , max_history_(max_history)
    , tick_ms_(tick_ms)
{}

PythonBehaviour::~PythonBehaviour()
{
    if (to_py_   >= 0) { ::close(to_py_);   to_py_   = -1; }
    if (from_py_ >= 0) { ::close(from_py_); from_py_ = -1; }
    if (py_pid_  >  0) {
        ::kill(py_pid_, SIGTERM);
        ::waitpid(py_pid_, nullptr, WNOHANG);
        py_pid_ = -1;
    }
}

/* ------------------------------------------------------------------ */
/* Cycle de vie                                                         */
/* ------------------------------------------------------------------ */

void PythonBehaviour::onStart()
{
    // Ignorer SIGPIPE : si Python meurt, write() retourne -1/EPIPE
    // au lieu de tuer tout le processus
    ::signal(SIGPIPE, SIG_IGN);

    spawn();
    if (py_pid_ < 0) {
        std::cerr << "[PythonBehaviour] impossible de lancer " << script_path_ << "\n";
        done_ = true;
        return;
    }

    // Envoyer la configuration dans l'événement start
    std::string start = "{\"event\":\"start\""
        ",\"system_prompt\":\"" + json_escape(system_prompt_) + "\""
        ",\"model\":\""         + json_escape(model_)         + "\""
        ",\"max_tokens\":"      + std::to_string(max_tokens_) +
        ",\"max_history\":"     + std::to_string(max_history_) +
        "}";
    send_event(start);
    std::string resp = recv_response(5000);
    if (!resp.empty()) execute(resp);
}

void PythonBehaviour::action()
{
    if (done_) return;

    // Vérifier si Python est encore vivant
    if (!python_alive()) {
        std::cerr << "[PythonBehaviour] script Python terminé inopinément\n";
        done_ = true;
        return;
    }

    auto opt = messaging::acl_receive(my_name_, tick_ms_);

    if (opt) {
        send_event("{\"event\":\"message\",\"msg\":" + msg_to_json(*opt) + "}");
    } else {
        send_event("{\"event\":\"tick\"}");
    }

    if (done_) return;  // send_event peut avoir détecté un EPIPE

    std::string resp = recv_response(5000);
    if (done_) return;  // recv_response peut avoir détecté EOF
    if (resp.empty()) return;
    execute(resp);
}

void PythonBehaviour::onEnd()
{
    if (python_alive()) {
        send_event("{\"event\":\"stop\"}");
        recv_response(2000);
    }
}

/* ------------------------------------------------------------------ */
/* Vérification que Python est vivant                                   */
/* ------------------------------------------------------------------ */

bool PythonBehaviour::python_alive()
{
    if (py_pid_ <= 0) return false;
    int status;
    pid_t r = ::waitpid(py_pid_, &status, WNOHANG);
    if (r == 0) return true;   // toujours en cours
    py_pid_ = -1;
    return false;
}

/* ------------------------------------------------------------------ */
/* Spawn Python                                                         */
/* ------------------------------------------------------------------ */

void PythonBehaviour::spawn()
{
    int to_py[2], from_py[2];
    if (::pipe(to_py) < 0 || ::pipe(from_py) < 0) {
        perror("[PythonBehaviour] pipe");
        return;
    }

    py_pid_ = ::fork();
    if (py_pid_ < 0) {
        perror("[PythonBehaviour] fork");
        return;
    }

    if (py_pid_ == 0) {
        ::dup2(to_py[0],   STDIN_FILENO);
        ::dup2(from_py[1], STDOUT_FILENO);
        ::close(to_py[0]);  ::close(to_py[1]);
        ::close(from_py[0]); ::close(from_py[1]);
        ::execl("/usr/bin/python3", "python3", script_path_.c_str(), nullptr);
        ::perror("[PythonBehaviour] execl");
        ::_exit(1);
    }

    ::close(to_py[0]);
    ::close(from_py[1]);
    to_py_   = to_py[1];
    from_py_ = from_py[0];
}

/* ------------------------------------------------------------------ */
/* Communication                                                        */
/* ------------------------------------------------------------------ */

void PythonBehaviour::send_event(const std::string& json_line)
{
    if (to_py_ < 0 || done_) return;
    std::string msg = json_line + "\n";
    ssize_t r = ::write(to_py_, msg.c_str(), msg.size());
    if (r < 0) {
        if (errno == EPIPE) {
            std::cerr << "[PythonBehaviour] pipe brisé (Python mort)\n";
            done_ = true;
        } else {
            perror("[PythonBehaviour] write");
        }
    }
}

std::string PythonBehaviour::recv_response(int timeout_ms)
{
    if (from_py_ < 0 || done_) return "";

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(from_py_, &fds);

    struct timeval tv;
    tv.tv_sec  = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int r = ::select(from_py_ + 1, &fds, nullptr, nullptr, &tv);
    if (r <= 0) return "";

    std::string line;
    char c;
    ssize_t n;
    while ((n = ::read(from_py_, &c, 1)) == 1) {
        if (c == '\n') break;
        line += c;
    }
    if (n == 0) {
        // EOF : Python a fermé son stdout
        std::cerr << "[PythonBehaviour] EOF sur stdout Python\n";
        done_ = true;
        return "";
    }
    return line;
}

/* ------------------------------------------------------------------ */
/* Exécution d'une action reçue depuis Python                          */
/* ------------------------------------------------------------------ */

void PythonBehaviour::execute(const std::string& json_line)
{
    std::string act = json_get_str(json_line, "action");

    if (act == "noop" || act.empty()) {
        return;
    } else if (act == "delete") {
        done_ = true;
    } else if (act == "suspend") {
        this_agent->doSuspend();
    } else if (act == "wake") {
        this_agent->doWake();
    } else if (act == "send") {
        std::string to  = json_get_str(json_line, "to");
        std::string obj = json_get_obj(json_line, "msg");
        if (to.empty() || obj.empty()) return;

        std::string perf_str = json_get_str(obj, "performative");
        std::string content  = json_get_str(obj, "content");
        std::string ontology = json_get_str(obj, "ontology");
        std::string language = json_get_str(obj, "language");
        std::string conv_id  = json_get_str(obj, "conversation_id");
        std::string reply_to = json_get_str(obj, "in_reply_to");

        auto perf = ACLMessage::stringToPerformative(perf_str);
        ACLMessage msg(perf);
        msg.setSender(AgentIdentifier{my_name_});
        msg.setContent(content);
        if (!ontology.empty()) msg.setOntology(ontology);
        if (!language.empty()) msg.setLanguage(language);
        if (!conv_id.empty())  msg.setConversationId(conv_id);
        if (!reply_to.empty()) msg.setInReplyTo(reply_to);

        messaging::acl_send(to, msg);
    }
}

/* ------------------------------------------------------------------ */
/* Helpers JSON                                                         */
/* ------------------------------------------------------------------ */

std::string PythonBehaviour::json_escape(const std::string& s)
{
    std::string r;
    for (char c : s) {
        switch (c) {
        case '"':  r += "\\\""; break;
        case '\\': r += "\\\\"; break;
        case '\n': r += "\\n";  break;
        case '\r': r += "\\r";  break;
        case '\t': r += "\\t";  break;
        default:   r += c;
        }
    }
    return r;
}

std::string PythonBehaviour::msg_to_json(const ACLMessage& msg)
{
    auto e = [](const std::string& s) { return PythonBehaviour::json_escape(s); };

    std::string j = "{";
    j += "\"performative\":\"" + e(ACLMessage::performativeToString(msg.getPerformative())) + "\"";
    j += ",\"sender\":\""      + e(msg.getSender().name) + "\"";
    j += ",\"content\":\""     + e(msg.getContent())     + "\"";
    if (!msg.getOntology().empty())       j += ",\"ontology\":\""        + e(msg.getOntology())       + "\"";
    if (!msg.getLanguage().empty())       j += ",\"language\":\""        + e(msg.getLanguage())       + "\"";
    if (!msg.getConversationId().empty()) j += ",\"conversation_id\":\"" + e(msg.getConversationId()) + "\"";
    if (!msg.getReplyWith().empty())      j += ",\"reply_with\":\""      + e(msg.getReplyWith())      + "\"";
    if (!msg.getInReplyTo().empty())      j += ",\"in_reply_to\":\""     + e(msg.getInReplyTo())      + "\"";
    if (!msg.getProtocol().empty())       j += ",\"protocol\":\""        + e(msg.getProtocol())       + "\"";
    j += "}";
    return j;
}

std::string PythonBehaviour::json_get_str(const std::string& json,
                                           const std::string& key)
{
    std::string pattern = "\"" + key + "\":\"";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size();

    std::string val;
    for (; pos < json.size(); ++pos) {
        if (json[pos] == '\\' && pos + 1 < json.size()) {
            char next = json[pos + 1];
            if      (next == '"')  { val += '"';  ++pos; }
            else if (next == '\\') { val += '\\'; ++pos; }
            else if (next == 'n')  { val += '\n'; ++pos; }
            else if (next == 't')  { val += '\t'; ++pos; }
            else val += json[pos];
        } else if (json[pos] == '"') {
            break;
        } else {
            val += json[pos];
        }
    }
    return val;
}

std::string PythonBehaviour::json_get_obj(const std::string& json,
                                           const std::string& key)
{
    std::string pattern = "\"" + key + "\":{";
    auto pos = json.find(pattern);
    if (pos == std::string::npos) return "";
    pos += pattern.size() - 1;

    int depth = 0;
    size_t start = pos;
    for (size_t i = pos; i < json.size(); ++i) {
        if      (json[i] == '{') ++depth;
        else if (json[i] == '}') {
            if (--depth == 0) return json.substr(start, i - start + 1);
        }
    }
    return "";
}

} // namespace python
} // namespace gagent
