/*
 ╔══════════════════════════════════════════════════════════════════════════╗
 ║   FF ELEVATOR TRAINER — Arduino Uno R3                                 ║
 ║   I/O + State Machine  —  v1.0  |  May 2026                           ║
 ╠══════════════════════════════════════════════════════════════════════════╣
 ║   PIN ASSIGNMENTS                                                       ║
 ║   D0   Serial RX  ← R4 WiFi pin 1 (Serial1 TX)                        ║
 ║   D1   Serial TX  → R4 WiFi pin 0 (Serial1 RX)                        ║
 ║   D2   Floor 1 button    INPUT_PULLUP, active LOW                      ║
 ║   D3   Floor 2 button    INPUT_PULLUP, active LOW                      ║
 ║   D4   Floor 3 button    INPUT_PULLUP, active LOW                      ║
 ║   D5   Phase 1 key ON    INPUT_PULLUP, active HIGH (NC contact)        ║
 ║   D6   Phase 1 key RESET INPUT_PULLUP, active LOW                      ║
 ║   D7   Phase 2 key ON    INPUT_PULLUP, active HIGH (NC contact)        ║
 ║   D8   Phase 2 key HOLD  INPUT_PULLUP, active LOW                      ║
 ║   D9   Call Cancel       INPUT_PULLUP, active LOW                      ║
 ║   D10  A6276 LE          latch enable, active HIGH pulse               ║
 ║   D11  A6276 SIN         serial data                                   ║
 ║   D12  Hall call button  INPUT_PULLUP, active LOW                      ║
 ║   D13  A6276 CLK         serial clock                                  ║
 ║   A1   FF Helmet LED     OUTPUT via Q1 (2N2222), 470Ω base             ║
 ║   A4   Display GND sw    OUTPUT via Q2 (2N2222), 470Ω base            ║
 ╠══════════════════════════════════════════════════════════════════════════╣
 ║   KEY SWITCH WIRING                                                     ║
 ║   Phase 1: Wire 3=GND  Wire 4=D5(NC)  Wire 2=D6(RESET)               ║
 ║   Phase 2: Wire 2=GND  Wire 1+3=D7(NC paired)  Wire 4=D8(HOLD)       ║
 ╠══════════════════════════════════════════════════════════════════════════╣
 ║   SERIAL LINK  9600 baud, pins 0/1                                     ║
 ║   Disconnect from R4 before uploading to either board.                ║
 ║   TX → R4: S:n,F:n,P1:n,P2:n,FA:n\n   (on change + 1s heartbeat)    ║
 ║   RX ← R4: FA:n  |  FL:n  |  RS:1                                     ║
 ╚══════════════════════════════════════════════════════════════════════════╝
*/

// ═══════════════════════════════════════════════════════════════════════════
// PIN DEFINITIONS
// ═══════════════════════════════════════════════════════════════════════════

#define PIN_BTN_FLOOR1    2
#define PIN_BTN_FLOOR2    3
#define PIN_BTN_FLOOR3    4
#define PIN_KEY_PH1_ON    5
#define PIN_KEY_PH1_RESET 6
#define PIN_KEY_PH2_ON    7
#define PIN_KEY_PH2_HOLD  8
#define PIN_BTN_CANCEL    9
#define PIN_LATCH        10
#define PIN_BTN_HALL     12
#define PIN_FF_LED       15   // A1, Q1 base via 470Ω
#define PIN_DISPLAY_PWR  18   // A4, Q2 base via 470Ω — HIGH connects display GND

// ═══════════════════════════════════════════════════════════════════════════
// A6276 SEGMENT BIT MAP
// ═══════════════════════════════════════════════════════════════════════════

#define U4_a1  ((uint32_t)1 <<  2)
#define U4_d1  ((uint32_t)1 <<  5)
#define U4_g1  ((uint32_t)1 <<  8)
#define U4_g2  ((uint32_t)1 <<  9)
#define U4_f   ((uint32_t)1 << 10)
#define U4_e   ((uint32_t)1 << 11)
#define U4_d2  ((uint32_t)1 << 12)
#define U4_c   ((uint32_t)1 << 13)
#define U4_b   ((uint32_t)1 << 14)
#define U4_a2  ((uint32_t)1 << 15)
#define U3_a1  ((uint32_t)1 << 18)
#define U3_d1  ((uint32_t)1 << 21)
#define U3_g1  ((uint32_t)1 << 24)
#define U3_g2  ((uint32_t)1 << 25)
#define U3_f   ((uint32_t)1 << 26)
#define U3_e   ((uint32_t)1 << 27)
#define U3_d2  ((uint32_t)1 << 28)
#define U3_c   ((uint32_t)1 << 29)
#define U3_b   ((uint32_t)1 << 30)
#define U3_a2  ((uint32_t)1 << 31)

