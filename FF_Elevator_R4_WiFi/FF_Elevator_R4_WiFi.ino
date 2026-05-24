/*
 â•”â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•—
 â•‘   FF ELEVATOR TRAINER â€” Arduino Uno R4 WiFi                            â•‘
 â•‘   WiFi AP + Instructor Web Interface  â€”  v4.0  |  May 2026            â•‘
 â• â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•£
 â•‘   ROLE: WiFi access point and web server only.                         â•‘
 â•‘   All I/O and the state machine run on the R3.                         â•‘
 â• â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•£
 â•‘   CONNECTIONS                                                           â•‘
 â•‘   D0   Serial1 RX  â† R3 pin 1 (TX)                                    â•‘
 â•‘   D1   Serial1 TX  â†’ R3 pin 0 (RX)                                    â•‘
 â•‘   D8   Phase 1 indicator LED  OUTPUT, HIGH when Phase 1 key ON        â•‘
 â•‘   Disconnect from R3 before uploading to either board.                â•‘
 â• â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•£
 â•‘   WiFi SSID: FF_Trainer   Password: ladder12                           â•‘
 â•‘   Instructor UI: http://192.168.4.1                                    â•‘
 â•šâ•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
*/

#include "WiFiS3.h"
#include "Arduino_LED_Matrix.h"
#include <EEPROM.h>

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// LED MATRIX FRAMES  (8 rows Ã— 12 cols)
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

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

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// STATE RECEIVED FROM R3
// state index: 0=IDLE 1=HALL_CALL 2=PHASE1 3=ARRIVING 4=LOBBY 5=PHASE2 6=HOLD 7=RESETTING
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

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
// WIFI + WEB SERVER GLOBALS  (declared here so doUpdateCheck can reference them)
// ═══════════════════════════════════════════════════════════════════════════

const char AP_SSID[] = "FF_Trainer";
const char AP_PASS[] = "ladder12";
WiFiServer server(80);

// ═══════════════════════════════════════════════════════════════════════════
// WIFI CREDENTIALS (EEPROM) + OTA UPDATE STATE
// ═══════════════════════════════════════════════════════════════════════════

#define CRED_MAGIC 0xA5
#define CRED_ADDR  0          // layout: [magic][ssid 33B][pass 64B]
struct WiFiCreds { uint8_t magic; char ssid[33]; char pass[64]; };

String storedSsid = "";
String pendingUpdateResult = "";

void loadCredentials() {
  WiFiCreds c;
  EEPROM.get(CRED_ADDR, c);
  if (c.magic == CRED_MAGIC) { storedSsid = String(c.ssid); }
}

void saveCredentials(const String& ssid, const String& pass) {
  WiFiCreds c;
  c.magic = CRED_MAGIC;
  ssid.toCharArray(c.ssid, sizeof(c.ssid));
  pass.toCharArray(c.pass, sizeof(c.pass));
  EEPROM.put(CRED_ADDR, c);
  storedSsid = ssid;
}

String checkRemoteVersion() {
  WiFiSSLClient client;
  if (!client.connect("raw.githubusercontent.com", 443)) return "Could not reach update server.";
  client.println("GET /mbombich-robotics/Elevator-Simulator/main/version.txt HTTP/1.0");
  client.println("Host: raw.githubusercontent.com");
  client.println("Connection: close");
  client.println();
  unsigned long t = millis();
  while (!client.available() && millis() - t < 8000) {}
  String resp = "";
  while (client.connected() || client.available()) {
    if (client.available()) resp += (char)client.read();
    if (resp.length() > 400) break;
  }
  client.stop();
  int bodyIdx = resp.indexOf("\r\n\r\n");
  if (bodyIdx < 0) return "Bad response from server.";
  String ver = resp.substring(bodyIdx + 4);
  ver.trim();
  return ver;
}

