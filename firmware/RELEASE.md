# Firmware Release Workflow

This directory holds the OTA-deployable R4 WiFi binary. Boards pull from:
`https://raw.githubusercontent.com/mbombich-robotics/Elevator-Simulator/main/firmware/FF_Elevator_R4_WiFi.ota`

## How to release a new version

1. Make your code changes in `FF_Elevator_R4_WiFi/FF_Elevator_R4_WiFi.ino`

2. Bump the version in two places:
   - Header comment: `v4.05` → `v4.06`
   - `#define FW_VERSION "4.05"` → `"4.06"`
   - `OTA_URL` stays as `.ota` — no change needed

3. Export the compiled binary:
   - Arduino IDE → Sketch → Export Compiled Binary
   - Find the output: `FF_Elevator_R4_WiFi.ino.bin` (inside the sketch folder under `build/`)

4. Package into .ota and copy to firmware/:
   ```
   py -3 create_ota.py FF_Elevator_R4_WiFi/build/arduino.renesas_uno.unor4wifi/FF_Elevator_R4_WiFi.ino.bin firmware/FF_Elevator_R4_WiFi.ota
   ```

5. Update `version.txt` in the repo root to the new version string (e.g. `4.06`)

6. Commit and push:
   ```
   git add firmware/FF_Elevator_R4_WiFi.ota version.txt FF_Elevator_R4_WiFi/FF_Elevator_R4_WiFi.ino
   git commit -m "Release v4.06"
   git push
   ```

Remote boards will report the update available on next version check, and the
instructor can apply it with one button tap from the web UI at 192.168.4.1.

## OTA flow on the board

1. Instructor opens 192.168.4.1 → Software Update → enters building WiFi → Check for Updates
2. Board connects, fetches version.txt, shows "Update available: v4.06 (running v4.05)"
3. Instructor taps "Apply Update to v4.06"
4. Board downloads FF_Elevator_R4_WiFi.ota (~30-60s), verifies, writes to ESP32 storage
5. Board reboots; bootloader flashes new firmware (~10s)
6. Board comes back up on v4.06 in AP mode
7. Instructor reconnects to FF_Trainer — "OTA OK: v4.06" shown in update section

## File formats

- `FF_Elevator_R4_WiFi.ota` — LZSS-compressed OTA package required by the UNO R4 WiFi
  ESP32 firmware. Created by `create_ota.py` from the raw `.bin`.
- `create_ota.py` — Python 3 script that packages a `.bin` into the `.ota` format
  (20-byte header: len + CRC32 + magic 0x23411002 + hdr_version, followed by
  LZSS-compressed firmware with EI=11, EJ=4, N=2048, F=17).