#define L_0  (U4_a1|U4_a2|U4_b|U4_c|U4_d1|U4_d2|U4_e|U4_f)
#define R_1  (U3_b|U3_c)
#define R_2  (U3_a1|U3_a2|U3_b|U3_g1|U3_g2|U3_e|U3_d1|U3_d2)
#define R_3  (U3_a1|U3_a2|U3_b|U3_c|U3_g1|U3_g2|U3_d1|U3_d2)

const uint32_t FLOOR_PAT[4] = { 0UL, L_0|R_1, L_0|R_2, L_0|R_3 };

// ═══════════════════════════════════════════════════════════════════════════
// STATE MACHINE ENUMS + TIMING
// ═══════════════════════════════════════════════════════════════════════════

enum SimState {
  STATE_IDLE, STATE_HALL_CALL, STATE_PHASE1,
  STATE_ARRIVING, STATE_LOBBY, STATE_PHASE2,
  STATE_HOLD, STATE_RESETTING
};
enum FaultMode { FAULT_NONE=0, FAULT_SLOW=1, FAULT_FAST=2, FAULT_DARK=3 };

const unsigned long TRAVEL_MS   = 4000;
const unsigned long ARRIVAL_MS  = 1500;
const unsigned long RESET_MS    =  800;
const unsigned long DEBOUNCE_MS =   50;

// ═══════════════════════════════════════════════════════════════════════════
// BUTTON DEBOUNCE
// invert=true: NC contact — pin HIGH = active (key ON)
// invert=false: NO contact — pin LOW  = active (button pressed)
// ═══════════════════════════════════════════════════════════════════════════

struct Button {
  uint8_t pin;
  bool state, lastRaw;
  unsigned long lastChange;
  bool pressed;
  bool invert;
};

Button btnFloor1  = {PIN_BTN_FLOOR1,    false, false, 0, false, false};
Button btnFloor2  = {PIN_BTN_FLOOR2,    false, false, 0, false, false};
Button btnFloor3  = {PIN_BTN_FLOOR3,    false, false, 0, false, false};
Button keyPh1On   = {PIN_KEY_PH1_ON,    false, false, 0, false, true };
Button keyPh1Rst  = {PIN_KEY_PH1_RESET, false, false, 0, false, false};
Button keyPh2On   = {PIN_KEY_PH2_ON,    false, false, 0, false, true };
Button keyPh2Hold = {PIN_KEY_PH2_HOLD,  false, false, 0, false, false};
Button btnCancel  = {PIN_BTN_CANCEL,    false, false, 0, false, false};
Button btnHall    = {PIN_BTN_HALL,      false, false, 0, false, false};

void readBtn(Button& b) {
  bool raw = b.invert ? (digitalRead(b.pin) == HIGH) : (digitalRead(b.pin) == LOW);
  b.pressed = false;
  if (raw != b.lastRaw) { b.lastChange = millis(); b.lastRaw = raw; }
  if (millis() - b.lastChange >= DEBOUNCE_MS) {
    if (b.state != raw) {
      b.state = raw;
      if (b.state) b.pressed = true;
    }
  }
}

void readAllButtons() {
  readBtn(btnFloor1); readBtn(btnFloor2); readBtn(btnFloor3);
  readBtn(keyPh1On);  readBtn(keyPh1Rst);
  readBtn(keyPh2On);  readBtn(keyPh2Hold);
  readBtn(btnCancel); readBtn(btnHall);
}

// ═══════════════════════════════════════════════════════════════════════════
// RUNTIME STATE
// ═══════════════════════════════════════════════════════════════════════════

SimState      simState        = STATE_IDLE;
FaultMode     faultMode       = FAULT_NONE;
int           currentFloor    = 3;
unsigned long stateTimer      = 0;
unsigned long resetTimer      = 0;
uint32_t      displayBits     = 0;
bool          ledOn           = false;
unsigned long lastLedToggle   = 0;
SimState      stateBeforeHold = STATE_IDLE;
int           phase2Destination = 0;  // 0 = no destination, 1-3 = traveling to that floor in Phase 2

// ═══════════════════════════════════════════════════════════════════════════
// DISPLAY
// ═══════════════════════════════════════════════════════════════════════════

void shiftOut32(uint32_t data) {
  for (int i = 31; i >= 0; i--) {
    digitalWrite(11, (data >> i) & 1);
    digitalWrite(13, HIGH); delayMicroseconds(5);
    digitalWrite(13, LOW);  delayMicroseconds(5);
  }
}

void latchDisplay() {
  digitalWrite(PIN_LATCH, LOW);  delayMicroseconds(10);
  digitalWrite(PIN_LATCH, HIGH); delayMicroseconds(100);
  digitalWrite(PIN_LATCH, LOW);  delayMicroseconds(10);
}

