"""
llm_agent.py — Agent Python avec mémoire de conversation et LLM configurable

Configuration passée par le C++ via l'événement "start" :
  system_prompt  — personnalité / rôle de l'agent
  model          — modèle OpenAI (ex: "gpt-4o-mini")
  max_tokens     — longueur max de réponse
  max_history    — nombre max de tours conservés en mémoire

La clé API est lue depuis OPENAI_API_KEY.
Sans clé, l'agent fonctionne en mode echo (sans LLM).
"""

import os
import sys

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "python"))

import gagent_py

# ── Client LLM ────────────────────────────────────────────────────────────────

try:
    from openai import OpenAI, APIError, APIConnectionError
    _api_key = os.environ.get("OPENAI_API_KEY", "")
    llm_client = OpenAI(api_key=_api_key) if _api_key else None
except ImportError:
    llm_client = None
    APIError = APIConnectionError = Exception


# ── Agent ─────────────────────────────────────────────────────────────────────

class LLMAgent(gagent_py.Agent):

    def on_start(self):
        # Historique de conversation (roulant, limité à max_history tours)
        self._history: list[dict] = []

        mode = f"OpenAI {self.model}" if llm_client else "mode echo (pas de clé API)"
        sys.stderr.write(
            f"[llm_agent.py] démarré — {mode} | "
            f"max_history={self.max_history} | "
            f"prompt={self.system_prompt[:60]!r}...\n"
        )
        sys.stderr.flush()

    def on_message(self, msg: gagent_py.ACLMessage):

        # Arrêt propre sur CANCEL
        if msg.performative == "cancel":
            sys.stderr.write("[llm_agent.py] CANCEL reçu — arrêt\n")
            sys.stderr.flush()
            return self.delete()

        # Seuls les REQUEST déclenchent le LLM
        if msg.performative != "request" or not msg.content:
            return self.noop()

        sys.stderr.write(f"[llm_agent.py] [{msg.sender}] {msg.content!r}\n")
        sys.stderr.flush()

        reply_content = self._ask(msg.content)

        sys.stderr.write(f"[llm_agent.py] → {reply_content!r}\n")
        sys.stderr.flush()

        return self.reply(msg, "inform", reply_content)

    def on_tick(self):
        return self.noop()

    # ── Appel LLM avec historique ─────────────────────────────────────

    def _ask(self, user_input: str) -> str:
        if not llm_client:
            return f"[echo] {user_input}"

        # Ajouter le message utilisateur à l'historique
        self._history.append({"role": "user", "content": user_input})

        # Garder seulement les max_history derniers tours (2 messages par tour)
        if len(self._history) > self.max_history * 2:
            self._history = self._history[-(self.max_history * 2):]

        system_msg = {
            "role": "system",
            "content": self.system_prompt or (
                "Tu es un assistant dans un système multi-agent FIPA. "
                "Réponds de façon concise (1-2 phrases max)."
            )
        }

        try:
            response = llm_client.chat.completions.create(
                model=self.model,
                messages=[system_msg] + self._history,
                max_tokens=self.max_tokens,
            )
            reply = response.choices[0].message.content.strip()
        except (APIError, APIConnectionError) as e:
            reply = f"[erreur LLM] {e}"
            # Ne pas polluer l'historique avec une erreur
            self._history.pop()
            return reply

        # Mémoriser la réponse
        self._history.append({"role": "assistant", "content": reply})
        return reply


if __name__ == "__main__":
    LLMAgent().run()
