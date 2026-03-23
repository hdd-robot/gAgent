"""
llm_agent.py — Agent Python qui répond aux messages via un LLM (OpenAI)

La clé API est lue depuis la variable d'environnement OPENAI_API_KEY.
Si elle est absente, l'agent fonctionne en mode echo (sans LLM).

Usage :
  export OPENAI_API_KEY=sk-...
  # le script est lancé automatiquement par llm_agent (C++)
"""

import os
import sys

# Ajoute le répertoire python/ au path pour trouver gagent_py
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "python"))

import gagent_py

# ── Client LLM (optionnel) ────────────────────────────────────────────────────

api_key = os.environ.get("OPENAI_API_KEY", "")

try:
    from openai import OpenAI
    llm_client = OpenAI(api_key=api_key) if api_key else None
except ImportError:
    llm_client = None


def ask_llm(prompt: str) -> str:
    if not llm_client:
        return f"[echo sans LLM] {prompt}"
    response = llm_client.chat.completions.create(
        model="gpt-4o-mini",
        messages=[
            {"role": "system", "content":
                "Tu es un assistant dans un système multi-agent FIPA. "
                "Réponds de façon concise (1-2 phrases max)."},
            {"role": "user", "content": prompt},
        ],
        max_tokens=120,
    )
    return response.choices[0].message.content.strip()


# ── Agent ─────────────────────────────────────────────────────────────────────

class LLMAgent(gagent_py.Agent):

    def on_start(self):
        mode = "OpenAI gpt-4o-mini" if llm_client else "mode echo (pas de clé API)"
        sys.stderr.write(f"[llm_agent.py] démarré — {mode}\n")
        sys.stderr.flush()

    def on_message(self, msg: gagent_py.ACLMessage):
        sys.stderr.write(f"[llm_agent.py] reçu de {msg.sender} : {msg.content!r}\n")
        sys.stderr.flush()

        if msg.performative == "request":
            reply_content = ask_llm(msg.content)
            sys.stderr.write(f"[llm_agent.py] réponse : {reply_content!r}\n")
            sys.stderr.flush()
            return self.reply(msg, "inform", reply_content)

        if msg.performative == "cancel":
            return self.delete()

        return self.noop()

    def on_tick(self):
        return self.noop()


if __name__ == "__main__":
    LLMAgent().run()