void writeDisplay(uint32_t bits) {
  displayBits = bits;
  shiftOut32(bits);
  latchDisplay();
}

void showFloor(int f) {
  writeDisplay(FLOOR_PAT[constrain(f, 0, 3)]);
}

// ═══════════════════════════════════════════════════════════════════════════
// FF HELMET LED
// ═══════════════════════════════════════════════════════════════════════════

void updateFFLED(bool ffActive) {
  unsigned long now = millis();
  if (!ffActive || faultMode == FAULT_DARK) {
    digitalWrite(PIN_FF_LED, LOW); ledOn = false; return;
  }
  if (faultMode == FAULT_NONE) {
    digitalWrite(PIN_FF_LED, HIGH); ledOn = true; return;
  }
  unsigned long interval = (faultMode == FAULT_SLOW) ? 500UL : 125UL;
  if (now - lastLedToggle >= interval) {
    lastLedToggle = now; ledOn = !ledOn;
    digitalWrite(PIN_FF_LED, ledOn);
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SERIAL LINK TO R4
// ═══════════════════════════════════════════════════════════════════════════

static int  lastSentState = -1;
static int  lastSentFloor = -1;
static int  lastSentFault = -1;
static bool lastSentPh1   = false;
static bool lastSentPh2   = false;
static unsigned long lastHeartbeat = 0;

void sendState() {
  // Guard: don't block if TX buffer can't fit the full line (~26 bytes)
  if (Serial.availableForWrite() < 30) return;
  Serial.print("S:");  Serial.print((int)simState);
  Serial.print(",F:"); Serial.print(currentFloor);
  Serial.print(",P1:"); Serial.print(keyPh1On.state ? 1 : 0);
  Serial.print(",P2:"); Serial.print(keyPh2On.state ? 1 : 0);
  Serial.print(",FA:"); Serial.println((int)faultMode);
  lastSentState = (int)simState;
  lastSentFloor = currentFloor;
  lastSentFault = (int)faultMode;
  lastSentPh1   = keyPh1On.state;
  lastSentPh2   = keyPh2On.state;
  lastHeartbeat = millis();
}

void enterState(SimState s);  // forward declaration

void checkSerial() {
  static String buf = "";
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n') {
      buf.trim();
      if (buf.startsWith("FA:")) {
        faultMode = (FaultMode)constrain(buf.substring(3).toInt(), 0, 3);
      } else if (buf.startsWith("FL:")) {
        int f = constrain(buf.substring(3).toInt(), 0, 3);
        currentFloor = f;
        showFloor(f);
      } else if (buf == "RS:1") {
        resetTimer = 0;
        enterState(STATE_RESETTING);
      }
      buf = "";
    } else if (c != '\r') {
      if (buf.length() < 32) buf += c;
    }
  }
}

void updateSerialLink() {
  checkSerial();
  bool changed = ((int)simState  != lastSentState ||
                  currentFloor   != lastSentFloor  ||
                  keyPh1On.state != lastSentPh1    ||
                  keyPh2On.state != lastSentPh2    ||
                  (int)faultMode != lastSentFault);
  if (changed || millis() - lastHeartbeat >= 1000UL) sendState();
}

// ═══════════════════════════════════════════════════════════════════════════
// STATE MACHINE
// ═══════════════════════════════════════════════════════════════════════════

void enterState(SimState s) {
  simState = s; stateTimer = millis();
  // Phase 2 destination is only meaningful inside PHASE2; clear it when leaving.
  if (s != STATE_PHASE2) phase2Destination = 0;
  if (s == STATE_IDLE) {
    currentFloor = 3; showFloor(3); faultMode = FAULT_NONE;
  }
}

