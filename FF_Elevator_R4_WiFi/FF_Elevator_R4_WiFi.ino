/*
 ╔══════════════════════════════════════════════════════════════════════════╗
 ║   FF ELEVATOR TRAINER — Arduino Uno R4 WiFi                            ║
 ║   WiFi AP + Instructor Web Interface  —  v4.0  |  May 2026            ║
 ╠══════════════════════════════════════════════════════════════════════════╣
 ║   ROLE: WiFi access point and web server only.                         ║
 ║   All I/O and the state machine run on the R3.                         ║
 ╠══════════════════════════════════════════════════════════════════════════╣
 ║   CONNECTIONS                                                           ║
 ║   D0   Serial1 RX  ← R3 pin 1 (TX)                                    ║
 ║   D1   Serial1 TX  → R3 pin 0 (RX)                                    ║
 ║   D8   Phase 1 indicator LED  OUTPUT, HIGH when Phase 1 key ON        ║
 ║   Disconnect from R3 before uploading to either board.                ║
 ╠══════════════════════════════════════════════════════════════════════════╣
 ║   WiFi SSID: FF_Trainer   Password: ladder12                           ║
 ║   Instructor UI: http://192.168.4.1                                    ║
 ╚══════════════════════════════════════════════════════════════════════════╝
*/

#include "WiFiS3.h"
#include "Arduino_LED_Matrix.h"

// ═══════════════════════════════════════════════════════════════════════════
// LED MATRIX FRAMES  (8 rows × 12 cols)
// ═══════════════════════════════════════════════════════════════════════════

ArduinoLEDMatrix matrix;

