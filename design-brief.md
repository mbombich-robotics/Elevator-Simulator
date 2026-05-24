# Firefighter Elevator Training Simulator
## Design Brief & Bill of Materials

**Based on Salvaged Otis 3-Stop Car Operating Panel**
Control Platform: Arduino Uno R4 WiFi + DFPlayer Mini
Version 2.3 | May 2026

---

## Collaboration

This project is a collaboration between the **Gary Sinise Foundation** and the **Vicksburg High School Applied Engineering & Robotics Class**. Students designed and built the simulator as a hands-on engineering capstone, repurposing salvaged elevator hardware to support firefighter training programs served by the Foundation.

---

## 1. Project Overview

This project converts a decommissioned Otis elevator car operating panel (COP) into a fully functional firefighter training simulator. The unit is a 3-stop panel originally installed in a commercial building. All elevator controller wiring has been severed. The panel is driven by a single Arduino Uno R4 WiFi handling all hardware I/O, WiFi instructor interface, and state machine. A DFPlayer Mini module provides MP3 audio playback, driving the original 4Ω panel speaker directly from its built-in 3W amplifier output. Power is supplied by a single USB-C connection from a battery bank — no mains wiring is required inside the project enclosure.

The simulator allows fire service trainees to practice Phase 1 Recall and Phase 2 Firefighter Operation procedures using the original key switches, buttons, and indicator lights from the real elevator panel. An instructor web interface served over WiFi enables scenario injection including fault conditions, all accessible from a phone or tablet.

### 1.1 Training Objectives

- Trainee activates Phase 1 Recall using the original fire recall key switch
- Floor display updates realistically as elevator "travels" from floor 3 to floor 1
- Audible chime sounds at each floor arrival — more beeps closer to ground floor
- Firefighter mode indicator lights activate on display board and COP panel
- Instructor injects fault conditions remotely via WiFi web interface
- Trainee uses Phase 2 key and floor buttons to operate elevator in FF mode

### 1.2 Salvaged Hardware Inventory

- Otis COP panel face with floor buttons (3, 2, ★1), key switches, door controls
- Display board AAA26800AGP — two A6276ELW 32-channel LED drivers (U3, U4)
- 4Ω speaker and piezo buzzer
- Firefighter indicator LED on Phase 1 recall switch trim
- Phase 1 Recall key switch and Phase 2 Fire Operation key switch

---

## 2. System Architecture

The original RSL (Remote Serial Link) communication bus is bypassed entirely. The Arduino Uno R4 WiFi replaces the elevator controller — driving the A6276ELW LED driver chips via hardware SPI, monitoring all physical inputs via GPIO, serving the instructor web interface directly over built-in WiFi, and controlling the DFPlayer Mini for realistic audio announcements through the panel speaker. Power is supplied by a 5V 2A USB wall adapter. The Uno's 5V GPIO outputs drive the A6276ELW well above its VIH minimum of 2.0V — no level shifter required. The Uno 5V pin also supplies the display board VCC rail directly.

### 2.1 Block Diagram

```
INPUTS                          ARDUINO UNO R4 WIFI              OUTPUTS
Phase 1 Key                     Renesas RA4M1 48MHz
Phase 2 Key        ──────►      Built-in ESP32-S3 WiFi   ──────► A6276 Display (SPI0)
Floor Buttons 1/2/3             State Machine                     FF Indicator LED
WiFi Web UI                     Display + I/O            ──────► Speaker / Chime

                   ◄──────      POWER: 5V 2A USB-C Wall Adapter (no mains wiring)
```

### 2.2 GPIO Pin Assignment

