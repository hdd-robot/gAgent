/*
 * demo_visualization.cpp — Démonstration de la visualisation web agentview
 *
 * 5 agents avec des mouvements distincts, visibles en temps réel dans
 * le navigateur.
 *
 * Lancement :
 *   Terminal 1 : ./agentplatform          (optionnel mais recommandé)
 *   Terminal 2 : ./agentview              → http://localhost:8080
 *   Terminal 3 : ./demo_visualization
 */

#include <gagent/core/AgentCore.hpp>
#include <gagent/env/Environnement.hpp>
#include <gagent/core/Agent.hpp>
#include <gagent/core/Behaviour.hpp>

#include <cmath>
#include <cstdio>
#include <iostream>
#include <thread>
#include <chrono>

using namespace gagent;

// ── Environnement ─────────────────────────────────────────────────────────────

class DemoEnv : public Environnement {
public:
    void init_env() override {
        map_width  = 600;
        map_height = 300;
        std::cout << "[Env] carte " << map_width << "x" << map_height << "\n";
    }

    void link_attribut() override {
        link_id    ("id");
        link_name  ("name");
        link_pos_x ("x");
        link_pos_y ("y");
        link_shape ("shape");
        link_color ("color");
        link_size_x("size_x");
        link_size_y("size_y");
        link_val   ("val");
    }

