/*
 * view/main.cpp — Serveur HTTP minimal pour la visualisation gAgent
 *
 * Usage :
 *   agentview [port]          (défaut : 8080)
 *
 * Variables d'environnement :
 *   GAGENT_VIEW_PORT          surcharge le port HTTP
 *   GAGENT_ENV_SOCK           socket Environnement  (défaut /tmp/gagent_env.sock)
 *   GAGENT_AMS_SOCK           socket AMS            (défaut /tmp/gagent_ams.sock)
 *
 * API :
 *   GET /           → page HTML (visualisation SVG + JS)
 *   GET /api/agents → JSON Environnement (list_visual_agents)
 *   GET /api/ams    → JSON liste AMS
 *   GET /api/nsap   → JSON pile NSAP
 */

#include <gagent/platform/EnvClient.hpp>
#include <gagent/platform/AMSClient.hpp>
#include <gagent/platform/PlatformConfig.hpp>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <cstring>
#include <cstdlib>

using namespace gagent::platform;

// ── Embedded HTML page ────────────────────────────────────────────────────────

static const char HTML_PAGE[] = R"HTML(<!DOCTYPE html>
<html lang="fr">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>gAgent — Visualisation</title>
<style>
  * { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: 'Segoe UI', sans-serif; background: #1a1a2e; color: #e0e0e0; }
  header { background: #16213e; padding: 12px 20px; display: flex; align-items: center; gap: 16px; border-bottom: 1px solid #0f3460; }
  header h1 { font-size: 1.2rem; color: #e94560; letter-spacing: 1px; }
  header span { font-size: 0.85rem; color: #888; }
  #status { margin-left: auto; font-size: 0.8rem; padding: 3px 10px; border-radius: 12px; background: #0f3460; }
  #status.ok   { color: #4caf50; }
  #status.err  { color: #e94560; }
  main { display: flex; gap: 12px; padding: 12px; height: calc(100vh - 50px); }
  #canvas-wrap { flex: 1; position: relative; background: #0d0d1a; border: 1px solid #0f3460; border-radius: 6px; overflow: hidden; }
  svg#world { width: 100%; height: 100%; }
  #sidebar { width: 260px; display: flex; flex-direction: column; gap: 8px; overflow-y: auto; }
  .card { background: #16213e; border: 1px solid #0f3460; border-radius: 6px; padding: 10px; }
  .card h2 { font-size: 0.8rem; text-transform: uppercase; letter-spacing: 1px; color: #888; margin-bottom: 8px; }
  #agent-list { font-size: 0.78rem; line-height: 1.6; }
  #agent-list .agent-row { display: flex; justify-content: space-between; padding: 2px 0; border-bottom: 1px solid #0f3460; }
  #agent-list .agent-row:last-child { border-bottom: none; }
  .dot { display: inline-block; width: 8px; height: 8px; border-radius: 50%; margin-right: 6px; }
  #ams-list { font-size: 0.78rem; line-height: 1.6; }
  .state-active    { color: #4caf50; }
  .state-suspended { color: #ff9800; }
  .state-waiting   { color: #2196f3; }
  .state-deleted   { color: #e94560; }
  #refresh-rate { font-size: 0.75rem; color: #888; }
  .slider-row { display: flex; align-items: center; gap: 8px; margin-top: 4px; }
  input[type=range] { flex: 1; }
</style>
</head>
<body>
<header>
  <h1>gAgent Visualization</h1>
  <span id="agent-count">0 agents</span>
  <span id="status" class="err">connecting…</span>
</header>
<main>
  <div id="canvas-wrap">
    <svg id="world" viewBox="0 0 600 300" preserveAspectRatio="xMidYMid meet"></svg>
  </div>
  <div id="sidebar">
    <div class="card">
      <h2>Agents visuels</h2>
      <div id="agent-list"><em style="color:#666">–</em></div>
    </div>
    <div class="card">
      <h2>AMS</h2>
      <div id="ams-list"><em style="color:#666">–</em></div>
    </div>
    <div class="card">
      <h2>Rafraîchissement</h2>
      <div class="slider-row">
        <input type="range" id="rate-slider" min="100" max="5000" step="100" value="500">
        <span id="refresh-rate">500 ms</span>
      </div>
    </div>
  </div>
</main>
<script>
// ── Rendering helpers ─────────────────────────────────────────────────────────

function agentShape(a) {
  const cx = a.x, cy = a.y;
  const rx = (a.size_x || a.size || 10), ry = (a.size_y || a.size || 10);
  const color = a.color || '#4fc3f7';
  const title = `<title>${a.name || a.id} — ${a.val || ''}</title>`;

  let el;
  switch (a.shape) {
    case 'circle':
      el = `<circle cx="${cx}" cy="${cy}" r="${Math.max(rx,ry)/2}" fill="${color}">${title}</circle>`;
      break;
    case 'square':
      el = `<rect x="${cx-rx/2}" y="${cy-ry/2}" width="${rx}" height="${ry}" fill="${color}">${title}</rect>`;
      break;
    case 'triangle': {
      const pts = `${cx},${cy-ry/2} ${cx+rx/2},${cy+ry/2} ${cx-rx/2},${cy+ry/2}`;
      el = `<polygon points="${pts}" fill="${color}">${title}</polygon>`;
      break;
    }
    case 'diamond': {
      const pts = `${cx},${cy-ry/2} ${cx+rx/2},${cy} ${cx},${cy+ry/2} ${cx-rx/2},${cy}`;
      el = `<polygon points="${pts}" fill="${color}">${title}</polygon>`;
      break;
    }
    case 'hexagon': {
      const pts = hex_pts(cx, cy, Math.max(rx,ry)/2);
      el = `<polygon points="${pts}" fill="${color}">${title}</polygon>`;
      break;
    }
    case 'star': {
      const pts = star_pts(cx, cy, Math.max(rx,ry)/2, Math.max(rx,ry)/4);
      el = `<polygon points="${pts}" fill="${color}">${title}</polygon>`;
      break;
    }
    default:
      if (a.shape && a.shape.startsWith('M')) {
        el = `<path d="${a.shape}" transform="translate(${cx},${cy})" fill="${color}">${title}</path>`;
      } else {
        el = `<circle cx="${cx}" cy="${cy}" r="${Math.max(rx,ry)/2}" fill="${color}">${title}</circle>`;
      }
  }
  return el;
}

function hex_pts(cx, cy, r) {
  let pts = [];
  for (let i = 0; i < 6; i++) {
    const a = Math.PI / 180 * (60 * i - 30);
    pts.push(`${cx + r*Math.cos(a)},${cy + r*Math.sin(a)}`);
  }
  return pts.join(' ');
}

function star_pts(cx, cy, outer, inner) {
  let pts = [];
  for (let i = 0; i < 10; i++) {
    const r = i % 2 === 0 ? outer : inner;
    const a = Math.PI / 180 * (36 * i - 90);
    pts.push(`${cx + r*Math.cos(a)},${cy + r*Math.sin(a)}`);
  }
  return pts.join(' ');
}

// ── State ─────────────────────────────────────────────────────────────────────

let interval = null;
let refreshMs = 500;

const svg    = document.getElementById('world');
const status = document.getElementById('status');

// ── Fetch & render ────────────────────────────────────────────────────────────

async function refresh() {
  try {
    const [agRes, amsRes] = await Promise.all([
      fetch('/api/agents'),
      fetch('/api/ams')
    ]);

    if (!agRes.ok) throw new Error('env error');
    const agData = await agRes.json();

    // Update SVG viewBox to match env dimensions
    svg.setAttribute('viewBox', `0 0 ${agData.width} ${agData.height}`);

    // Render agents
    const agents = agData.agents || [];
    svg.innerHTML = agents.map(agentShape).join('');

    // Sidebar agent list
    document.getElementById('agent-count').textContent = `${agents.length} agent${agents.length !== 1 ? 's' : ''}`;
    const agList = agents.map(a => {
      const dot = `<span class="dot" style="background:${a.color||'#4fc3f7'}"></span>`;
      return `<div class="agent-row"><span>${dot}${a.name||a.id}</span><span style="color:#888">${a.val||''}</span></div>`;
    }).join('') || '<em style="color:#666">–</em>';
    document.getElementById('agent-list').innerHTML = agList;

    status.textContent = 'live';
    status.className = 'ok';
  } catch {
    status.textContent = 'env offline';
    status.className = 'err';
    svg.innerHTML = '';
    document.getElementById('agent-list').innerHTML = '<em style="color:#666">–</em>';
  }

  // AMS (non-blocking — no throw if absent)
  try {
    const amsRes = await fetch('/api/ams');
    if (amsRes.ok) {
      const amsData = await amsRes.json();
      const rows = (amsData.agents || []).map(a => {
        const sc = `state-${a.state}`;
        return `<div class="agent-row"><span>${a.name}</span><span class="${sc}">${a.state}</span></div>`;
      }).join('') || '<em style="color:#666">–</em>';
      document.getElementById('ams-list').innerHTML = rows;
    }
  } catch { /* AMS offline is non-fatal */ }
}

function setRefresh(ms) {
  refreshMs = ms;
  document.getElementById('refresh-rate').textContent = ms + ' ms';
  if (interval) clearInterval(interval);
  interval = setInterval(refresh, ms);
}

document.getElementById('rate-slider').addEventListener('input', e => {
  setRefresh(parseInt(e.target.value));
});

// Start
setRefresh(refreshMs);
refresh();
</script>
</body>
</html>
)HTML";

// ── HTTP helpers ──────────────────────────────────────────────────────────────

static void send_response(int fd,
                           int code,
                           const std::string& content_type,
                           const std::string& body)
{
    std::string status_line = (code == 200) ? "200 OK" :
                              (code == 404) ? "404 Not Found" :
                                              "500 Internal Server Error";
    std::ostringstream hdr;
    hdr << "HTTP/1.1 " << status_line << "\r\n"
        << "Content-Type: " << content_type << "\r\n"
        << "Content-Length: " << body.size() << "\r\n"
        << "Access-Control-Allow-Origin: *\r\n"
        << "Connection: close\r\n"
        << "\r\n";
    std::string h = hdr.str();
    ::write(fd, h.c_str(), h.size());
    ::write(fd, body.c_str(), body.size());
}

static std::string read_request_line(int fd)
{
    std::string line;
    char c;
    while (::read(fd, &c, 1) == 1) {
        if (c == '\n') break;
        if (c != '\r') line += c;
    }
    return line;
}

// Drain remaining HTTP headers
static void drain_headers(int fd)
{
    std::string line;
    char c;
    int crlf = 0;
    while (::read(fd, &c, 1) == 1) {
        if (c == '\r') continue;
        if (c == '\n') { if (++crlf == 2) break; }
        else           { crlf = 0; }
    }
}

// ── AMS list → JSON ───────────────────────────────────────────────────────────

static std::string ams_to_json()
{
    AMSClient ams;
    auto agents = ams.list();
    std::ostringstream js;
    js << "{\"agents\":[";
    bool first = true;
    for (auto& a : agents) {
        if (!first) js << ",";
        first = false;
        js << "{\"name\":\"" << a.name << "\","
           << "\"pid\":"     << a.pid  << ","
           << "\"address\":\"" << a.address << "\","
           << "\"state\":\"" << a.state << "\"}";
    }
    js << "]}";
    return js.str();
}

// ── Per-client handler ────────────────────────────────────────────────────────

static void handle_client(int fd)
{
    std::string req = read_request_line(fd);
    drain_headers(fd);

    // Parse method + path
    std::string method, path;
    {
        std::istringstream ss(req);
        ss >> method >> path;
    }

    if (method != "GET") {
        send_response(fd, 404, "text/plain", "Not Found");
        ::close(fd);
        return;
    }

    if (path == "/" || path == "/index.html") {
        send_response(fd, 200, "text/html; charset=utf-8", HTML_PAGE);
    }
    else if (path == "/api/agents") {
        EnvClient env;
        std::string json = env.getAgents();
        if (json.empty()) json = "{\"error\":\"env offline\",\"agents\":[],\"width\":600,\"height\":300}";
        send_response(fd, 200, "application/json", json);
    }
    else if (path == "/api/ams") {
        send_response(fd, 200, "application/json", ams_to_json());
    }
    else if (path == "/api/nsap") {
        EnvClient env;
        std::string json = env.getNsap();
        if (json.empty()) json = "{\"error\":\"env offline\",\"count\":0,\"snaps\":[]}";
        send_response(fd, 200, "application/json", json);
    }
    else {
        send_response(fd, 404, "text/plain", "Not Found");
    }

    ::close(fd);
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);

    int port = 8080;
    if (const char* env = std::getenv("GAGENT_VIEW_PORT"))
        port = std::atoi(env);
    if (argc >= 2)
        port = std::atoi(argv[1]);

    int srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }

    int opt = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(static_cast<uint16_t>(port));

    if (::bind(srv, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind"); return 1;
    }
    ::listen(srv, 32);

    std::cout << "[agentview] Listening on http://0.0.0.0:" << port << "\n";
    std::cout << "  env socket : " << gagent::platform::env_socket_path() << "\n";
    std::cout << "  ams socket : " << gagent::platform::ams_socket_path() << "\n";

    while (true) {
        int cli = ::accept(srv, nullptr, nullptr);
        if (cli < 0) continue;
        std::thread([cli]{ handle_client(cli); }).detach();
    }
}