| Arduino Pin | Signal | Function | Notes |
|---|---|---|---|
| D0 | DFP_RX | DFPlayer TX→Uno | Serial1 RX — no conflicts with upload on R4 |
| D1 | DFP_TX | DFPlayer RX←Uno | Serial1 TX via 1kΩ resistor |
| D2 | BTN_FLOOR1 | Floor 1 button | INPUT_PULLUP, active LOW |
| D3 | BTN_FLOOR2 | Floor 2 button | INPUT_PULLUP, active LOW |
| D4 | BTN_FLOOR3 | Floor 3 button | INPUT_PULLUP, active LOW |
| D5 | KEY_PH1_ON | Phase 1 key ON | INPUT_PULLUP, active LOW |
| D6 | KEY_PH1_RESET | Phase 1 key RESET | INPUT_PULLUP, active LOW |
| D7 | KEY_PH2_ON | Phase 2 key ON | INPUT_PULLUP, active LOW |
| D8 | KEY_PH2_HOLD | Phase 2 key HOLD | INPUT_PULLUP, active LOW |
| D9 | BTN_CANCEL | Call Cancel button | INPUT_PULLUP, active LOW |
| D10 | A6276_LATCH | SPI latch strobe | Rising edge transfers shift reg → outputs |
| D11 | A6276_SIN | SPI data | Write-only |
| D12 | BTN_HALL | Hall call button | INPUT_PULLUP, active LOW |
| D13 | A6276_CLK | SPI clock | |
| A0 | FF_HELMET_LED | FF Helmet indicator | OUTPUT — 2N2222 transistor, 470Ω base, 220Ω anode |
| A1–A5 | (spare) | Available | Reserved for future expansion |

> **Note:** D0/D1 carry DFPlayer serial (Serial1). The Uno R4 WiFi uploads via a dedicated USB peripheral separate from Serial1, so DFPlayer wiring does not interfere with sketch uploads.

---

## 3. Operational Flowchart

The simulator state machine follows standard ASME A17.1 firefighter operation logic. Instructor fault injection is available at any point via WiFi. The state machine runs non-blocking using `millis()` throughout.

```
POWER ON
  Startup chime, display shows Floor 3
  ▼
IDLE
  Display: 3 | FF Light: OFF
  (fault LED flash active if injected by instructor)
  ▼
Phase 1 Recall Key → ON
PHASE 1 — RECALL ACTIVE
  FF Light: Steady ON | Display: 3 | Timer: 4 sec/floor
  ▼  4 seconds elapsed
FLOOR 2 ARRIVAL
  Chime: 2 beeps (medium volume) | Display: 2
  ▼  1.5 sec dwell
CONTINUING TOWARD LOBBY
  ▼  4 seconds elapsed
FLOOR 1 — LOBBY ARRIVAL
  Chime: 3 beeps (loudest) | Display: 1 | Awaiting Phase 2
  ▼
Phase 2 FF Op Key → ON
PHASE 2 — FF OPERATION
  Trainee selects floor with buttons
  Display updates on each press
  ▼  Either key → OFF
RESET
  Display blanks 0.8 sec → Idle
```

### 3.1 Instructor Fault Injection Modes

| Mode | FF Light Behavior | Training Purpose |
|---|---|---|
| Normal | Steady ON when FF active | Standard firefighter operation drill |
| Slow Fault | 1 Hz flash | Non-critical fault — trainee must decide to abort or continue |
| Critical Fault | 4 Hz fast flash | Critical malfunction / abort scenario training |
| Comms Loss | LED OFF entirely | Complete system failure — no indication of elevator status |

---

## 4. Bill of Materials

### 4.1 Electronics

