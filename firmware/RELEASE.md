# Firmware Release Workflow

This directory holds the OTA-deployable R4 WiFi binary. Boards pull from:
`https://raw.githubusercontent.com/mbombich-robotics/Elevator-Simulator/main/firmware/FF_Elevator_R4_WiFi.bin`

## How to release a new version

1. Make your code changes in `FF_Elevator_R4_WiFi/FF_Elevator_R4_WiFi.ino`

2. Bump the version in two places:
   - Header comment: `v4.02` → `v4.03`
   - `#define FW_VERSION "4.02"` → `"4.03"`

3. Export the compiled binary:
   - Arduino IDE → Sketch → Export Compiled Binary
   - Find the output: `FF_Elevator_R4_WiFi.ino.bin` (inside the sketch folder)

4. Copy and rename it here:
   ```
   cp FF_Elevator_R4_WiFi/FF_Elevator_R4_WiFi.ino.bin firmware/FF_Elevator_R4_WiFi.bin
   ```

5. Update `version.txt` in the repo root to the new version string (e.g. `4.03`)

6. Commit both files and push:
   ```
   git add firmware/FF_Elevator_R4_WiFi.bin version.txt FF_Elevator_R4_WiFi/FF_Elevator_R4_WiFi.ino
   git commit -m "Release v4.03"
   git push
   ```

Remote boards will report the update available on next version check, and the
instructor can apply it with one button tap from the web UI at 192.168.4.1.

## OTA flow on the board

1. Instructor opens 192.168.4.1 → Software Update → enters building WiFi → Check for Updates
2. Board connects, fetches version.txt, shows "Update available: v4.03 (running v4.02)"
3. Instructor taps "Apply Update to v4.03"
4. Board downloads FF_Elevator_R4_WiFi.bin (~30-60s), verifies, writes to ESP32 storage
5. Board reboots; bootloader flashes new firmware (~10s)
6. Board comes back up on v4.03 in AP mode
7. Instructor reconnects to FF_Trainer — "OTA OK: v4.03" shown in update section