void doUpdateCheck(const String& ssid, const String& pass) {
  saveCredentials(ssid, pass);
  pendingUpdateResult = "Connecting to " + ssid + "...";

  server.end();              // cleanly stop server before dropping the AP
  WiFi.end(); delay(2000);
  WiFi.begin(ssid.c_str(), pass.c_str());
  unsigned long t = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t < 15000) delay(200);

  if (WiFi.status() != WL_CONNECTED) {
    pendingUpdateResult = "Could not connect to " + ssid + ". Check SSID and password.";
  } else {
    String remoteVer = checkRemoteVersion();
    pendingUpdateResult = remoteVer.length() > 0
      ? "Remote version: " + remoteVer + " — flash via Arduino IDE if newer than installed sketch."
      : "Connected but could not read version file.";
  }

  // Restart as AP — longer delays to let the ESP32 co-processor reinitialize
  WiFi.end(); delay(2000);
  WiFi.beginAP(AP_SSID, AP_PASS);
  delay(6000);
  server.begin();
}

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// SERIAL LINK FROM R3
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

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
      case 0:  /* IDLE â€” silent on reset */ break;
      case 1:  // don't re-announce when returning from ARRIVING (state 3)
        if (oldState != 3) { pendingAudio = "Hall call. Elevator traveling."; audioSeq++; }
        break;
      case 2:  // don't re-announce when returning from ARRIVING (state 3)
        if (oldState != 3) { pendingAudio = "Phase one. Firefighter service."; audioSeq++; }
        break;
      case 3:
        if (newFloor > 1) { pendingAudio = "Floor " + String(newFloor) + "."; audioSeq++; }
        break;
      case 4:  pendingAudio = "Lobby. Doors opening."; audioSeq++; break;
      case 5:  // don't re-announce when resuming from HOLD (6) or returning from ARRIVING (3)
        if (oldState != 6 && oldState != 3) { pendingAudio = "Firefighter operation. Select destination floor."; audioSeq++; }
        break;
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

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// MATRIX UPDATE
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

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

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// WIFI + WEB SERVER
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

String urlDecode(const String& s) {
  String result = "";
  for (int i = 0; i < (int)s.length(); i++) {
    if (s[i] == '+') { result += ' '; }
    else if (s[i] == '%' && i + 2 < (int)s.length()) {
      char h1 = s[i+1], h2 = s[i+2];
      int val = 0;
      if      (h1 >= '0' && h1 <= '9') val = (h1-'0')    << 4;
      else if (h1 >= 'A' && h1 <= 'F') val = (h1-'A'+10) << 4;
      else if (h1 >= 'a' && h1 <= 'f') val = (h1-'a'+10) << 4;
      if      (h2 >= '0' && h2 <= '9') val |= (h2-'0');
      else if (h2 >= 'A' && h2 <= 'F') val |= (h2-'A'+10);
      else if (h2 >= 'a' && h2 <= 'f') val |= (h2-'a'+10);
      result += (char)val; i += 2;
    } else { result += s[i]; }
  }
  return result;
}

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

#include "html.h"
#include "docs.h"

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
               "\",\"audSeq\":\"" + String(audioSeq) +
               "\",\"ssid\":\""   + storedSsid + "\"}";
    pendingAudio = "";  // clear after serving
    wClient.print("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n");
    wClient.print(j);
    wClient.stop();
    return;
  }

  if (getLine.indexOf("/docs") >= 0) {
    wClient.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
    wClient.print(DOCS_HTML);
    wClient.stop(); return;
  }

  if (getLine.indexOf("/update-check") >= 0) {
    // Read POST body (body arrives after headers)
    unsigned long bt = millis();
    while (wClient.connected() && millis() - bt < 200) {
      if (wClient.available()) req += (char)wClient.read();
    }
    int bodyStart = req.indexOf("\r\n\r\n");
    String body = (bodyStart >= 0) ? req.substring(bodyStart + 4) : "";
    String ssid = urlDecode(getParam(body, "ssid"));
    String pass = urlDecode(getParam(body, "pass"));
    wClient.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n");
    wClient.print("ok");
    wClient.stop();
    if (ssid.length() > 0) doUpdateCheck(ssid, pass);
    return;
  }

  if (getLine.indexOf("/update-status") >= 0) {
    String result = pendingUpdateResult.length() > 0
      ? pendingUpdateResult : "No update check performed yet.";
    wClient.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\n\r\n");
    wClient.print(result);
    wClient.stop(); return;
  }

  wClient.print("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n");
  wClient.print(HTML);
  delay(5);
  wClient.stop();
}

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// SETUP
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

void setup() {
  Serial.begin(115200);  // USB debug

  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);

  matrix.begin();
  showMatrix(MTX_BLANK);

  Serial1.begin(9600);  // R3 link â€” disconnect before programming
  loadCredentials();

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

// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•
// LOOP
// â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•â•

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