| Qty | Component | Digikey / Source | Est. Cost | Notes |
|---|---|---|---|---|
| 1 | Arduino Uno R4 WiFi | B0C8V88Z9D (Amazon) | $26.95 | Main controller — 5V logic, SPI, PWM, GPIO, WiFi |
| 1 | DFPlayer Mini MP3 Module (5-pack) | B0CH2WZT5Q (Amazon) | $9.99 (5-pack) | MP3 playback via serial — drives panel speaker directly |
| 1 | MicroSD Card 8GB+ | Amazon | ~$5 | FAT32 formatted, stores MP3 files as 0001.mp3 etc. |
| 2 | 2N2222A NPN Transistor | 4491-2N2222A-ND | $1 | FF LED transistor switch + 1 spare |
| 4 | 470Ω Resistor 1/4W | CF14JT470RCT-ND | $0.25 | 2N2222A base resistor |
| 2 | 220Ω Resistor 1/4W | CF14JT220RCT-ND | $0.10 | FF helmet LED anode current limit |
| 1 | 1kΩ Resistor 1/4W | CF14JT1K00CT-ND | $0.10 | DFPlayer RX noise suppression |
| 2 | 100µF 16V Electrolytic Capacitor | UVR1C220MDD1TA | $0.50 | Power supply decoupling |
| 4 | 0.1µF 50V Ceramic Capacitor | K104K15X7RF5TH5 | $0.25 | IC bypass decoupling |
| 1 | Small perfboard ~10×7cm | — | $3 | Mounts transistors, passives |
| 3 | DIANN 8-pin PCB Screw Terminal Block | B0BLHGK43D (Amazon) | $7.98 (10-pack) | 2.54mm pitch, solder-in, 150V 6A |
| 1m | 22 AWG hookup wire assortment | — | $4 | Internal wiring |

### 4.2 Power Supply

| Qty | Item | Source | Est. Cost | Notes |
|---|---|---|---|---|
| 1 | Anker PowerCore 10K Power Bank | Amazon B0D5CLSMFB | $21.98 | 10,000mAh — ~11hr runtime at full load |
| 1 | USB-C cable, 1m | On hand / Amazon | $0–6 | Battery → Uno R4 WiFi |

### 4.3 Wire

| Qty | Item | Digikey Part No. | Est. Cost | Notes |
|---|---|---|---|---|
| 1 | 30 AWG wire-wrap, Blue, 100ft | R30B-0100 | ~$16 | U4 SIN → Uno D11 (MOSI) |
| 1 | 30 AWG wire-wrap, Red, 100ft | R30R-0100 | ~$16 | U4 CLK → Uno D13 (SCK) |
| 1 | 30 AWG wire-wrap, Yellow, 50ft | R-30Y-0050 | ~$11 | U4 LE → Uno D10 |
| 5 | 22 AWG stranded hookup, assorted | Alpha Wire 3051 series | ~$8 ea | Red/black/white/green/blue — all longer runs |

> U4 pin 21 (/OE) is soldered directly to a nearby GND via on the display board — no long wire run needed for this connection.

### 4.4 Hardware & Enclosure

| Qty | Item | Source | Est. Cost | Notes |
|---|---|---|---|---|
| 1 | Otis COP Panel Assembly | Salvaged | $0 | Buttons, keys, speaker all intact |
| 1 | Display Board AAA26800AGP | Salvaged | $0 | U4 chain entry verified; bridge rectifier intact |
| 1 | 4Ω Speaker | Salvaged | $0 | Original elevator speaker, driven by DFPlayer SPK_1/SPK_2 |
| 1 | LB3371-11CNWRN FF Helmet Indicator LED | Salvaged | $0 | Dual red LED; pins 6/8 anodes, 1/3 cathodes |
| 1 | Plywood mounting board | On hand | $0 | Panel already mounted |
| 1 | Small project enclosure | Amazon | $8 | Houses proto shield on Uno R4 WiFi |
| 4 | #6-32 machine screws + nuts | Hardware store | $2 | Mounting perfboard in enclosure |

### 4.5 Software & Tools

| Item | Cost | Notes |
|---|---|---|
| Arduino IDE 2.x | Free | Primary development environment |
| Arduino Uno R4 board package | Free | Via Arduino Board Manager |
| DFRobotDFPlayerMini library | Free | Via Arduino Library Manager |
| Multimeter | On hand | Continuity tracing and voltage verification |
| Logic analyzer (optional) | ~$55 | Useful for verifying A6276 SPI waveforms |
| Soldering iron | On hand | Perfboard assembly and display board tap points |

