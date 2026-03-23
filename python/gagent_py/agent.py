"""
gagent_py.agent — Classe de base pour les agents Python gAgent

Protocole stdin/stdout (newline-delimited JSON) :

  C++ → Python :
    {"event":"start"}
    {"event":"message","msg":{"performative":"request","sender":"alice","content":"...",...}}
    {"event":"tick"}
    {"event":"stop"}

  Python → C++ :
    {"action":"noop"}
    {"action":"send","to":"bob","msg":{"performative":"inform","content":"..."}}
    {"action":"delete"}
    {"action":"suspend"}
    {"action":"wake"}
"""

from __future__ import annotations

import json
import sys
from typing import Optional


class ACLMessage:
    """Représentation d'un message FIPA ACL reçu depuis le C++."""

    def __init__(self, data: dict):
        self.performative    = data.get("performative", "not-understood")
        self.sender          = data.get("sender", "")
        self.content         = data.get("content", "")
        self.ontology        = data.get("ontology", "")
        self.language        = data.get("language", "")
        self.conversation_id = data.get("conversation_id", "")
        self.reply_with      = data.get("reply_with", "")
        self.in_reply_to     = data.get("in_reply_to", "")
        self.protocol        = data.get("protocol", "")

    def __repr__(self) -> str:
        return (f"ACLMessage(performative={self.performative!r}, "
                f"sender={self.sender!r}, content={self.content!r})")


class Agent:
    """
    Classe de base pour un agent Python gAgent.

    Surcharger les méthodes on_start, on_message, on_tick, on_stop.
    Retourner une action (dict) ou None (= noop).

    Exemple minimal :

        class MonAgent(gagent_py.Agent):
            def on_message(self, msg):
                return self.send(msg.sender, "inform", "réponse : " + msg.content)

        MonAgent().run()
    """

    # ── Actions helpers ──────────────────────────────────────────────

    def send(self, to: str, performative: str, content: str, **kwargs) -> dict:
        """Envoyer un message ACL à un autre agent."""
        msg: dict = {"performative": performative, "content": content}
        msg.update(kwargs)
        return {"action": "send", "to": to, "msg": msg}

    def reply(self, original: ACLMessage, performative: str,
              content: str, **kwargs) -> dict:
        """Répondre à un message en conservant le conversation_id."""
        return self.send(
            original.sender, performative, content,
            conversation_id=original.conversation_id,
            in_reply_to=original.reply_with,
            **kwargs,
        )

    def delete(self) -> dict:
        """Demander la suppression de l'agent."""
        return {"action": "delete"}

    def noop(self) -> dict:
        """Ne rien faire."""
        return {"action": "noop"}

    def suspend(self) -> dict:
        """Suspendre l'agent."""
        return {"action": "suspend"}

    def wake(self) -> dict:
        """Réveiller l'agent."""
        return {"action": "wake"}

    # ── À surcharger ─────────────────────────────────────────────────

    def on_start(self) -> Optional[dict]:
        """Appelé une fois au démarrage."""
        return None

    def on_message(self, msg: ACLMessage) -> Optional[dict]:
        """Appelé à chaque message ACL reçu."""
        return None

    def on_tick(self) -> Optional[dict]:
        """Appelé périodiquement quand aucun message n'arrive."""
        return None

    def on_stop(self) -> None:
        """Appelé avant l'arrêt."""

    # ── Boucle principale ─────────────────────────────────────────────

    def run(self) -> None:
        """Démarre la boucle événementielle (bloquant)."""
        for raw_line in sys.stdin:
            line = raw_line.strip()
            if not line:
                continue

            try:
                event = json.loads(line)
            except json.JSONDecodeError as e:
                self._respond({"action": "noop"})
                continue

            evt = event.get("event", "")
            action: Optional[dict] = None

            if evt == "start":
                action = self.on_start()
            elif evt == "message":
                msg = ACLMessage(event.get("msg", {}))
                action = self.on_message(msg)
            elif evt == "tick":
                action = self.on_tick()
            elif evt == "stop":
                self.on_stop()
                break

            self._respond(action if action is not None else {"action": "noop"})

    def _respond(self, action: dict) -> None:
        print(json.dumps(action), flush=True)
