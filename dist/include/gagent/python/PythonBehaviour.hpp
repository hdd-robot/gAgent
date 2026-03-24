/*
 * PythonBehaviour.hpp — Behaviour dont la logique est déléguée à un script Python
 *
 * Le script Python reçoit des événements en JSON sur stdin et retourne
 * des actions en JSON sur stdout. Il peut appeler n'importe quel LLM.
 *
 * ── Protocole ──────────────────────────────────────────────────────────────
 *
 *  C++ → Python (stdin, une ligne JSON par événement) :
 *
 *    {"event":"start","system_prompt":"...","model":"gpt-4o-mini","max_tokens":200,"max_history":20}
 *    {"event":"message","msg":{"performative":"request","sender":"alice","content":"...",...}}
 *    {"event":"tick"}
 *    {"event":"stop"}
 *
 *  Python → C++ (stdout, une ligne JSON par action) :
 *
 *    {"action":"noop"}
 *    {"action":"send","to":"bob","msg":{"performative":"inform","content":"..."}}
 *    {"action":"delete"}
 *    {"action":"suspend"}
 *    {"action":"wake"}
 *
 * ── Usage ───────────────────────────────────────────────────────────────────
 *
 *  class MonAgent : public Agent {
 *  public:
 *      void setup() override {
 *          addBehaviour(new PythonBehaviour(this, "mon-agent", "mon_script.py",
 *              "Tu es un agent spécialisé en planification."));
 *      }
 *      void takeDown() override { messaging::acl_unlink("mon-agent"); }
 *  };
 */

#ifndef GAGENT_PYTHON_PYTHONBEHAVIOUR_HPP_
#define GAGENT_PYTHON_PYTHONBEHAVIOUR_HPP_

#include <gagent/core/Behaviour.hpp>
#include <gagent/messaging/ACLMessage.hpp>
#include <gagent/messaging/AclMQ.hpp>

#include <string>
#include <sys/types.h>

namespace gagent {
namespace python {

class PythonBehaviour : public Behaviour {
public:
    /**
     * @param ag            pointeur vers l'agent propriétaire
     * @param my_name       nom de la queue ACL (/acl_<my_name>)
     * @param script_path   chemin vers le script Python
     * @param system_prompt prompt système envoyé au LLM (configurable depuis C++)
     * @param model         modèle LLM (ex: "gpt-4o-mini")
     * @param max_tokens    longueur max de la réponse
     * @param max_history   nombre max de tours de conversation conservés
     * @param tick_ms       intervalle de tick quand aucun message n'arrive (ms)
     */
    PythonBehaviour(Agent*             ag,
                    const std::string& my_name,
                    const std::string& script_path,
                    const std::string& system_prompt = "",
                    const std::string& model         = "gpt-4o-mini",
                    int                max_tokens     = 200,
                    int                max_history    = 20,
                    int                tick_ms        = 200);

    ~PythonBehaviour();

    void onStart() override;
    void action()  override;
    void onEnd()   override;
    bool done()    override { return done_; }

private:
    std::string my_name_;
    std::string script_path_;
    std::string system_prompt_;
    std::string model_;
    int         max_tokens_;
    int         max_history_;
    int         tick_ms_;
    bool        done_ = false;

    pid_t py_pid_  = -1;
    int   to_py_   = -1;
    int   from_py_ = -1;

    void        spawn();
    void        send_event(const std::string& json_line);
    std::string recv_response(int timeout_ms = 3000);
    void        execute(const std::string& json_line);
    bool        python_alive();

    static std::string msg_to_json(const ACLMessage& msg);
    static std::string json_escape(const std::string& s);
    static std::string json_get_str(const std::string& json,
                                    const std::string& key);
    static std::string json_get_obj(const std::string& json,
                                    const std::string& key);
};

} // namespace python
} // namespace gagent

#endif /* GAGENT_PYTHON_PYTHONBEHAVIOUR_HPP_ */