### 4.6 Cost Summary

| Category | Estimated Total |
|---|---|
| Electronics components | ~$42 |
| Power supply | $0–8 |
| Hardware & enclosure | ~$55 |
| Software & tools | ~$10 |
| Salvaged panel hardware | $0 |
| **TOTAL PROJECT COST** | **~$109–117** |

---

## 5. Safety Notes

This project runs entirely on 5V USB power. No mains voltage is present inside the project enclosure.

- Power entry is a USB-C port — inherently current-limited and safe
- The original rotary disconnect switch on the panel frame is non-functional and should be labeled **"DISPLAY ONLY"** to avoid confusion
- All wiring inside the COP should be inspected for cut wire ends; any exposed conductors should be insulated with heat-shrink or electrical tape
- The project enclosure housing the perfboard and Uno should be secured with screws and clearly labeled

---

## 6. Wiring Reference

### 6.1 Wire Color Reference

| Gauge | Color | Signal | From | To |
|---|---|---|---|---|
| 30 AWG | Black | /OE tack | U4 pin 21 | Nearest GND via on display board |
| 30 AWG | Blue | SIN | U4 pin 2 | Splice to 22 AWG blue at board edge |
| 30 AWG | Red | CLK | U4 pin 3 | Splice to 22 AWG white at board edge |
| 30 AWG | Yellow | LE | U4 pin 4 | Splice to 22 AWG yellow at board edge |
| 22 AWG | Red | VCC | Display board U4 pin 24 rail | Uno 5V pin |
| 22 AWG | Black | GND | Display board U4 pin 1 rail | Uno GND / perfboard common |
| 22 AWG | Blue | SIN | Splice to 30 AWG blue | Uno D11 (MOSI) |
| 22 AWG | White | CLK | Splice to 30 AWG red | Uno D13 (SCK) |
| 22 AWG | Yellow | LE | Splice to 30 AWG yellow | Uno D10 |
| 22 AWG | Blue | Phase 1 key signal | Recall key switch NO contact | Uno D5 |
| 22 AWG | Black | Phase 1 key common | Recall key switch common | GND rail |
| 22 AWG | Yellow | Phase 2 key signal | FF Op key switch NO contact | Uno D7 |
| 22 AWG | Black | Phase 2 key common | FF Op key switch common | GND rail |
| 22 AWG | White | Floor 3 signal | Floor 3 button NO contact | Uno D4 |
| 22 AWG | Red | Floor 2 signal | Floor 2 button NO contact | Uno D2 |
| 22 AWG | Blue | Floor 1 signal | Floor 1 button NO contact | Uno A0 |
| 22 AWG | Black | Button commons | All three button common contacts | GND rail (daisy-chain) |
| 22 AWG | Yellow | FF LED signal | Perfboard 2N2222 collector | FF indicator LED anode |
| 22 AWG | Black | FF LED return | FF indicator LED cathode | GND rail |
| 22 AWG | White | Speaker + | PAM8403 L output | Speaker + terminal |
| 22 AWG | Black | Speaker − | PAM8403 GND output | Speaker − terminal |

### 6.2 Schematic — Power Distribution

| Source | Rail | Loads |
|---|---|---|
| Anker PowerCore USB-C | 5V → Uno R4 USB-C | Uno R4 WiFi (all logic) |
| Uno R4 5V pin | 5V rail on proto shield | Display board VCC, DFPlayer VCC, transistor collector supply |
| Uno R4 GND pin | GND rail on proto shield | All grounds common |

### 6.3 Proto Shield Component Connections

