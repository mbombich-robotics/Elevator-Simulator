// Condensed design & wiring reference served at /docs (offline-capable)
const char DOCS_HTML[] = R"HTML(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>FF Elevator Trainer — Reference</title>
<style>
:root{--red:#E02020;--red-dk:#8B0000;--amber:#FF8C00;--green:#1DB954;--bg:#0A0A0A;--panel:#111;--border:#2A2A2A;--text:#CCC;--dim:#555}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:Arial,sans-serif;padding:0 0 40px}
.hdr{background:var(--red-dk);border-bottom:3px solid var(--red);padding:14px 20px}
.hdr-title{font-weight:800;font-size:1.2em;letter-spacing:.08em;text-transform:uppercase;color:#fff}
.hdr-sub{font-family:monospace;font-size:.65em;color:rgba(255,255,255,.5);letter-spacing:.15em;margin-top:3px}
.back{display:inline-block;margin:14px 20px 0;font-family:monospace;font-size:.7em;color:var(--dim);text-decoration:none;border-bottom:1px solid var(--border)}
.back:hover{color:var(--text)}
h2{font-size:.75em;letter-spacing:.2em;text-transform:uppercase;color:var(--dim);padding:18px 20px 6px;border-bottom:1px solid var(--border)}
h3{font-size:.8em;font-weight:700;color:var(--amber);margin:14px 20px 6px}
p,li{font-size:.85em;line-height:1.6;color:var(--text);padding:0 20px;margin-bottom:6px}
ul{padding:0 20px 0 40px;margin-bottom:8px}
table{width:calc(100% - 40px);margin:8px 20px;border-collapse:collapse;font-size:.78em}
th{background:#1a1a1a;color:var(--dim);text-align:left;padding:6px 10px;border-bottom:2px solid var(--border);font-size:.75em;letter-spacing:.1em;text-transform:uppercase}
td{padding:6px 10px;border-bottom:1px solid var(--border);font-family:monospace;color:var(--text)}
td:first-child{color:var(--amber)}
.state-badge{display:inline-block;padding:2px 6px;border-radius:2px;font-size:.75em;font-family:monospace}
.s-idle{border:1px solid var(--dim);color:var(--dim)}
.s-ph1{border:1px solid var(--amber);color:var(--amber)}
.s-ph2{border:1px solid var(--red);color:var(--red)}
.s-ok{border:1px solid var(--green);color:var(--green)}
code{font-family:monospace;background:#1a1a1a;padding:1px 4px;border-radius:2px;font-size:.9em}
</style></head><body>
<div class="hdr">
  <div class="hdr-title">&#x1F692; FF Elevator Trainer — Reference</div>
  <div class="hdr-sub">DESIGN BRIEF &amp; WIRING QUICK-REFERENCE &nbsp;&middot;&nbsp; v4.01</div>
</div>
<a class="back" href="/">&#x2190; Back to Instructor Interface</a>

<h2>Overview</h2>
<p>A firefighter elevator training simulator built from a salvaged Otis COP (Car Operating Panel).
Simulates ASME A17.1 Phase 1 and Phase 2 firefighter emergency operation for training and
evaluation. Developed in collaboration with the <strong>Gary Sinise Foundation</strong> and the
Vicksburg High School Applied Engineering &amp; Robotics class.</p>

<h2>Two-Board Architecture</h2>
<table>
<tr><th>Board</th><th>Role</th><th>Key I/O</th></tr>
<tr><td>Uno R3 (ATmega328P)</td><td>State machine, all panel I/O, LED driver</td><td>SPI → A6276ELW, keys, buttons, Serial1 TX</td></tr>
<tr><td>Uno R4 WiFi (RA4M1)</td><td>WiFi AP, instructor web interface, LED matrix</td><td>Serial1 RX, D8 Phase 1 LED, 192.168.4.1</td></tr>
</table>
<p>The two boards communicate via a 9600-baud serial link (R3 D1→R4 D0, R3 D0←R4 D1).
Disconnect the link before uploading to either board.</p>

<h2>R3 — I/O Pin Assignments</h2>
<table>
<tr><th>Pin</th><th>Function</th></tr>
<tr><td>D0 / D1</td><td>Serial RX/TX link to R4 WiFi</td></tr>
<tr><td>D3</td><td>SPI MOSI → A6276ELW LED driver</td></tr>
<tr><td>D4</td><td>SPI CLK → A6276ELW</td></tr>
<tr><td>D5</td><td>A6276ELW Latch (STROBE)</td></tr>
<tr><td>D6</td><td>A6276ELW Output Enable (active LOW)</td></tr>
<tr><td>D7</td><td>Phase 2 key switch input (INPUT_PULLUP)</td></tr>
<tr><td>D8</td><td>Phase 1 key switch input (INPUT_PULLUP)</td></tr>
<tr><td>D9</td><td>Floor 1 button (INPUT_PULLUP)</td></tr>
<tr><td>D10</td><td>Floor 2 button (INPUT_PULLUP)</td></tr>
<tr><td>D11</td><td>Floor 3 button (INPUT_PULLUP)</td></tr>
<tr><td>D12</td><td>Door Open button (INPUT_PULLUP)</td></tr>
<tr><td>D13</td><td>Recall button / recall station input (INPUT_PULLUP)</td></tr>
</table>

<h2>R4 WiFi — I/O Pin Assignments</h2>
<table>
<tr><th>Pin</th><th>Function</th></tr>
<tr><td>D0 / D1</td><td>Serial1 RX/TX link to R3</td></tr>
<tr><td>D8</td><td>Phase 1 indicator LED output (HIGH when Phase 1 key ON)</td></tr>
<tr><td>Built-in</td><td>12×8 LED matrix — floor/state display</td></tr>
</table>

<h2>Serial Protocol (R3 → R4)</h2>
<p>R3 sends a status packet on every state/floor change and once per second as a heartbeat:</p>
<p><code>S:&lt;state&gt;,F:&lt;floor&gt;,P1:&lt;0|1&gt;,P2:&lt;0|1&gt;,FA:&lt;0-3&gt;\n</code></p>
<p>R4 sends commands to R3:</p>
<table>
<tr><th>Command</th><th>Effect</th></tr>
<tr><td><code>FA:0</code>–<code>FA:3</code></td><td>Set fault injection level (0=none, 1=slow, 2=critical, 3=comms loss)</td></tr>
<tr><td><code>FL:1</code>–<code>FL:3</code></td><td>Override displayed floor</td></tr>
<tr><td><code>RS:1</code></td><td>Force reset to IDLE</td></tr>
</table>

<h2>State Machine</h2>
<table>
<tr><th>#</th><th>State</th><th>Description</th></tr>
<tr><td>0</td><td><span class="state-badge s-idle">IDLE</span></td><td>Normal service, waiting for Phase 1 key</td></tr>
<tr><td>1</td><td><span class="state-badge s-ph1">HALL CALL</span></td><td>Phase 1 key ON — elevator recalled to lobby</td></tr>
<tr><td>2</td><td><span class="state-badge s-ph1">PHASE 1</span></td><td>Car traveling to lobby floor</td></tr>
<tr><td>3</td><td><span class="state-badge s-idle">ARRIVING</span></td><td>Car arriving at intermediate floor (announcement)</td></tr>
<tr><td>4</td><td><span class="state-badge s-ok">LOBBY</span></td><td>Car at lobby, doors open</td></tr>
<tr><td>5</td><td><span class="state-badge s-ph2">PHASE 2 — FF OP</span></td><td>Firefighter control — floor buttons active</td></tr>
<tr><td>6</td><td><span class="state-badge s-idle">HOLD</span></td><td>Door-open button held</td></tr>
<tr><td>7</td><td><span class="state-badge s-idle">RESETTING</span></td><td>Brief reset sequence in progress</td></tr>
</table>

<h2>Timing Constants (R3)</h2>
<table>
<tr><th>Constant</th><th>Value</th><th>Purpose</th></tr>
<tr><td>TRAVEL_MS</td><td>4000 ms</td><td>Time to travel one floor</td></tr>
<tr><td>ARRIVAL_MS</td><td>1500 ms</td><td>Arrival announcement dwell</td></tr>
<tr><td>RESET_MS</td><td>800 ms</td><td>Reset sequence duration</td></tr>
</table>

<h2>WiFi Access Point</h2>
<table>
<tr><th>Setting</th><th>Value</th></tr>
<tr><td>SSID</td><td>FF_Trainer</td></tr>
<tr><td>Password</td><td>ladder12</td></tr>
<tr><td>Instructor UI</td><td>http://192.168.4.1</td></tr>
</table>

<h2>Software Update</h2>
<p>The device operates as a WiFi access point and has no internet connection by default. To check for sketch updates:</p>
<ul>
<li>Use the <strong>Software Update</strong> section on the instructor interface main page</li>
<li>Enter the building WiFi credentials — the device will temporarily connect, check the version on GitHub, then return to AP mode</li>
<li>If a newer version is available, download the updated sketch and flash via Arduino IDE (USB connection required)</li>
<li>Current sketch version: <strong>v4.01</strong></li>
</ul>

<div style="text-align:center;padding:24px 20px 8px;font-family:monospace;font-size:.6em;color:var(--dim);letter-spacing:.12em">
GARY SINISE FOUNDATION &nbsp;&middot;&nbsp; VICKSBURG HS APPLIED ENGINEERING &amp; ROBOTICS &nbsp;&middot;&nbsp; 2026
</div>
</body></html>
)HTML";
