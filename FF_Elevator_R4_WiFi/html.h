// Instructor web interface — kept in a separate header so the Arduino
// preprocessor does not mistake JavaScript 'function' keywords for C++.
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
<div class="hdr"><div class="hdr-icon">&#x1F692;</div><div style="flex:1">
<div class="hdr-title">FF Elevator Trainer</div>
<div class="hdr-sub">INSTRUCTOR CONTROL INTERFACE</div>
</div><div id="fwVer" style="font-family:monospace;font-size:.75em;color:rgba(255,255,255,.45);letter-spacing:.1em;align-self:center;white-space:nowrap"></div></div>
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
<button class="arm-btn" id="armBtn" onclick="armAudio()">&#x1F507; Tap to Arm Audio</button></div></div>

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

<div class="section"><div class="section-title">Software Update</div>
<div class="section-body" style="font-size:.85em">
<p style="color:var(--dim);font-size:.85em;margin-bottom:10px">Connect to building WiFi to check for and apply firmware updates. The device reconnects to FF_Trainer automatically.</p>
<div style="display:flex;flex-direction:column;gap:8px">
<input id="wifiSsid" type="text" placeholder="Network Name (SSID)"
  style="background:#1a1a1a;border:1px solid var(--border);color:var(--text);padding:10px;border-radius:3px;font-size:.9em;font-family:Arial,sans-serif">
<input id="wifiPass" type="password" placeholder="Password"
  style="background:#1a1a1a;border:1px solid var(--border);color:var(--text);padding:10px;border-radius:3px;font-size:.9em;font-family:Arial,sans-serif">
<button id="checkBtn" class="btn x" onclick="checkUpdates()" style="padding:12px">&#x1F4F6; &nbsp; Check for Updates</button>
<button id="applyBtn" class="btn g" onclick="applyUpdate()" style="padding:12px;display:none">&#x2B06; &nbsp; Apply Update</button>
</div>
<div id="updateStatus" style="margin-top:10px;font-family:monospace;font-size:.75em;min-height:16px"></div>
</div></div>

<div style="text-align:center;padding:18px 20px 8px;font-family:monospace;font-size:.6em;color:var(--dim);letter-spacing:.12em">
<a href="/docs"
   style="color:var(--dim);text-decoration:none;border-bottom:1px solid var(--border);padding-bottom:1px">
&#x1F4CB; &nbsp; DESIGN BRIEF &amp; WIRING REFERENCE
</a><br><br>
GARY SINISE FOUNDATION &nbsp;&middot;&nbsp; VICKSBURG HS APPLIED ENGINEERING &amp; ROBOTICS &nbsp;&middot;&nbsp; 2026
</div>

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

const ARRIVAL_VOL={1:1.0, 2:0.225, 3:0.15};

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

function setupAutoArm(){
  let wasArmed=false;
  try{ wasArmed = localStorage.getItem('elevatorAudioArmed')==='1'; }catch(e){}
  if(!wasArmed) return;
  const b=document.getElementById("armBtn");
  if(b){ b.innerHTML="&#x1F50A; Audio — tap anywhere to enable"; }
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
      if(st===3) playArrivalTone(fl);
      if(st===4) playArrivalTone(1);
    }
    prevPollState=st;
    const aseq=parseInt(d.audSeq||0);
    if(aseq!==lastAudSeq&&d.aud){lastAudSeq=aseq; speak(d.aud);}
    const ssidFld=document.getElementById('wifiSsid');
    if(d.ssid&&ssidFld&&ssidFld.value==='')ssidFld.value=d.ssid;
  }).catch(()=>{});
}
setInterval(refresh,1000); refresh();

function setUpdateStatus(msg, col){
  const el=document.getElementById('updateStatus');
  const colors={amber:'var(--amber)',green:'var(--green)',red:'var(--red)',dim:'var(--dim)'};
  el.style.color=colors[col]||colors.amber;
  el.innerHTML=msg;
}

function checkUpdates(){
  const ssid=document.getElementById('wifiSsid').value.trim();
  const pass=document.getElementById('wifiPass').value;
  if(!ssid){setUpdateStatus('Enter network name.','amber');return;}
  document.getElementById('checkBtn').disabled=true;
  document.getElementById('applyBtn').style.display='none';
  setUpdateStatus('Connecting to <b>'+ssid+'</b>… Reconnects in ~30s.','amber');
  const body='ssid='+encodeURIComponent(ssid)+'&pass='+encodeURIComponent(pass);
  fetch('/update-check',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body}).catch(()=>{});
  setTimeout(pollVersionResult, 32000);
}

function pollVersionResult(){
  fetch('/update-status').then(r=>r.json()).then(d=>{
    document.getElementById('checkBtn').disabled=false;
    if(d.otaResult){
      const ok=d.otaResult.indexOf('OK')>=0;
      setUpdateStatus(d.otaResult, ok?'green':'red');
      return;
    }
    if(d.checkResult){
      if(d.updateAvail){
        setUpdateStatus('Update available: v'+d.remoteVer+' (running v'+d.localVer+') — downloading…','amber');
        applyUpdate();
      } else if(d.remoteVer){
        setUpdateStatus('✓ Up to date — v'+d.localVer,'green');
      } else {
        setUpdateStatus(d.checkResult,'amber');
      }
    }
  }).catch(()=>{
    setUpdateStatus('Still reconnecting — please wait…','amber');
    setTimeout(pollVersionResult, 6000);
  });
}

function applyUpdate(){
  document.getElementById('applyBtn').style.display='none';
  document.getElementById('checkBtn').disabled=true;
  setUpdateStatus('Downloading firmware… Board will reboot. Reconnect to FF_Trainer when ready (~2 min).','amber');
  fetch('/apply-update',{method:'POST'}).catch(()=>{});
  setTimeout(pollOTAResult, 100000);
}

function pollOTAResult(){
  fetch('/update-status').then(r=>r.json()).then(d=>{
    document.getElementById('checkBtn').disabled=false;
    if(d.otaResult){
      const ok=d.otaResult.indexOf('OK')>=0;
      setUpdateStatus(d.otaResult, ok?'green':'red');
    } else if(d.checkResult&&d.checkResult.indexOf('OTA')>=0){
      const fail=d.checkResult.indexOf('failed')>=0;
      setUpdateStatus(d.checkResult, fail?'red':'amber');
    } else {
      setUpdateStatus('✓ Board rebooted — run version check to confirm.','green');
    }
  }).catch(()=>{
    setUpdateStatus('Board still rebooting — please wait…','amber');
    setTimeout(pollOTAResult, 15000);
  });
}

fetch('/update-status').then(r=>r.json()).then(d=>{
  const verEl=document.getElementById('fwVer');
  if(verEl&&d.localVer)verEl.textContent='v'+d.localVer;
  if(d.otaResult){
    const ok=d.otaResult.indexOf('OK')>=0;
    setUpdateStatus(d.otaResult, ok?'green':'red');
  }
}).catch(()=>{});
</script></body></html>
)HTML";