    // Rafraîchit list_visual_agents toutes les 100 ms
    // → agentview voit les nouvelles positions à chaque poll
    void event_loop() override {
        while (true) {
            make_agent();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
};

// ── Helpers ───────────────────────────────────────────────────────────────────

static void init_visual(Agent* ag,
                         const std::string& id,
                         const std::string& name,
                         const std::string& shape,
                         const std::string& color,
                         float sx, float sy)
{
    ag->addAttribut("id");     ag->addAttribut("name");
    ag->addAttribut("x");      ag->addAttribut("y");
    ag->addAttribut("shape");  ag->addAttribut("color");
    ag->addAttribut("size_x"); ag->addAttribut("size_y");
    ag->addAttribut("val");

    ag->setAttribut("id",    id);
    ag->setAttribut("name",  name);
    ag->setAttribut("shape", shape);
    ag->setAttribut("color", color);

    char buf[16];
    snprintf(buf, sizeof(buf), "%.0f", sx); ag->setAttribut("size_x", buf);
    snprintf(buf, sizeof(buf), "%.0f", sy); ag->setAttribut("size_y", buf);
}

static void set_pos(Agent* ag, float x, float y, const std::string& val = "")
{
    char bx[16], by[16];
    snprintf(bx, sizeof(bx), "%.1f", x);
    snprintf(by, sizeof(by), "%.1f", y);
    ag->setAttribut("x",  bx);
    ag->setAttribut("y",  by);
    if (!val.empty()) ag->setAttribut("val", val);
    ag->attributUpdated();
}

// ── Behaviours ────────────────────────────────────────────────────────────────

// Orbite autour d'un centre
class OrbitBehaviour : public TickerBehaviour {
    float t_, cx_, cy_, r_;
public:
    OrbitBehaviour(Agent* ag, float cx, float cy, float r, int ms = 60)
        : TickerBehaviour(ag, ms), t_(0), cx_(cx), cy_(cy), r_(r) {}

    void onTick() override {
        t_ += 0.06f;
        set_pos(this_agent,
                cx_ + r_ * std::cos(t_),
                cy_ + r_ * std::sin(t_));
    }
};

// Rebond horizontal
class BounceBehaviour : public TickerBehaviour {
    float x_, y_, dx_, xmin_, xmax_;
public:
    BounceBehaviour(Agent* ag, float x, float y,
                    float dx, float xmin, float xmax, int ms = 40)
        : TickerBehaviour(ag, ms),
          x_(x), y_(y), dx_(dx), xmin_(xmin), xmax_(xmax) {}

    void onTick() override {
        x_ += dx_;
        if (x_ < xmin_ || x_ > xmax_) dx_ = -dx_;
        set_pos(this_agent, x_, y_);
    }
};

// Patrouille verticale
class PatrolBehaviour : public TickerBehaviour {
    float x_, y_, dy_, ymin_, ymax_;
public:
    PatrolBehaviour(Agent* ag, float x, float y,
                    float dy, float ymin, float ymax, int ms = 50)
        : TickerBehaviour(ag, ms),
          x_(x), y_(y), dy_(dy), ymin_(ymin), ymax_(ymax) {}

    void onTick() override {
        y_ += dy_;
        if (y_ < ymin_ || y_ > ymax_) dy_ = -dy_;
        set_pos(this_agent, x_, y_);
    }
};

// Lemniscate de Bernoulli (figure en 8)
class LemniscateBehaviour : public TickerBehaviour {
    float t_, cx_, cy_, a_;
public:
    LemniscateBehaviour(Agent* ag, float cx, float cy, float a, int ms = 60)
        : TickerBehaviour(ag, ms), t_(0), cx_(cx), cy_(cy), a_(a) {}

    void onTick() override {
        t_ += 0.04f;
        float denom = 1.0f + std::sin(t_) * std::sin(t_);
        set_pos(this_agent,
                cx_ + a_ * std::cos(t_) / denom,
                cy_ + a_ * std::sin(t_) * std::cos(t_) / denom);
    }
};

// Spirale amortie (oscille vers le centre, puis repart)
class SpiralBehaviour : public TickerBehaviour {
    float t_;
    float cx_, cy_;
public:
    SpiralBehaviour(Agent* ag, float cx, float cy, int ms = 70)
        : TickerBehaviour(ag, ms), t_(0), cx_(cx), cy_(cy) {}

    void onTick() override {
        t_ += 0.05f;
        float r = 80.0f * std::fabs(std::sin(t_ * 0.3f));
        set_pos(this_agent,
                cx_ + r * std::cos(t_),
                cy_ + r * std::sin(t_));
    }
};

// ── Agents ────────────────────────────────────────────────────────────────────

// Orbiter — cercle cyan, grande orbite
class OrbiterAgent : public Agent {
public:
    void setup() override {
        init_visual(this, "orbiter", "Orbiter", "circle", "#4fc3f7", 28, 28);
        setAttribut("val", "orbit");
        attributUpdated();
        addBehaviour(new OrbitBehaviour(this, 300, 150, 110));
    }
};

// Bouncer — carré rouge, rebond horizontal rapide
class BounceAgent : public Agent {
public:
    void setup() override {
        init_visual(this, "bouncer", "Bouncer", "square", "#e94560", 26, 26);
        setAttribut("val", "bounce");
        attributUpdated();
        addBehaviour(new BounceBehaviour(this, 50, 75, 5, 20, 580));
    }
};

// Patrol — triangle vert, patrouille verticale
class PatrolAgent : public Agent {
public:
    void setup() override {
        init_visual(this, "patrol", "Patrol", "triangle", "#4caf50", 28, 28);
        setAttribut("val", "patrol");
        attributUpdated();
        addBehaviour(new PatrolBehaviour(this, 500, 30, 3, 20, 280));
    }
};

// Figure-8 — diamant orange, lemniscate au centre
class Figure8Agent : public Agent {
public:
    void setup() override {
        init_visual(this, "figure8", "Figure-8", "diamond", "#ff9800", 26, 26);
        setAttribut("val", "figure-8");
        attributUpdated();
        addBehaviour(new LemniscateBehaviour(this, 300, 225, 130));
    }
};

// Star — étoile violette, spirale autour du centre
class StarAgent : public Agent {
public:
    void setup() override {
        init_visual(this, "star", "Star", "star", "#ce93d8", 32, 32);
        setAttribut("val", "spiral");
        attributUpdated();
        addBehaviour(new SpiralBehaviour(this, 300, 150));
    }
};

// ── main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::cout << "=== Demo Visualisation gAgent ===\n"
              << "  agentview    → http://localhost:8080\n"
              << "  agentmanager list\n"
              << "  Ctrl+C pour arrêter\n\n";

    AgentCore::initAgentSystem();

    DemoEnv env;
    AgentCore::initEnvironnementSystem(env);

    OrbiterAgent a1; a1.init();
    BounceAgent  a2; a2.init();
    PatrolAgent  a3; a3.init();
    Figure8Agent a4; a4.init();
    StarAgent    a5; a5.init();

    AgentCore::syncAgentSystem();
    return 0;
}