| Component | Pin | Connects To | Via |
|---|---|---|---|
| Q1 (2N2222A) | Base | A0 | 470Ω resistor (R1) |
| Q1 (2N2222A) | Collector | T1 LED− | Also connects to 220Ω (R2) from 5V to LED+ |
| Q1 (2N2222A) | Emitter | GND rail | Direct |
| R1 (470Ω) | A0 → Q1 base | Transistor base current limiting | |
| R2 (220Ω) | 5V → T1 LED+ | FF Helmet LED anode current limiting | |
| R3 (1kΩ) | D1 → DFPlayer RX | Noise suppression | |
| C1, C2 (100µF) | 5V rail to GND | Bulk decoupling | |
| C3–C6 (0.1µF) | 5V rail to GND | High-frequency bypass | |
| DFPlayer Mini | VCC | 5V rail | Via female header |
| DFPlayer Mini | GND | GND rail | Via female header |
| DFPlayer Mini | RX | D1 via R3 | 1kΩ noise suppression |
| DFPlayer Mini | TX | D0 | Direct |
| DFPlayer Mini | SPK_1/SPK_2 | T1 SPK+/SPK− | Via screw terminal → speaker |

### 6.4 Screw Terminal Block T1 (Display, LED, Speaker)

| Pos | Signal | From (shield side) | To (external) |
|---|---|---|---|
| 1 | CLK | D13 | Display board U4 pin 3 |
| 2 | SIN | D11 | Display board U4 pin 2 |
| 3 | LE | D10 | Display board U4 pin 4 |
| 4 | 5V | 5V rail | Display board VCC rail |
| 5 | GND | GND rail | Display board GND rail |
| 6 | LED+ | 5V via R2 (220Ω) | FF Helmet LED anode |
| 7 | LED− | Q1 collector | FF Helmet LED cathode |
| 8 | SPK+ | DFPlayer SPK_1 | Speaker + terminal |

> U4 pin 21 (/OE) is soldered directly to a GND via on the display board — not routed through the terminal block.

### 6.5 Screw Terminal Block T2 (Inputs and Speaker Return)

| Pos | Signal | Uno Pin | External Connection |
|---|---|---|---|
| 1 | SPK− | DFPlayer SPK_2 | Speaker − terminal |
| 2 | Floor 3 | D4 | Floor 3 button NO contact |
| 3 | Floor 2 | D3 | Floor 2 button NO contact |
| 4 | Floor 1 | D2 | Floor 1 button NO contact |
| 5 | Phase 2 ON | D7 | Phase 2 key ON contacts |
| 6 | Phase 1 RESET | D6 | Phase 1 key RESET contact |
| 7 | Phase 1 ON | D5 | Phase 1 key ON contact |
| 8 | GND common | GND rail | All switch/button commons |

> Phase 2 HOLD (D8), Call Cancel (D9), and Hall Call (D12) wire directly from panel to Uno header pins — not routed through terminal blocks.

### 6.6 Key Switch Wiring Detail

| Switch | Wire | Connects To | Notes |
|---|---|---|---|
| Phase 1 Recall | Wire 3 (GND) | GND rail | Common |
| Phase 1 Recall | Wire 4 (ON) | D5 via T2 | Also feeds LED+ via 470Ω for shroud LED |
| Phase 1 Recall | Wire 2 (RESET) | D6 via T2 | Momentary contact |
| Phase 1 Recall | Red (LED anode) | Wire 4 node via 470Ω | Shroud LED lights when key is ON |
| Phase 1 Recall | Black (LED cathode) | GND | |
| Phase 2 FF Op | Wire 2 (GND) | GND rail | Common |
| Phase 2 FF Op | Wire 1 + 3 (ON) | D7 via T2 | Paired ON contacts — tied together at terminal |
| Phase 2 FF Op | Wire 4 (HOLD) | D8 direct | HOLD contact |

---

*Firefighter Elevator Training Simulator — Design Brief v2.3 | May 2026*
*A collaboration between the Gary Sinise Foundation and Vicksburg High School Applied Engineering & Robotics*