void runStateMachine() {
  readAllButtons();
  unsigned long now = millis(), elapsed = now - stateTimer;

  if (keyPh1Rst.pressed) {
    writeDisplay(0UL); updateFFLED(false);
    resetTimer = 0; enterState(STATE_RESETTING); return;
  }
  if (keyPh2Hold.pressed &&
      (simState == STATE_PHASE1 || simState == STATE_HALL_CALL)) {
    stateBeforeHold = simState; enterState(STATE_HOLD); return;
  }
  if (!keyPh2Hold.state && simState == STATE_HOLD) {
    enterState(stateBeforeHold); return;
  }

  switch (simState) {

    case STATE_IDLE:
      updateFFLED(false);
      if (keyPh1On.state) enterState(STATE_PHASE1);
      if (btnHall.pressed) enterState(STATE_HALL_CALL);
      break;

    case STATE_HALL_CALL:
      updateFFLED(false);
      if (elapsed >= TRAVEL_MS) {
        if (currentFloor > 1) {
          currentFloor--; showFloor(currentFloor);
          enterState(STATE_ARRIVING);
        } else {
          enterState(STATE_RESETTING);
        }
      }
      break;

    case STATE_PHASE1:
      updateFFLED(true);
      if (!keyPh1On.state) { enterState(STATE_RESETTING); break; }
      if (elapsed >= TRAVEL_MS) {
        if (currentFloor > 1) {
          currentFloor--; showFloor(currentFloor);
          enterState(STATE_ARRIVING);
        } else {
          enterState(STATE_LOBBY);
        }
      }
      break;

    case STATE_ARRIVING:
      updateFFLED(keyPh1On.state);
      if (elapsed >= ARRIVAL_MS) {
        if (currentFloor <= 1)
          enterState(keyPh1On.state ? STATE_LOBBY : STATE_RESETTING);
        else
          enterState(keyPh1On.state ? STATE_PHASE1 : STATE_HALL_CALL);
      }
      break;

    case STATE_LOBBY:
      showFloor(1); updateFFLED(true);
      if (!keyPh1On.state) { enterState(STATE_RESETTING); break; }
      if (keyPh2On.state)  enterState(STATE_PHASE2);
      break;

    case STATE_PHASE2:
      updateFFLED(true);
      if (!keyPh1On.state || !keyPh2On.state) {
        enterState(STATE_RESETTING); break;
      }
      if (keyPh2Hold.state) {
        stateBeforeHold = STATE_PHASE2; enterState(STATE_HOLD); break;
      }
      if (btnCancel.pressed) {
        phase2Destination = 0; stateTimer = now;
        showFloor(currentFloor); break;
      }
      // Set destination — the actual floor change is delayed by TRAVEL_MS per
      // floor, mirroring how Phase 1 / Hall Call travel downward.
      if (btnFloor1.pressed && currentFloor != 1) { phase2Destination = 1; stateTimer = now; }
      if (btnFloor2.pressed && currentFloor != 2) { phase2Destination = 2; stateTimer = now; }
      if (btnFloor3.pressed && currentFloor != 3) { phase2Destination = 3; stateTimer = now; }
      // Step toward destination one floor per TRAVEL_MS
      if (phase2Destination != 0 && phase2Destination != currentFloor && elapsed >= TRAVEL_MS) {
        if (currentFloor < phase2Destination) currentFloor++;
        else                                   currentFloor--;
        showFloor(currentFloor);
        stateTimer = now;
        if (currentFloor == phase2Destination) phase2Destination = 0;
      }
      break;

    case STATE_HOLD:
      updateFFLED(keyPh1On.state);
      showFloor(currentFloor);
      break;

    case STATE_RESETTING:
      updateFFLED(false); writeDisplay(0UL);
      if (resetTimer == 0) resetTimer = now;
      if (now - resetTimer >= RESET_MS) {
        resetTimer = 0; enterState(STATE_IDLE);
      }
      break;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// SETUP
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
  // Q2 base HIGH — connects display board GND to system GND
  pinMode(PIN_DISPLAY_PWR, OUTPUT);
  digitalWrite(PIN_DISPLAY_PWR, HIGH);

  pinMode(PIN_LATCH, OUTPUT); digitalWrite(PIN_LATCH, LOW);
  pinMode(11, OUTPUT);        digitalWrite(11, LOW);
  pinMode(13, OUTPUT);        digitalWrite(13, LOW);

  pinMode(PIN_FF_LED, OUTPUT); digitalWrite(PIN_FF_LED, LOW);

  pinMode(PIN_BTN_FLOOR1,    INPUT_PULLUP);
  pinMode(PIN_BTN_FLOOR2,    INPUT_PULLUP);
  pinMode(PIN_BTN_FLOOR3,    INPUT_PULLUP);
  pinMode(PIN_KEY_PH1_ON,    INPUT_PULLUP);
  pinMode(PIN_KEY_PH1_RESET, INPUT_PULLUP);
  pinMode(PIN_KEY_PH2_ON,    INPUT_PULLUP);
  pinMode(PIN_KEY_PH2_HOLD,  INPUT_PULLUP);
  pinMode(PIN_BTN_CANCEL,    INPUT_PULLUP);
  pinMode(PIN_BTN_HALL,      INPUT_PULLUP);

  // Display startup test — all segments on, then floor 3
  writeDisplay(0xFFFFFFFFUL); delay(500);
  writeDisplay(0UL);          delay(200);
  showFloor(3);

  // Start serial link to R4 (pins 0/1 — disconnect before programming)
  Serial.begin(9600);
}

// ═══════════════════════════════════════════════════════════════════════════
// LOOP
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
  shiftOut32(displayBits);
  latchDisplay();
  runStateMachine();
  updateSerialLink();
}