uint8_t MTX_FLOOR3[8][12] = {
  { 0,0,1,1,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,0,0,0,1,1,0,0,0 },
  { 0,0,0,0,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,0,0,0,1,1,0,0,0 },
  { 0,0,0,0,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
};
uint8_t MTX_FLOOR2[8][12] = {
  { 0,0,1,1,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,0,0,0,1,1,0,0,0 },
  { 0,0,0,0,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,1,1,1,1,0,0,0,0 },
  { 0,0,1,1,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
};
uint8_t MTX_FLOOR1[8][12] = {
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
};
uint8_t MTX_DOWN[8][12] = {
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
  { 0,0,0,1,1,1,1,1,0,0,0,0 },
  { 0,0,0,0,1,1,1,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
};
uint8_t MTX_FF[8][12] = {
  { 0,0,0,1,1,1,1,1,1,0,0,0 },
  { 0,0,1,1,0,0,0,0,1,1,0,0 },
  { 0,1,1,0,0,0,0,0,0,1,1,0 },
  { 0,1,1,0,0,0,0,0,0,1,1,0 },
  { 1,1,1,1,1,1,1,1,1,1,1,1 },
  { 0,0,1,1,1,1,1,1,1,1,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
};
uint8_t MTX_HOLD[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,1,1,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,0,0,0,1,1,0,0,0 },
  { 0,0,1,1,0,0,0,1,1,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 },
};
uint8_t MTX_WARN[8][12] = {
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
  { 0,0,0,0,1,1,1,0,0,0,0,0 },
  { 0,0,0,1,1,0,1,1,0,0,0,0 },
  { 0,0,1,1,0,1,0,1,1,0,0,0 },
  { 0,0,1,1,0,1,0,1,1,0,0,0 },
  { 0,0,0,1,1,0,0,1,1,0,0,0 },
  { 0,0,0,0,1,1,1,0,0,0,0,0 },
  { 0,0,0,0,0,1,0,0,0,0,0,0 },
};
uint8_t MTX_BLANK[8][12] = {
  {0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0},
};

void showMatrix(uint8_t frame[8][12]) { matrix.loadPixels(&frame[0][0], 96); }
void showFloorMatrix(int f) {
  switch (constrain(f, 0, 3)) {
    case 1: showMatrix(MTX_FLOOR1); break;
    case 2: showMatrix(MTX_FLOOR2); break;
    case 3: showMatrix(MTX_FLOOR3); break;
    default: showMatrix(MTX_BLANK); break;
  }
}

bool matrixFlashState = false;
unsigned long lastMatrixFlash = 0;

// ═══════════════════════════════════════════════════════════════════════════
// STATE RECEIVED FROM R3
// state index: 0=IDLE 1=HALL_CALL 2=PHASE1 3=ARRIVING 4=LOBBY 5=PHASE2 6=HOLD 7=RESETTING
// ═══════════════════════════════════════════════════════════════════════════

int  r3State    = 0;
int  r3Floor    = 3;
bool r3Ph1      = false;
bool r3Ph2      = false;
int  r3Fault    = 0;
unsigned long lastR3Rx = 0;

// audio cue: set on state/floor transitions, cleared after one /status serve
String pendingAudio = "";
int    audioSeq     = 0;

// ═══════════════════════════════════════════════════════════════════════════
// SERIAL LINK FROM R3
// ═══════════════════════════════════════════════════════════════════════════

int extractField(const String& s, const char* key) {
  String k = String(key) + ":";
  int idx = s.indexOf(k);
  if (idx < 0) return -1;
  return s.substring(idx + k.length()).toInt();
}

void computeAudioCue(int newState, int newFloor, int oldState, int oldFloor) {
  if (newState == oldState && newFloor == oldFloor) return;

  if (newState != oldState) {
    switch (newState) {
      case 0:  /* IDLE — silent on reset */ break;
      case 1:  pendingAudio = "Hall call. Elevator traveling."; audioSeq++; break;
      case 2:  pendingAudio = "Phase one. Firefighter service."; audioSeq++; break;
      case 3:
        if (newFloor > 1) { pendingAudio = "Floor " + String(newFloor) + "."; audioSeq++; }
        break;
      case 4:  pendingAudio = "Lobby. Doors opening."; audioSeq++; break;
      case 5:  pendingAudio = "Firefighter operation. Select destination floor."; audioSeq++; break;
      case 6:  pendingAudio = "Hold."; audioSeq++; break;
      default: break;
    }
  } else if (newState == 5 && newFloor != oldFloor) {
    // Floor change in Phase 2
    String dir = (newFloor < oldFloor) ? "down" : "up";
    pendingAudio = "Going " + dir + " to floor " + String(newFloor) + ".";
    audioSeq++;
  }
}

void parseSerialFromR3() {
  static String buf = "";
  static int prevState = -1;
  static int prevFloor = 3;

  while (Serial1.available()) {
    char c = (char)Serial1.read();
    if (c == '\n') {
      buf.trim();
      if (buf.length() > 0 && buf.startsWith("S:")) {
        int s  = extractField(buf, "S");
        int f  = extractField(buf, "F");
        int p1 = extractField(buf, "P1");
        int p2 = extractField(buf, "P2");
        int fa = extractField(buf, "FA");
        if (s >= 0) {
          int ps = (prevState < 0) ? s : prevState;
          computeAudioCue(s, constrain(f,0,3), ps, prevFloor);
          prevState = r3State;
          prevFloor = r3Floor;
          r3State = s;
          r3Floor = constrain(f, 0, 3);
          r3Ph1   = (p1 == 1);
          r3Ph2   = (p2 == 1);
          r3Fault = constrain(fa, 0, 3);
          lastR3Rx = millis();
        }
      }
      buf = "";
    } else if (c != '\r') {
      if (buf.length() < 64) buf += c;
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// MATRIX UPDATE
// ═══════════════════════════════════════════════════════════════════════════

void updateMatrix() {
  unsigned long now = millis();
  bool ffActive = (r3State == 2 || r3State == 3 || r3State == 4 || r3State == 5);
  if (r3Fault != 0 && r3Fault != 3 && ffActive) {
    unsigned long interval = (r3Fault == 1) ? 500UL : 125UL;
    if (now - lastMatrixFlash >= interval) {
      lastMatrixFlash = now;
      matrixFlashState = !matrixFlashState;
      if (matrixFlashState) showMatrix(MTX_WARN);
      else showFloorMatrix(r3Floor);
    }
    return;
  }
  switch (r3State) {
    case 0: showFloorMatrix(r3Floor); break;  // IDLE
    case 1:                                    // HALL_CALL
    case 2: showMatrix(MTX_DOWN);     break;  // PHASE1
    case 3: showFloorMatrix(r3Floor); break;  // ARRIVING
    case 4: showMatrix(MTX_FLOOR1);   break;  // LOBBY
    case 5: showMatrix(MTX_FF);       break;  // PHASE2
    case 6: showMatrix(MTX_HOLD);     break;  // HOLD
    case 7: showMatrix(MTX_BLANK);    break;  // RESETTING
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// WIFI + WEB SERVER
// ═══════════════════════════════════════════════════════════════════════════

const char AP_SSID[] = "FF_Trainer";
const char AP_PASS[] = "ladder12";
WiFiServer server(80);

String getParam(const String& line, const char* key) {
  String search = String(key) + "=";
  int idx = line.indexOf(search);
  if (idx < 0) return "";
  int start = idx + search.length();
  int end = start;
  while (end < (int)line.length() &&
         line[end] != '&' && line[end] != ' ' && line[end] != '\r') end++;
  return line.substring(start, end);
}

const char HTML[] = R"HTML(
<!DOCTYPE html><html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>FF Elevator Trainer</title>
<style>
:root{--red:#E02020;--red-dk:#8B0000;--amber:#FF8C00;--green:#1DB954;--bg:#0A0A0A;--panel:#111;--border:#2A2A2A;--text:#CCC;--dim:#555}
*{box-sizing:border-box;margin:0;padding:0}
body{background:var(--bg);color:var(--text);font-family:Arial,sans-serif;padding-bottom:40px}
.hdr{background:var(--red-dk);border-bottom:3px solid var(--red);padding:14px 20px;display:flex;align-items:center;gap:14px}
.hdr-icon{font-size:2em}.hdr-title{font-weight:800;font-size:1.3em;letter-spacing:.08em;text-transform:uppercase;color:#fff}
.hdr-sub{font-family:monospace;font-size:.7em;color:rgba(255,255,255,.5);letter-spacing:.15em;margin-top:2px}
.floor-display{text-align:center;padding:20px;background:#0D0D0D;border-bottom:1px solid var(--border)}
.floor-label{font-family:monospace;font-size:.65em;color:var(--dim);letter-spacing:.25em;text-transform:uppercase;margin-bottom:6px}
.floor-number{font-family:monospace;font-size:5em;color:var(--red);text-shadow:0 0 30px rgba(224,32,32,.4);line-height:1}
.status-panel{background:var(--panel);border-bottom:1px solid var(--border);padding:14px 20px;display:grid;grid-template-columns:1fr 1fr;gap:10px}
.status-item{display:flex;flex-direction:column;gap:2px}
.status-label{font-family:monospace;font-size:.6em;color:var(--dim);letter-spacing:.2em;text-transform:uppercase}
.status-value{font-family:monospace;font-size:1.1em;color:#fff}
.status-value.red{color:var(--red)}.status-value.green{color:var(--green)}.status-value.amber{color:var(--amber)}
.section{margin:14px 14px 0;background:var(--panel);border:1px solid var(--border);border-radius:4px;overflow:hidden}
.section-title{font-weight:600;font-size:.7em;letter-spacing:.2em;text-transform:uppercase;color:var(--dim);padding:8px 14px;border-bottom:1px solid var(--border);background:#0D0D0D}
.section-body{padding:12px}
.btn-grid{display:grid;gap:8px}.g2{grid-template-columns:1fr 1fr}.g4{grid-template-columns:repeat(4,1fr)}
.btn{display:block;width:100%;padding:12px 8px;border:1.5px solid;border-radius:3px;font-weight:600;font-size:.85em;letter-spacing:.06em;text-transform:uppercase;text-align:center;text-decoration:none;cursor:pointer;background:transparent;transition:all .1s;font-family:Arial,sans-serif}
.btn:active{transform:scale(.97)}
.r{border-color:var(--red);color:var(--red)}.r:hover{background:rgba(224,32,32,.2)}
.a{border-color:var(--amber);color:var(--amber)}.a:hover{background:rgba(255,140,0,.2)}
.g{border-color:var(--green);color:var(--green)}.g:hover{background:rgba(29,185,84,.2)}
.x{border-color:var(--dim);color:var(--dim)}.x:hover{background:rgba(85,85,85,.25);color:var(--text)}
.b{border-color:#1E90FF;color:#1E90FF}.b:hover{background:rgba(30,144,255,.2)}
.arm-btn{width:100%;padding:14px;border:1.5px solid var(--red);border-radius:3px;background:rgba(224,32,32,.12);color:var(--red);font-weight:700;font-size:.9em;letter-spacing:.1em;text-transform:uppercase;cursor:pointer;font-family:Arial,sans-serif;transition:all .2s}
.arm-btn.armed{border-color:var(--green);background:rgba(29,185,84,.12);color:var(--green)}
.fault-strip{display:flex;gap:6px;padding:10px 14px;border-top:1px solid var(--border);background:#0D0D0D;align-items:center}
.fault-label{font-family:monospace;font-size:.6em;color:var(--dim);letter-spacing:.15em;text-transform:uppercase;flex:1}
.fault-dot{width:10px;height:10px;border-radius:50%;background:var(--dim)}
.fault-dot.a0{background:var(--green);box-shadow:0 0 6px var(--green)}
.fault-dot.a1{background:var(--amber);box-shadow:0 0 6px var(--amber)}
.fault-dot.a2{background:var(--red);box-shadow:0 0 6px var(--red)}
.reset-btn{display:block;margin:14px;padding:14px;background:rgba(139,0,0,.2);border:1.5px solid var(--red-dk);border-radius:4px;color:rgba(255,255,255,.6);font-weight:600;font-size:.85em;letter-spacing:.1em;text-transform:uppercase;text-align:center;text-decoration:none}
.reset-btn:hover{background:rgba(139,0,0,.4);color:#fff}
.conn-bar{display:flex;align-items:center;gap:8px;padding:6px 20px;background:#0A0A0A;border-bottom:1px solid var(--border);font-family:monospace;font-size:.6em;color:var(--dim);letter-spacing:.15em}
.conn-dot{width:6px;height:6px;border-radius:50%}
.online{background:var(--green);box-shadow:0 0 4px var(--green);animation:pulse 2s infinite}
.offline{background:var(--red);box-shadow:0 0 4px var(--red)}
@keyframes pulse{0%,100%{opacity:1}50%{opacity:.3}}
</style></head><body>
<div class="hdr"><div class="hdr-icon">&#x1F692;</div><div>
<div class="hdr-title">FF Elevator Trainer</div>
<div class="hdr-sub">INSTRUCTOR CONTROL INTERFACE</div>
</div></div>
<div class="conn-bar">
<div class="conn-dot online" id="wifiDot"></div><span id="wifiLbl">WIFI CONNECTED</span>
&nbsp;&middot;&nbsp;
<div class="conn-dot" id="r3Dot"></div><span id="r3Lbl">R3 —</span>
</div>
<div class="floor-display">
<div class="floor-label">Current Floor</div>
<div class="floor-number" id="fl">—</div>
</div>
<div class="status-panel">
<div class="status-item"><div class="status-label">State</div><div class="status-value" id="st">—</div></div>
<div class="status-item"><div class="status-label">Fault</div><div class="status-value" id="fa">—</div></div>
<div class="status-item"><div class="status-label">Phase 1 Key</div><div class="status-value" id="p1">—</div></div>
<div class="status-item"><div class="status-label">Phase 2 Key</div><div class="status-value" id="p2">—</div></div>
</div>

<div class="section"><div class="section-title">Voice Announcements</div>
<div class="section-body">
<button class="arm-btn" id="armBtn" onclick="armAudio()">&#x1F507; Tap to Arm Audio</button>
<div class="btn-grid g2" style="margin-top:10px">
<button class="btn x" onclick="speak('Going down.')">&#x2B07; Going Down</button>
<button class="btn x" onclick="speak('Going up.')">&#x2B06; Going Up</button>
<button class="btn x" onclick="speak('Lobby. Doors opening.')">&#x1F6AA; Lobby</button>
<button class="btn x" onclick="speak('Firefighter service. Select floor.')">&#x1F692; FF Service</button>
<button class="btn x" onclick="speak('Floor one.')">Floor 1</button>
<button class="btn x" onclick="speak('Floor two.')">Floor 2</button>
<button class="btn x" onclick="speak('Floor three.')">Floor 3</button>
<button class="btn r" onclick="speechSynthesis.cancel()">&#x23F9; Stop</button>
</div></div></div>

<div class="section"><div class="section-title">Fault Injection</div>
<div class="section-body"><div class="btn-grid g2">
<a class="btn g" href="/cmd?fault=0">&#x2705; Normal</a>
<a class="btn a" href="/cmd?fault=1">&#x26A0; Slow Fault</a>
<a class="btn r" href="/cmd?fault=2">&#x1F6A8; Critical</a>
<a class="btn x" href="/cmd?fault=3">&#x26AB; Comms Loss</a>
</div></div>
<div class="fault-strip"><span class="fault-label">Active</span>
<div class="fault-dot" id="fd0"></div><div class="fault-dot" id="fd1"></div>
<div class="fault-dot" id="fd2"></div><div class="fault-dot" id="fd3"></div>
</div></div>

<div class="section"><div class="section-title">Display Override</div>
<div class="section-body"><div class="btn-grid g4">
<a class="btn b" href="/cmd?floor=3">03</a>
<a class="btn b" href="/cmd?floor=2">02</a>
<a class="btn b" href="/cmd?floor=1">01</a>
<a class="btn x" href="/cmd?floor=0">—</a>
</div></div></div>

<a class="reset-btn" href="/cmd?reset=1">&#x21BA; &nbsp; Force Reset to Idle</a>

<script>
const STATES=["IDLE","HALL CALL","PHASE 1","ARRIVING","LOBBY","PHASE 2 — FF OP","HOLD","RESETTING"];
const FAULTS=["None","Slow 1Hz","Critical 4Hz","Comms Loss"];
const FCOL=["green","amber","red",""];
let lastAudSeq=-1, audioArmed=false, audioCtx=null;
let prevPollState=-1;

function ensureAudioCtx(){
  if(!audioCtx) audioCtx=new(window.AudioContext||window.webkitAudioContext)();
  if(audioCtx.state==='suspended') audioCtx.resume();
  return audioCtx;
}

// Volume increases as elevator approaches floor 1
const ARRIVAL_VOL={1:1.0, 2:0.45, 3:0.15};

function playArrivalTone(floor){
  if(!audioArmed) return;
  const ctx=ensureAudioCtx();
  const vol=ARRIVAL_VOL[floor]||0.3;
  const osc=ctx.createOscillator();
  const gain=ctx.createGain();
  osc.connect(gain); gain.connect(ctx.destination);
  osc.type='sine'; osc.frequency.value=560;
  gain.gain.setValueAtTime(0, ctx.currentTime);
  gain.gain.linearRampToValueAtTime(vol, ctx.currentTime+0.05);
  gain.gain.exponentialRampToValueAtTime(0.001, ctx.currentTime+2.0);
  osc.start(); osc.stop(ctx.currentTime+2.0);
}

function armAudio(){
  ensureAudioCtx();
  const u=new SpeechSynthesisUtterance("Audio armed.");
  u.rate=0.9; speechSynthesis.speak(u);
  audioArmed=true;
  try{ localStorage.setItem('elevatorAudioArmed','1'); }catch(e){}
  const b=document.getElementById("armBtn");
  b.className="arm-btn armed"; b.innerHTML="&#x1F508; Audio Armed";
}

// Browsers require a user gesture to unlock AudioContext, but localStorage
// remembers the user's preference across refreshes. If audio was armed before,
// silently re-arm on the very next tap anywhere on the page — no need to hunt
// for the dedicated Arm button.
function setupAutoArm(){
  let wasArmed=false;
  try{ wasArmed = localStorage.getItem('elevatorAudioArmed')==='1'; }catch(e){}
  if(!wasArmed) return;
  const b=document.getElementById("armBtn");
  if(b){ b.innerHTML="&#x1F50A; Audio &mdash; tap anywhere to enable"; }
  const silentArm=()=>{
    if(audioArmed) return;
    ensureAudioCtx();
    audioArmed=true;
    if(b){ b.className="arm-btn armed"; b.innerHTML="&#x1F508; Audio Armed"; }
  };
  ['pointerdown','touchstart','keydown'].forEach(ev=>{
    document.addEventListener(ev, silentArm, {once:true, capture:true, passive:true});
  });
}
setupAutoArm();

function speak(text){
  if(!audioArmed) return;
  speechSynthesis.cancel();
  const u=new SpeechSynthesisUtterance(text);
  u.rate=0.9; speechSynthesis.speak(u);
}

function refresh(){
  fetch("/status").then(r=>r.json()).then(d=>{
    const st=parseInt(d.state), fl=parseInt(d.floor);
    document.getElementById("fl").textContent=d.floor=="0"?"—":"0"+d.floor;
    document.getElementById("st").textContent=STATES[st]||d.state;
    document.getElementById("p1").textContent=d.ph1;
    document.getElementById("p1").className="status-value "+(d.ph1=="ON"?"red":"");
    document.getElementById("p2").textContent=d.ph2;
    document.getElementById("p2").className="status-value "+(d.ph2=="ON"?"red":"");
    const fi=parseInt(d.fault);
    document.getElementById("fa").textContent=FAULTS[fi]||d.fault;
    document.getElementById("fa").className="status-value "+(FCOL[fi]||"");
    for(let i=0;i<4;i++)document.getElementById("fd"+i).className="fault-dot"+(i===fi?" a"+i:"");
    const r3ok=parseInt(d.r3age)<4000;
    document.getElementById("r3Dot").className="conn-dot "+(r3ok?"online":"offline");
    document.getElementById("r3Lbl").textContent="R3 "+(r3ok?"LINKED":"OFFLINE");
    if(prevPollState>=0 && st!==prevPollState){
      if(st===3) playArrivalTone(fl);  // ARRIVING — tone scales with floor
      if(st===4) playArrivalTone(1);  // LOBBY — always loudest
    }
    prevPollState=st;
    const aseq=parseInt(d.audSeq||0);
    if(aseq!==lastAudSeq&&d.aud){lastAudSeq=aseq; speak(d.aud);}
  }).catch(()=>{});
}
setInterval(refresh,1000); refresh();
</script></body></html>
)HTML";

void handleWebServer() {
  WiFiClient wClient = server.available();
  if (!wClient) return;

  String req = "";
  unsigned long t = millis();
  while (wClient.connected() && millis() - t < 100) {
    if (wClient.available()) {
      req += (char)wClient.read();
      if (req.endsWith("\r\n\r\n")) break;
    }
  }
  if (req.length() == 0) { wClient.stop(); return; }

  String getLine = "";
  int lineEnd = req.indexOf("\r\n");
  if (lineEnd > 0) getLine = req.substring(0, lineEnd);

  if (getLine.indexOf("/cmd") >= 0) {
    String fault = getParam(getLine, "fault");
    String floor = getParam(getLine, "floor");
    String reset = getParam(getLine, "reset");

    if (fault.length() > 0) {
      int f = constrain(fault.toInt(), 0, 3);
      r3Fault = f;  // optimistic update; R3 confirms on next packet
      Serial1.print("FA:"); Serial1.println(f);
    }
    if (floor.length() > 0) {
      int f = constrain(floor.toInt(), 0, 3);
      r3Floor = f;
      Serial1.print("FL:"); Serial1.println(f);
    }
    if (reset == "1") {
      Serial1.println("RS:1");
    }
    wClient.print("HTTP/1.1 302 Found\r\nLocation: /\r\nConnection: close\r\n\r\n");
    wClient.stop();
    return;
  }

  if (getLine.indexOf("/status") >= 0) {
    unsigned long age = millis() - lastR3Rx;
    String j = "{\"state\":\"" + String(r3State) +
               "\",\"floor\":\"" + String(r3Floor) +
               "\",\"fault\":\"" + String(r3Fault) +
               "\",\"ph1\":\""   + (r3Ph1 ? "ON" : "OFF") +
               "\",\"ph2\":\""   + (r3Ph2 ? "ON" : "OFF") +
               "\",\"r3age\":\""  + String(age) +
               "\",\"aud\":\""   + pendingAudio +
               "\",\"audSeq\":\"" + String(audioSeq) + "\"}";
    pendingAudio = "";  // clear after serving
    wClient.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
    wClient.print(j);
    wClient.stop();
    return;
  }

  wClient.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
  wClient.print(HTML);
  delay(5);
  wClient.stop();
}

// ═══════════════════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);  // USB debug

  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  matrix.begin();
  showMatrix(MTX_BLANK);

  Serial1.begin(9600);  // R3 link — disconnect before programming

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("WiFi module missing!");
    while (true);
  }
  WiFi.end();
  delay(500);
  WiFi.beginAP(AP_SSID, AP_PASS);
  delay(3000);
  server.begin();
  Serial.print("SSID: "); Serial.println(WiFi.SSID());
  Serial.print("IP:   "); Serial.println(WiFi.localIP());

  showFloorMatrix(3);
  Serial.println("Ready. Waiting for R3...");
}

// ═══════════════════════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
  parseSerialFromR3();
  digitalWrite(8, r3Ph1 ? HIGH : LOW);
  handleWebServer();
  updateMatrix();

  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 2000) {
    lastDebug = millis();
    Serial.print("R3 state:"); Serial.print(r3State);
    Serial.print(" floor:");   Serial.print(r3Floor);
    Serial.print(" fault:");   Serial.print(r3Fault);
    Serial.print(" age:");     Serial.println(millis() - lastR3Rx);
  }
}
