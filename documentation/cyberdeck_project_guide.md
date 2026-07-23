# Space-Themed Cyberdeck Project Guide
> **Note:** This guide was generated with AI assistance and is intended as a starting point for learning purposes. Steps will be updated and verified as the build progresses.
>
> **Build order note:** This guide is structured **case-last**. All parts are ordered, and all firmware/software is built and bench-tested first (Pi, SDR ground station, CV star tracker, and the antenna rotator), using a temporary bench setup. The case is designed and built only once every component for Phases 1-4 is in hand and working, so the case is sized and laid out around the *final, fully-known* part list instead of being redesigned partway through. See the new **Phase 5: Case Build** for the case steps (these were previously sections 1.9/1.10).

A full step-by-step build guide: SBC prototype → daily-use deck → satellite ground station → CV star tracker → (optional) custom SoM carrier + antenna rotator.

This guide includes both a private `build-log.md` (your detailed working notes) and prompts to write public-facing portfolio summaries — suitable for sharing on GitHub/LinkedIn.

---

## Ongoing Throughout Project
- [ ] Commit to build-log.md after every session (even short notes)
- [ ] Take photos at each stage for documentation
- [ ] Keep a running "issues encountered" list with solutions — this is valuable for write-ups and interviews later
- [ ] As new parts arrive each phase, store them in your parts container (anti-static bags for bare boards/PCBs) rather than leaving them loose on the desk
- [ ] After each phase's portfolio summary is written, consider a short LinkedIn post linking to it — builds a visible trail of progress over time, not just one big reveal at the end

---

## SYSTEM DESIGN REFERENCE
*A standing reference for the full system architecture — Pi, STM32, and power management PCB — covering what connects where, why, and what protects it. This is the answer to "where does X go and why" at any point in the build; the phase sections below are the step-by-step execution. Update this section if any wiring/component decision changes mid-build.*

### Power source note
Primary power: **5V/5A USB-C PD power bank, multiple output ports.** Since the bank already outputs regulated 5V, no main buck/step-down converter is needed — the open question is **current budget**, not voltage conversion. Pi 5 alone can draw up to ~5A momentarily under full load; two MG996R + one SG90 servos can add several more amps under simultaneous stall. With multiple ports available, **dedicate a separate port to the servo rail** rather than sharing the Pi's port — confirm each port's individual current rating (not just the bank's total) before finalizing.

### STM32

**Hardware**

| Component | Purpose |
|---|---|
| VL53L4CX ToF sensor | Detects hand gestures (distance/motion) |
| I2C pull-up resistors on SDA/SCL | Required for I2C bus to function. **3.6kΩ, no series resistor** for fast mode (400kHz, ≤90pF load) or **1.5kΩ + 100Ω series resistor** for fast mode plus (1MHz, ≤90pF load) — confirm actual values against your PCB trace capacitance |
| XSHUT pull-up resistor (10kΩ) | XSHUT must always be driven to avoid leakage current |
| XSHUT → STM32 GPIO | Hardware standby control; also needed for future multi-sensor address reassignment |
| GPIO1 (interrupt) → STM32 GPIO + 10kΩ pull-up | Avoids pure polling — sensor signals when a result is ready (ST recommends this over polling) |
| Decoupling caps on AVDD, placed close to AVDDVCSEL/AVSSVCSEL pins | Datasheet-specified placement detail — matters for PCB layout in Phase 6 |
| ~~2.8V LDO regulator~~ — likely unnecessary | AVDD/IOVDD ranges both cover 3.3V; check breakout board schematic first before adding |
| TVS/ESD array on I2C lines | Extra margin beyond the sensor's own ±2kV HBM rating |
| Servo 1, 2, 3 PWM output pins | Physical pins driving azimuth, elevation, and pop-up servos |
| ~~Flyback diode per servo~~ — **not needed** | Standard analog hobby servos (MG996R, SG90) have internal H-bridge driver circuitry; the motor coil isn't directly exposed to the GPIO pin |
| TVS diode on 5V servo power rail | Clamps general transient/switching noise on the shared servo rail |
| Unidirectional logic level shifter (3.3V→5V), e.g. 74AHCT125 | A bidirectional I2C-style shifter (e.g. BSS138) relies on pull-ups and weak drive strength, which struggles with PWM signal integrity over servo cable length — use a proper unidirectional buffer |
| JST-SM 3-pin connector ×3 | Standard hobby servo connector |
| GPIO wire → Pi RUN pin | Physical line used to wake the Pi |
| GPIO wire → Pi shutdown signal pin | Physical line used to warn Pi of imminent shutdown |
| GPIO input from TPS3840 | Carries the "power failing" signal into the STM32 |
| STM32 status LED | Visual indicator, blink pattern set by firmware |
| 32.768kHz crystal | Drives the onboard RTC oscillator |
| RTC supercap (0.1-1F) + diode | Keeps RTC ticking through power loss |
| BOOT0 pull-down resistor | Holds STM32 in normal run mode on boot |
| SWD debug header (SWDIO, SWDCLK, GND, VCC) | Connector for flashing/debugging firmware |
| Fuse on main power input | Limits current if something shorts downstream |
| Test points on power rails (3.3V, 5V, servo rail) | Probe access without back-probing connectors |
| 100nF ceramic on every VDD pin | High-frequency noise filtering |
| 10µF bulk cap near power input | Smooths larger supply fluctuations |
| 1µF + 10nF on VDDA | Separate decoupling for analog supply |

**Firmware**

| Task | Purpose |
|---|---|
| I2C driver + gesture detection logic | Talks to ToF sensor at address 0x52 |
| Interrupt handler on GPIO1 | Reads ranging result on data-ready signal instead of polling |
| XSHUT control routine | Hardware standby / address reassignment |
| I2C bus-reset/recovery routine | If the I2C bus hangs (a device holds SDA low, e.g. after a power glitch), reconfigure I2C pins as GPIO, toggle SCL 9 times to free the stuck device, then reinitialize the I2C peripheral |
| Servo PWM generation (x3) | Generates duty cycles to command each servo's position |
| Staggered/soft-start servo movement | Avoid commanding all three servos to move simultaneously at full speed — reduces peak current draw, easing pressure on polyfuse sizing |
| UART driver (TX/RX to Pi) | Sends gesture events/status, receives az/el motor commands |
| Wake controller logic | Decides what counts as a wake gesture, pulls Pi RUN pin |
| Power supervisor task | Watches TPS3840 GPIO input, triggers shutdown signal to Pi |
| LED blink pattern logic | Sets idle vs active blink rate on status LED |
| RTC peripheral configuration | Sets up and reads time from the crystal-driven RTC |
| Watchdog timer | Auto-resets STM32 if firmware hangs |
| Motor control loop | Converts target az/el into PWM duty cycles |
| Heartbeat/status task | Periodically sends position + RTC time to Pi |
| I2C address reassignment routine (stretch) | Multi-sensor support via XSHUT |

### Pi 5

**Hardware**

| Component | Purpose |
|---|---|
| Power button → J2 header (panel-mount, parallel with onboard) | External on/off access without opening the case; parallel wiring means either button works independently |
| Gesture toggle button | Physical GPIO input |
| Reset/reboot button (hold 2+ sec, recessed) | Physical GPIO input |
| Spare button | Unassigned GPIO input |
| Power indicator LED | Physical LED |
| Gesture enabled LED | Physical LED |
| UART connection to STM32 | Crossed TX/RX wiring + common ground |
| Keyboard, trackpad, RTL-SDR dongle | USB peripherals via 4-port powered hub (Cruxtec AU3-H4-SG) |
| Panel-mount external USB-A port | Routes one hub port to case exterior for occasional USB devices |
| Pi Camera Module 3 (wide angle) — star tracker | CSI port 1 — faces sky for Phase 3 star tracker |
| Pi Camera Module 3 (wide angle) — webcam | CSI port 2 — faces forward, mounted at top of screen bezel like a laptop webcam |

| Main display | HDMI-connected screen |
| Audio output (3.5mm — kept free for headphones) | Headphone/headset jack, not used for internal speaker |
| Internal mono speaker (3W 4Ω) + PAM8403 amp | Good quality built-in speaker behind case grille; driven from Pi audio output via PAM8403 amp board |
| INMP441 I2S MEMS microphone | Connects to Pi GPIO via I2S interface (not USB); built-in mic for voice commands, audio recording, off-grid comms; frees a USB port vs USB mic |
| INA219 power monitor (I2C to Pi) | Reads actual battery voltage/current from power bank rail — without this there is no way to measure real battery % from a USB-C power bank |
| WS2812B addressable RGB LED strip | Ambient/decorative lighting; driven by STM32 (timer+DMA) due to strict bit-timing requirements |
| Ethernet (RJ45 panel-mount, optional) | Wired networking option |
| Official Pi 5 Active Cooler (heatsink + PWM fan) | Running RTL-SDR + OpenCV star tracker + Skyfield simultaneously is a heavy sustained CPU load; cheap insurance against throttling regardless of whether it's strictly necessary for your specific workload |

**Software**

| Task | Purpose |
|---|---|
| Gesture toggle handling | Debounces button press, flips gesture-enabled flag |
| Reset button handling | Detects 2-second hold, triggers `sudo reboot` |
| Power LED control | Sets LED based on awake/suspended state |
| Gesture LED control | Mirrors current gesture toggle state |
| UART communication handler | Sends az/el commands, receives gesture events + heartbeat |
| Star tracker CV pipeline | Processes camera images with OpenCV |
| Satellite pass prediction | Uses Skyfield to calculate upcoming passes |
| SDR control/recording | Controls SDR++/SatDump |
| Gesture → action mapping | Maps gesture events to volume/brightness/desktop/antenna actions |
| NTP time sync | Keeps system clock accurate when online |
| RTC fallback | Reads STM32's RTC over UART when offline |
| Auto-suspend timer daemon | Puts Pi to sleep after inactivity |
| Graceful shutdown script | Safely powers off on STM32's low-voltage signal |
| Mission control dashboard GUI | Displays satellite countdown, SDR status, battery %, antenna position, star tracker results |
| Staggered peripheral init on boot | RTL-SDR can cause a momentary USB power surge on init — sequence OpenCV/Skyfield/SDR startup rather than initializing all at once |

### Power Management PCB

**Hardware**

| Component | Purpose |
|---|---|
| LTC3225 supercap charger IC | Charges shutdown-buffer supercap, limits charge current |
| 10-47F supercap (shutdown buffer) | Stores buffered shutdown energy |
| Boost/UPS converter downstream of supercap, e.g. LTC3110 | Supercaps discharge with a drooping voltage curve, not a flat one — without a boost/UPS IC between the supercap and the Pi, most of the supercap's stored capacity would be wasted before voltage sags out of the Pi's usable range |
| TPS3840 voltage supervisor IC | Watches input voltage, signals STM32 below threshold |
| Reverse polarity protection (P-MOSFET/Schottky) | Protects downstream components if power connected backwards |
| Overvoltage protection | Stops voltage spikes exceeding ratings |
| Bulk decoupling cap at main input | Smooths first stage of incoming power |
| TVS diode on main input | Clamps large voltage transients |
| Fuse on main power input | First line of overcurrent protection |

*No firmware/software runs on this board — purely analog/passive protection circuitry.*

### Shared / cross-board

**Hardware**

| Component | Purpose |
|---|---|
| Servo power rail (5V, separate output port from Pi) | Prevents servo current spikes from browning out Pi/STM32 |
| 470µF electrolytic on servo rail | Absorbs bulk current surge on servo movement |
| 100nF ceramic in parallel on servo rail | Handles high-frequency noise the bulk cap can't |
| 100nF ceramic per servo signal line | Filters PWM noise from coupling into STM32 |
| Polyfuse, 7-8A on servo rail | Two MG996R at ~2.5A stall each plus an SG90 can exceed 5A under simultaneous load — sized up from initial 3-5A spec to avoid nuisance tripping |
| Common ground between Pi and STM32 | Required for UART/GPIO signaling to work |

### Servo selection (Hardware)

- **Servo 1 + 2 (rotator az/el):** MG996R — metal gear, holds antenna against wind/gravity
- **Servo 3 (pop-up):** SG90 — lightweight, enough torque to lift the antenna arm

### STM32 FreeRTOS task structure (Firmware — priority detail)

| Task | Priority | Function |
|---|---|---|
| UART Receive | High | Receive motor commands from Pi |
| Gesture/ToF (interrupt-driven via GPIO1) | High | React to ranging-ready interrupt, detect gestures, send to Pi |
| Power Supervisor | High | Monitor voltage supervisor signal, trigger Pi shutdown |
| I2C Bus Recovery | High (event-triggered) | Free a hung I2C bus if a device holds SDA low |
| Motor Control (with staggered movement) | Medium | Drive servos to target az/el position without simultaneous peak draw |
| Wake Controller | Medium | Monitor for wake gesture, pull Pi RUN pin |
| Heartbeat/Status | Low | Send position + RTC time to Pi periodically |

### Known Gaps (decisions, not parts — resolve before/during build)

- **Per-port current rating of your power bank** — confirm each individual output port's rated current, not just the bank's total 5A figure
- **STM32 power budget** — "always-on, ~mA draw" hasn't been measured directly; check against the specific Nucleo board's datasheet
- **PWM timer allocation** — confirm enough independent timer channels for 3 simultaneous servo PWM outputs
- **I2C speed decision** — fast mode vs fast mode plus changes pull-up/series resistor values (see STM32 hardware table above)
- **VL53L4CX breakout schematic check** — confirm what's already handled onboard (regulation, pull-ups, decoupling) before duplicating
- **Test point placement** — beyond power rails, decide on I2C/UART probe access
- **Case grounding strategy** — conductive vs non-conductive enclosure affects ESD/grounding approach at panel cutouts
- **RF/SDR grounding and shielding** — good practice flagged (shield SDR, ground SMA connector to chassis if metal case), but exact needs depend on case material and layout relative to switching regulators
- **Real-world thermal testing** — confirm whether the Pi 5 actually throttles under the combined SDR + OpenCV + Skyfield workload, even with the active cooler

---

## PHASE 1: Core Cyberdeck Build (SBC)

### 1.1 Order Parts
- [ ] Buy a storage container big enough to hold all project parts (Pi + accessories, screen, keyboard, power bank, small electronics, SDR + antenna, camera, and eventually STM32 + motors) until the case is built — look for something roughly A4-A3 footprint and 15-20cm deep (e.g. a stackable tote with a clip lid), ideally with a separate small compartment tray/tackle-box inside for screws/buttons/LEDs, and keep bare boards (Pi, STM32, SDR, camera) in anti-static bags within it
- [x] Raspberry Pi 5 (8GB) — **purchased** (Scorptec bundle w/ official power supply)
- [x] Active cooler for Pi 5 — **purchased**
- [x] microSD card, 64GB, A1 rated — **owned** (optional upgrade later: SanDisk Extreme 128GB A2 for better performance, or Pi M.2 HAT+ + NVMe SSD for a bigger jump — decide before finalising case design as HAT+ adds height under the board)
- [ ] Display — size/type TBD (see decision note below)
- [ ] USB-C PD power bank — **must explicitly list 5V/5A (25W) output**, not just high mAh (~$40-60 AUD) — look for "100W PD power bank" listings and check the spec sheet for the 5V/5A profile before buying; standard phone power banks (5V/2-3A) will undervolt the Pi 5 under load
- [ ] Mechanical keyboard (60% USB) or salvaged keyboard, wired (~$0-40 AUD) — Jaycar/Facebook Marketplace/JB Hi-Fi
- [ ] USB trackpad module (small, panel-mountable, built into case) (~$20-30 AUD) — AliExpress, search "USB trackpad module DIY"
- [ ] Bluetooth mini mouse (~$15-25 AUD) — for portable/away-from-deck use; pairs to Pi 5's built-in Bluetooth, no USB port used
- [ ] Powered USB hub — Cruxtec AU3-H4-SG 4-port USB 3.0 (~$19 AUD, Centrecom/CPL/Scorptec) — keyboard + trackpad + RTL-SDR + panel-mount USB-A are the 4 permanent devices; powered via USB-C port from power bank (not wall adapter); Pi's 4 native ports remain free for direct/occasional use
- [ ] USB-A to USB-C cable (if power bank doesn't include one) (~$5 AUD)
- [ ] Momentary push buttons x3-4 (panel-mount, ~$1-2 each) — AliExpress, search "panel mount momentary push button"
- [ ] Status LEDs x2-3 + 330Ω resistors (~$2 total) — AliExpress, search "5mm LED assorted kit"
- [ ] Panel-mount USB-A extension (~$5-8 AUD) — AliExpress
- [ ] Panel-mount 3.5mm audio extension cable (~$3-5 AUD) — AliExpress
- ~~USB-A to microSD card reader~~ — **removed**: redundant, Pi boots directly from its own microSD slot; occasional external storage handled via panel-mount USB-A port instead
- [ ] (Optional) RJ45 panel-mount extension cable (~$5-10 AUD) — Pi 5 has onboard Gigabit Ethernet; this routes it to case exterior
- [x] ToF sensor (VL53L4CX) — **owned**
- [x] STM32 Nucleo board (NUCLEO-L476RG) — **owned**

**Smaller Parts (order from AliExpress unless noted):**
- [ ] INMP441 I2S MEMS microphone module (~$3-5 AUD) — connects directly to Pi GPIO via I2S (not USB); built-in mic, no USB port used; search "INMP441 I2S microphone module"
- [ ] Good quality mono speaker, 3W 4Ω (~$5-10 AUD) — internal speaker, mounts behind case grille cutout (Phase 5); search "3W 4 ohm speaker"
- [ ] PAM8403 amplifier board (~$2-5 AUD) — drives the 3W speaker from Pi audio output; search "PAM8403 amplifier module"
- [ ] Addressable RGB LED strip, WS2812B 5V (~$8-15 AUD) — ambient/decorative lighting; search "WS2812B LED strip 5V"
- [ ] INA219 power monitor breakout (~$3-8 AUD) — connects to Pi I2C; reads actual battery voltage/current from power bank rail; feeds real battery % to dashboard
- [ ] Potentiometer with built-in push-button, panel-mount (~$3-8 AUD) — rotary knob controls LED brightness (STM32 ADC), push controls LED mode cycling (STM32 GPIO); search "potentiometer switch panel mount"
- [ ] Passive buzzer (~$1-3 AUD) — PWM-driven from STM32 for audible alerts; search "passive buzzer 5V"
- [ ] STM32 power mode toggle switch, panel-mount latching (~$1-3 AUD) — Low power vs Off mode for STM32; search "panel mount toggle switch"
- [ ] MG996R servo x2 (~$8-12 AUD each) — azimuth + elevation for antenna rotator (Phase 4)
- [ ] SG90 servo x1 (~$3-5 AUD) — antenna pop-up deploy/stow (Phase 4 stretch)
- [ ] 32.768kHz crystal (~$1-2 AUD) — drives STM32 RTC oscillator for accurate timekeeping
- [ ] 0.1-1F supercap for RTC backup (~$2-5 AUD) — keeps STM32 RTC running through power loss
- [ ] 10-47F supercap for shutdown buffer (~$5-15 AUD) — provides 5-10 second power buffer for graceful Pi shutdown
- [ ] TPS3840 voltage supervisor IC (~$2-5 AUD, Mouser/AliExpress) — watches power bank voltage, signals STM32 when dropping
- [ ] LTC3225 supercap charger IC (~$5-10 AUD, Mouser/AliExpress) — safely charges shutdown supercap with current limiting
- [ ] LTC3110 boost/UPS IC (~$5-10 AUD, Mouser/AliExpress) — prevents wasted supercap energy as voltage droops during shutdown
- [ ] 74AHCT125 logic level shifter (~$1-3 AUD) — 3.3V STM32 → 5V for WS2812B data line and servo signals
- [ ] 470Ω resistor on WS2812B data line — in resistor kit
- [ ] 1000µF electrolytic cap for WS2812B power input (~$1 AUD)
- [ ] 470µF electrolytic cap for servo power rail (~$1 AUD)
- [ ] 100nF ceramic cap pack of 100 (~$3-5 AUD) — decoupling on every IC
- [ ] 10µF electrolytic cap pack of 50 (~$3-5 AUD) — bulk decoupling
- [ ] TVS diode for main power input (~$1-2 AUD)
- [ ] TVS/ESD array for I2C lines (~$1-3 AUD)
- [ ] Polyfuse 7-8A for servo power rail (~$2-5 AUD)
- [ ] P-channel MOSFET for reverse polarity protection (~$1-2 AUD)
- [ ] JST-SM 3-pin connectors x3 for servos (~$2-3 AUD)
- [ ] SMA panel-mount connector for antenna feed-through (~$3-5 AUD)
- [ ] Strain relief cable clamps, assorted pack (~$3-5 AUD)
- [ ] SWD debug header pins for STM32 (~$1-2 AUD)
- [ ] Diode assortment pack (~$2-3 AUD)
- [ ] Resistor kit (~$5-10 AUD if not already owned)
- [ ] Fuse holder + fuses (~$2-5 AUD)
- [ ] Status LEDs x7 + 330Ω resistors — need: green (power on), blue (gesture active), amber (charging), red (low battery), white (STM32 heartbeat), purple (satellite pass active), teal (SDR recording)

**Screen size decision (do this before ordering display):**
- [ ] Decide: more laptop-like (7-10", easier to read/code, less portable) vs. more compact/handheld (5-7", easier to carry, smaller text)
- [ ] Once size is chosen, confirm it's HDMI input (simplest, universal) rather than DSI-only
- [ ] Order display once size/interface confirmed

### 1.2 Documentation Setup
- [x] Create a GitHub repository (e.g. "space-cyberdeck")
- [x] Write README.md with project goal (1 paragraph) — this is the first thing anyone (including LinkedIn visitors) will see, so keep it clear and welcoming
- [x] Create a `build-log.md` file — your detailed working notes, commit here after each session
- [x] Create a `/docs` or `/portfolio` folder for public-facing summaries — shorter, polished write-ups per phase (what it does, why it's interesting, a photo/demo) suitable for linking from LinkedIn
- [x] Create a `/photos` folder for build photos as you go
- [x] Add & start BOM Excel sheet
- [ ] Add a short "Project Overview" section to README covering: what the deck does, what skills it demonstrates, and links to each phase's portfolio write-up (you'll fill these in as you complete each phase)

### 1.3 Initial Hardware Setup
- [x] Mount active cooler onto bare Pi board before anything else

### 1.4 Initial Software Setup
- [x] Download Raspberry Pi Imager on your laptop (raspberrypi.com/software)
- [x] Flash Raspberry Pi OS (64-bit, Desktop version) onto the microSD card
  - In Imager: click "Edit Settings" before flashing — set hostname (`aphelion`), enable SSH, set username (`emma`), set password, configure wifi (saves doing this on first boot)
- [x] Insert microSD into Pi 5, connect display, keyboard, power
- [x] Boot the Pi — confirm it reaches the desktop
- [x] Connect to wifi (if not pre-configured)
- [x] Open terminal, run:
  ```
  sudo apt update && sudo apt full-upgrade -y
  sudo reboot
  ```
- [x] Switch SSH authentication from password to public key
- [x] Link Raspberry Pi to GitHub via SSH
- [x] Link my local laptop to the Raspberry Pi via SSH for easy file transfer/access

### 1.5 Install Core Software
- [x] Install VS Code:
  ```
  sudo apt install code -y
  ```
  (if not found, follow official VS Code ARM64 .deb install instructions)
- [x] Confirm Chromium is installed (usually pre-installed): `chromium-browser --version`
- [x] Install Git:
  ```
  sudo apt install git -y
  ```
- [x] Configure Git identity:
  ```
  git config --global user.name "Your Name"
  git config --global user.email "your@email.com"
  ```
- [x] (Optional) Install LibreOffice:
  ```
  sudo apt install libreoffice -y
  ```

### 1.6 Test the Build
- [x] Open VS Code, test by editing .md repo files, confirm editing works smoothly
- [x] Open Chromium and Firefox, browse a few sites, confirm performance is acceptable --> I am gonna make Firefox my default as it seems to work better
- [x] Initial test with my laptop's Bluetooth mouse to see if the Bluetooth module works
- [ ] Test touchscreen responsiveness (if using touchscreen)
- [ ] Test keyboard — all keys register correctly
- [ ] Test trackpad — registers correctly, no conflicts
- [ ] Pair Bluetooth mouse: Pi desktop → Bluetooth icon → scan → select mouse → confirm pairing; test click/movement
- [ ] Check battery runtime with power bank (note down approx hours)
- [ ] Decide on whether to upgrade the storage device or not to A2 SD card or NVME hat

### 1.7 Gesture Detection & Power Management (ToF Sensor on STM32)
*The VL53L4CX connects to the STM32 via I2C — not the Pi. The STM32 handles all gesture detection logic and sends gesture events to the Pi over UART. This is the correct final architecture (not a stretch goal anymore) — it means the STM32 can detect gestures and wake the Pi even when the Pi is fully suspended, which a Pi-connected sensor could never do.*

**STM32 firmware (C + FreeRTOS — done as part of Phase 4 STM32 work):**
- [x] Wire VL53L4CX to STM32 I2C pins (SDA, SCL, XSHUT, GPIO1, VDD, GND) — refer to System Design Reference for pull-up/protection values
- [x] Set up STM32CubeMX project for the ToF sensor, including setting up interrupt on GPIO1 pin — sensor signals data-ready instead of polling
- [x] Get a blinking LED test working on the STM32 to confirm it has been set up correctly
- [x] Set up JLink RTT Viewer for debugging JLink printf output via WSL
- [x] Ensure RTT works by printing a test statement
- [x] Write ToF initialisation and ranging code using the VL53L4CX driver API
- [ ] Optimise the ToF ranging code to ensure it only triggers when there's an object in front of the sensor e.g. software filter
- [ ] Write gesture detection logic: interpret distance changes across zones/over time as swipe left/right, hand near/far
- [ ] Send gesture event codes to Pi over UART (e.g. `GESTURE:SWIPE_LEFT\n`)
- [ ] Implement wake controller: on wake gesture → pull Pi RUN pin low → Pi wakes from suspend
- [ ] Read power mode toggle switch GPIO — Low power mode (ToF polling active) vs Off mode (STM32 deep sleep, no gesture detection)
- [ ] Test: trigger gestures by hand, confirm UART outputs correct event codes (use serial monitor on laptop)

**Pi software (Python — done here in Phase 1):**
- [ ] Enable UART on Pi:
  ```
  sudo raspi-config
  ```
  → Interface Options → Serial Port → disable login shell, enable serial hardware
- [ ] Write Python UART listener script: reads gesture event codes from STM32, maps to Pi actions
- [ ] Map gesture events to actions:
  - Swipe = switch virtual desktop
  - Hand near = wake screen / increase brightness
  - Hand far + hold = sleep / decrease brightness
- [ ] Test: trigger gesture on STM32, confirm Pi executes correct action
- [ ] Add gesture enable/disable flag controlled by toggle button (section 1.8)

**Other gesture ideas (software-only, no motors — easy additions once core gestures work):**
- [ ] Swipe up/down → system volume or screen brightness
- [ ] Wave → cycle status readout on screen (battery %, next satellite pass time)
- [ ] Manual antenna nudge (Phase 4): once rotator exists, gesture sends nudge command to STM32 motor control task

**Power management (STM32 always-on wake controller):**
- [ ] Pi auto-suspend timer daemon: Pi suspends after X minutes of inactivity (configure in Phase 1 software setup)
- [ ] STM32 wake sequence: gesture detected → STM32 pulls Pi RUN pin low → Pi wakes fully
- [ ] Document power draw in build-log: Pi always-on vs STM32-watching + Pi suspended — this is a real spacecraft housekeeping pattern

### 1.8 Button & LED Software (Bench Testing)
Test all button and LED logic on the bench with jumper wires — using your storage container/bench box setup, not the final case. Debugging a loose GPIO connection inside a sealed enclosure is miserable, so all of this stays bench-only until Phase 5.

**Gesture detection toggle button**
- [ ] Wire a second momentary push button to a free Pi GPIO pin (e.g. GPIO17) with a pull-up resistor (or use Pi's internal pull-up in software)
- [ ] In your gesture detection script (from 1.7), add logic:
  - [ ] On startup, set a flag `gesture_enabled = True`
  - [ ] Poll the button GPIO; on each press (debounced), toggle `gesture_enabled`
  - [ ] Only run gesture logic when `gesture_enabled` is True
- [ ] Wire a status LED (with appropriate resistor, e.g. 330Ω) to another GPIO pin
  - [ ] LED on = gesture detection active, LED off = disabled
  - [ ] Update LED state in script whenever toggle changes
- [ ] Test: press button, confirm LED toggles and gesture actions stop/start accordingly

**Power button (safe shutdown + boot)**
- [ ] Wire a momentary push button to Pi 5's power button header (GPIO pins labelled for power, check Pi 5 documentation for exact pins — typically a 2-pin header near USB-C)
- [ ] Test: single press while off → boots Pi; press while on → triggers safe shutdown (default Pi 5 behavior, no extra software needed for basic on/off)

**Reset/reboot button (optional but useful)**
- [ ] Wire a third momentary button to another GPIO pin
- [ ] Write a small background script that triggers `sudo reboot` when held 2+ seconds (avoids accidental reboots)
- [ ] Test: confirm hold triggers reboot, short press does nothing

**Record GPIO pin assignments**
- [ ] Update build-log.md with the GPIO pin used for every button and LED — you'll reference this constantly during case assembly in Phase 5

---

## PHASE 2: Satellite Tracking & SDR Ground Station

*Still bench-based — SDR and antenna are tested loose, not mounted in anything yet. Store the dongle/antenna in your parts container between sessions.*

### 2.1 Order Parts
- [ ] RTL-SDR dongle with bundled antenna (~$35-45 AUD)

### 2.2 Install Tracking Software
- [ ] Install Gpredict:
  ```
  sudo apt install gpredict -y
  ```
- [ ] Open Gpredict, set your ground station location:
  - Edit → Preferences → Ground Stations → Add new
  - Set location to Brisbane (lat/long, altitude)
- [ ] Update TLE data: Edit → Update TLE Data → From Network
- [ ] Confirm satellites appear and orbits are plotted correctly

### 2.3 Verify Pass Prediction
- [ ] In Gpredict, find the next ISS pass for your location
- [ ] Cross-check timing against heavens-above.com for the same location
- [ ] Confirm predicted times match (within a minute or two)

### 2.4 Install SDR Software
- [ ] Plug in RTL-SDR dongle, confirm it's detected:
  ```
  lsusb
  ```
  (should show "Realtek" device)
- [ ] Install SDR++ or SatDump:
  ```
  sudo apt install sdrpp -y
  ```
  (if not in repo, download AppImage/build from GitHub releases — check SatDump GitHub for ARM64 builds)
- [ ] Open SDR++, select RTL-SDR as source, confirm you can see a live spectrum waterfall

### 2.5 Receive a NOAA Weather Satellite Pass
- [ ] In Gpredict, find next pass of NOAA-15, NOAA-18, or NOAA-19:
  - [ ] In the satellite list, locate the NOAA satellites (add them if not shown: right-click satellite list → select satellites → search "NOAA")
  - [ ] Check the "next pass" panel for each — pick the pass with highest max elevation (higher = better signal, less obstruction)
  - [ ] Note down: start time, max elevation time, end time, and frequency (~137.1/137.62/137.9125 MHz depending on satellite)
- [ ] Position antenna with clear sky view:
  - [ ] Take antenna + SDR + Pi outdoors (or near a window with clear sky access) at least 10 mins before pass start
  - [ ] Orient antenna vertically (most simple antennas for these passes work best vertical, polarization is circular but linear works adequately)
  - [ ] Avoid metal objects, walls, or trees directly overhead if possible
- [ ] At pass time, tune SDR++ to the satellite's frequency:
  - [ ] Open SDR++, set frequency to the noted value (e.g. 137.9125 MHz for NOAA-19)
  - [ ] Set mode to WFM (wide FM) or NFM depending on SatDump's recorder requirements (check SatDump docs for exact mode/bandwidth, typically ~40-50kHz bandwidth)
  - [ ] Set sample rate appropriately (e.g. 1.024 MSPS or as recommended by SatDump)
- [ ] Record the audio/IQ during the pass:
  - [ ] Start SatDump's "live processing" mode with APT decoder selected, or use SDR++'s recorder to save a .wav/IQ file for offline decoding
  - [ ] Start recording 1-2 minutes before predicted pass start (satellite may appear early at low elevation)
  - [ ] Keep recording until pass ends (signal will fade out)
- [ ] Decode using SatDump (APT decoder):
  - [ ] If recorded live, decoding happens automatically; if recorded as file, open SatDump, select "APT Decoder" module, load the recorded file
  - [ ] Let it process — produces a raw image file
- [ ] Save the decoded image:
  - [ ] Locate output image file (check SatDump's output directory)
  - [ ] Copy to your project folder, this is your "received from space" deliverable — even a noisy/partial image counts as success on first attempt

### 2.6 Receive ISS APRS Packets (optional second win)
- [ ] Find ISS pass in Gpredict (frequency ~145.825 MHz)
- [ ] Tune SDR++ to this frequency during pass
- [ ] Use a packet decoder (e.g. Direwolf) to decode APRS packets
- [ ] Log any received packets

### 2.7 Python Orbit Prediction Script
- [ ] Install Skyfield:
  ```
  pip install skyfield
  ```
- [ ] Write a script that:
  - Downloads/loads a TLE for the ISS
  - Calculates next pass time for your location (Brisbane coordinates)
  - Prints pass start time, max elevation, end time
- [ ] Run the script, compare output against Gpredict's prediction for the same satellite
- [ ] Commit script to GitHub repo

### 2.8 Document Phase 2
- [ ] Update build-log.md: what worked, what didn't, decoded image attached
- [ ] Write a short "requirements + test" note: what you expected the SDR setup to do, and how you verified it worked
- [ ] Write a public portfolio summary (`/docs/phase2-ground-station.md`): 3-5 sentences on what you built, include your best decoded NOAA image, link to your Skyfield script — this is the entry you'd point someone to on LinkedIn
- [ ] Take a photo of your antenna/SDR setup in action for the portfolio folder

### 2.9 (Stretch) Hardware Preamp/Filter Board for 137MHz
- [ ] Research existing 137MHz LNA (low-noise amplifier) + bandpass filter designs (search for open-source "137MHz preamp" schematics, common in NOAA APT hobbyist community)
- [ ] Identify a suitable LNA IC (e.g. low-cost RF amplifier chip) and filter component values for ~137MHz passband
- [ ] Design schematic: antenna → bandpass filter → LNA → output to RTL-SDR
- [ ] Design PCB layout (KiCad) — keep RF traces short, follow grounding best practices for RF circuits
- [ ] Order PCB + components, assemble (SMD soldering — consider uni lab equipment for small components)
- [ ] Bench test: confirm amplifier draws expected current, no oscillation/instability
- [ ] Field test: repeat a NOAA pass reception (section 2.5) with and without the preamp inline
  - [ ] Compare decoded image quality (less noise/static lines = improvement)
  - [ ] Note signal strength readings in SDR++ with/without preamp
- [ ] Document results in build-log.md — this is a genuine RF PCB design + test deliverable for your portfolio
- [ ] Store the assembled preamp board in an anti-static bag in your parts container until case design (Phase 5) — bare PCBs are easily damaged loose

---

## PHASE 3: CV Star Tracker

*Still bench-based — camera is handheld/tripod-mounted for testing, not mounted in the case yet.*

### 3.1 Order Parts
- [ ] Raspberry Pi Camera Module 3 Wide Angle x2 (~$35-45 AUD each) — one for star tracker (CSI port 1, sky-facing), one for webcam (CSI port 2, forward-facing at top of screen bezel); both connect via CSI ribbon cable, no USB ports used
- [ ] Camera ribbon cable (usually included)
- [ ] Small tripod or mount for camera (improvised is fine)

### 3.2 Setup Camera
- [ ] Connect camera module to Pi's CSI port (power off first)
- [ ] Enable camera in raspi-config:
  ```
  sudo raspi-config
  ```
  → Interface Options → Camera → Enable
- [ ] Reboot, test capture:
  ```
  libcamera-still -o test.jpg
  ```
- [ ] Confirm image looks correct

### 3.3 Capture Night Sky Images
- [ ] Wait for a clear night with minimal cloud cover (check weather forecast/BOM radar) and ideally away from heavy light pollution (e.g. backyard with lights off, vs. CBD)
- [ ] Let camera/Pi sit outside for ~15 mins before shooting (temperature stabilization reduces noise)
- [ ] Set camera to long exposure / high ISO settings:
  - [ ] Use `libcamera-still` manual controls, e.g.:
    ```
    libcamera-still -o sky_test.jpg --shutter 5000000 --gain 8 --awb off
    ```
    (shutter in microseconds — 5000000 = 5 seconds; adjust gain/shutter based on results)
  - [ ] If images are too bright/washed out, reduce shutter time or gain; if too dark, increase
- [ ] Capture several test images pointing at different known regions:
  - [ ] Use a star chart app (Stellarium mobile, or just look up) to identify where Orion or the Southern Cross currently is in the sky
  - [ ] Point camera at that region (mount on tripod or prop against something stable — any movement during exposure blurs stars)
  - [ ] Take 3-5 shots per region, varying exposure slightly each time
- [ ] Review images on a larger screen (transfer to laptop if needed):
  - [ ] Zoom in — confirm stars appear as small distinct white dots, not blurry streaks (streaks = camera moved during exposure)
  - [ ] Confirm background is dark, not washed-out grey (if grey, reduce exposure/gain)
  - [ ] Pick your best 2-3 images to carry forward to section 3.5

### 3.4 Install CV Software
- [ ] Install OpenCV:
  ```
  pip install opencv-python numpy
  ```
- [ ] (Optional reference tool) Install Stellarium on your laptop for cross-checking star positions:
  - Download from stellarium.org

### 3.5 Build Star Detection Pipeline
- [ ] Write Python script to:
  - Load captured image
  - Apply thresholding to isolate bright points (stars) from background
  - Use OpenCV blob detection (`cv2.SimpleBlobDetector`) to find star centroids
  - Output list of (x, y) pixel coordinates for detected stars
- [ ] Test on your captured images — confirm detected points match visible stars

### 3.6 Star Pattern Matching (Attitude Estimation)
- [ ] Research triangle-matching/plate-solving concept:
  - [ ] Read astrometry.net's "how it works" page for the general approach (geometric hashing of star triangles)
  - [ ] Key idea to understand: relative angles/distances between stars are unique "fingerprints" that don't change with rotation, so you match patterns rather than absolute positions
- [ ] Build a small reference catalog:
  - [ ] Pick 1-2 constellations visible from Brisbane that you captured in 3.3 (e.g. Southern Cross, Orion's Belt)
  - [ ] Using Stellarium, look up the RA/Dec coordinates of each major star in your chosen constellation
  - [ ] Manually calculate relative angular distances between each pair of stars (or note their relative pixel positions from a reference image) — store as a small lookup table/dictionary in your script
- [ ] Write matching script:
  - [ ] Take the (x,y) star coordinates output from 3.5
  - [ ] Calculate pairwise distances/angles between all detected points (same method as your reference catalog)
  - [ ] Compare detected pattern's ratios/angles against each reference constellation's pattern
  - [ ] Find best match (smallest difference) — this tells you which constellation is in view
- [ ] Output: print which constellation/region was identified, and roughly which part of the image corresponds to which known star (label them)

### 3.7 Validate
- [ ] Compare your script's output against what Stellarium shows for that time/location/orientation
- [ ] Note accuracy — how close is the estimate?

### 3.8 Document Phase 3
- [ ] Update build-log.md with results, sample images, accuracy notes
- [ ] Write reflection: what would need to improve for higher accuracy (wider star catalog, better lens, etc.)
- [ ] Write a public portfolio summary (`/docs/phase3-star-tracker.md`): explain attitude determination in plain terms, show a sample night-sky image with detected stars marked, note your match accuracy
- [ ] Add this to your README's project overview — this is your strongest "real space industry skill" talking point, worth highlighting clearly

---

## PHASE 4 (Optional/Stretch): Antenna Rotator

*Still bench-based — mount, motors, and firmware are all built and tested loose/on a temporary stand, not inside the case.*

### 4.1 Order Parts
- [ ] 2x servo motors or stepper motors (az/el control) (~$20-40 AUD)
- [ ] Motor driver board (if using steppers)
- [ ] Az/el mount hardware (3D printed or basic bracket build)
- [ ] UART connection cable (Pi ↔ Nucleo)

### 4.2 Build Mount
- [ ] Design 2-axis (azimuth + elevation) mount:
  - [ ] Azimuth stage: a base platform that rotates 0-360° (motor mounted vertically, driving a turntable/bearing)
  - [ ] Elevation stage: mounted on top of the azimuth stage, tilts the antenna 0-90° (motor mounted horizontally, driving an arm holding the antenna)
  - [ ] Search for existing open-source az/el rotator designs (e.g. on Thingiverse/Printables) to use/adapt rather than designing from scratch
- [ ] Print/build the mount:
  - [ ] Print all parts (or cut/assemble if non-printed design)
  - [ ] Assemble azimuth stage first — test it rotates freely 360° by hand before attaching motor
  - [ ] Attach elevation stage on top — test it tilts freely 0-90° by hand before attaching motor
- [ ] Attach motors:
  - [ ] Mount azimuth motor, connect to rotating platform (via gear, belt, or direct coupling depending on design)
  - [ ] Mount elevation motor, connect to tilting arm
  - [ ] Secure motor wiring with strain relief (zip ties) so cables don't tangle during rotation
- [ ] Test manual movement range:
  - [ ] Power motors individually (bench power supply or driver board, not yet connected to STM32 logic)
  - [ ] Send simple test signals (e.g. basic PWM for servos, or step pulses for steppers) to confirm azimuth motor can sweep full 0-360°
  - [ ] Confirm elevation motor can sweep 0-90° without mechanical binding
  - [ ] Note any physical limits (e.g. cable wraps at certain az angles) — these become software limits later
- [ ] Mount the antenna (from Phase 2) onto the elevation arm, secure so it doesn't wobble during movement

### 4.3 Firmware (STM32 + FreeRTOS)
- [ ] Set up FreeRTOS in your STM32 project (via STM32CubeMX — enable FreeRTOS middleware, generate code)
- [ ] Create Task 1: UART Receive Task
  - Listens for az/el commands from Pi
  - Parses received angle values
  - Pushes target angles onto a queue
- [ ] Create Task 2: Motor Control Task
  - Reads target angles from the queue
  - Runs control loop (PID or simple proportional control) to drive motors toward target
  - Reads encoder/feedback if available
- [ ] Create Task 3 (optional): Status/Heartbeat Task
  - Periodically reports current position back to Pi over UART (useful for debugging)
- [ ] Set appropriate task priorities (UART receive > motor control > status reporting)
- [ ] Test: send manual commands from a serial terminal, confirm motors move to correct angles
- [ ] Note in build-log: why FreeRTOS was chosen here (task separation, deliberate RTOS skill-building) as a talking point for portfolio/interviews

### 4.4 Integration (Pi side)
- [ ] Modify Phase 2 Python script (or use Gpredict's hamlib/rotator interface) to output real-time az/el for current satellite pass
- [ ] Send az/el updates over UART to STM32 at regular intervals (e.g. every 1-2 seconds)

### 4.5 Live Tracking Test
- [ ] During a satellite pass, run the full system: Pi predicts position → sends to STM32 → motors move antenna
- [ ] Observe whether antenna tracks the pass smoothly
- [ ] Check if signal reception (Phase 2 setup) improves with active tracking vs fixed antenna

### 4.6 Document Phase 4
- [ ] Update build-log.md with design files, firmware code, test results, video/photos of tracking in action
- [ ] Write a public portfolio summary (`/docs/phase4-antenna-rotator.md`): explain the FreeRTOS task structure with a simple diagram, include a short video of the antenna tracking a live pass — videos are highly effective on LinkedIn

### 4.7 (Stretch) Gesture-Triggered Antenna Pop-Up
*A small, contained motor project — antenna sits stowed flat when not in use, gesture (from your Phase 1 ToF sensor) deploys it upright. Note: because this mechanism deploys through the case wall itself, its hinge cutout needs to be planned as part of Phase 5's case design, not bolted on afterward.*
- [ ] Design simple deploy mechanism: a single hinge point with a small servo or DC motor providing the lift force (much lighter/simpler than a full rotator — just one motion, stowed to upright)
- [ ] Add a limit switch or basic position sensing at the "deployed" end-stop, so the motor knows when to stop (avoids over-driving and stalling the motor)
- [ ] Wire motor + limit switch to STM32 (can reuse Phase 4.3's STM32 setup, or run on a separate small MCU if you want it independent of the rotator)
- [ ] Add a new FreeRTOS task (or simple state machine if not using RTOS here): listens for "deploy" signal from Pi (triggered by gesture sensor), drives motor to deployed position, stops at limit switch
- [ ] Add a "stow" trigger too (second gesture, or a software command) — reverses motor to flat position
- [ ] Test: trigger gesture, confirm antenna reliably deploys and stows without binding or stalling
- [ ] Document in build-log.md: mechanism design choices, what you learned about motor control/torque sizing for a real moving mechanism

---

## PHASE 5: Case Build

*Moved here deliberately — by this point every component from Phases 1-4 has been bought and bench-tested, so the case is designed once around the complete, final part list instead of being reworked as new phases add hardware. (Previously sections 1.9/1.10 in early drafts of this guide.)*

### 5.1 Final Parts & Layout Audit
- [ ] Empty your storage container and lay out every component that needs to fit in or on the case: Pi 5 + cooler, screen, keyboard, trackpad, USB hub, power bank, buttons/LEDs, ToF sensor, RTL-SDR + antenna, (optional) preamp board, camera module + mount, (optional) STM32 + motors + az/el mount
- [ ] Note which items must be exterior-accessible (screen, keyboard, trackpad, ports, buttons, camera — needs sky view, antenna — needs to protrude or have a feed-through) vs. which can live fully internal (Pi, USB hub, STM32, preamp board)
- [ ] Decide whether the antenna rotator (Phase 4) is a separate standalone unit cabled to the deck, or physically integrated into the case — this materially changes the case footprint, so confirm before sketching layout
- [ ] Re-confirm physical dimensions of every component now that you physically have them (don't rely on spec-sheet numbers — measure)

### 5.2 Design
- [ ] Sketch rough layout on paper: top-down view showing screen position, keyboard position, trackpad position, Pi location, battery location, USB hub location, SDR/camera/STM32 placement, cable routing paths
- [ ] Decide enclosure approach: 3D printed (design in Fusion360/Tinkercad/FreeCAD), laser-cut panels, or repurposed enclosure (e.g. old hardware case/toolbox)
- [ ] If 3D printing: design/find a base plate with mounting holes matching Pi 5's hole pattern (49mm x 58mm spacing) and screen's mounting holes
- [ ] Print or cut a test/prototype version first (doesn't need to be final material — cardboard mockup is fine for layout check)

### 5.3 Mount Core Components
- [ ] Mount the Pi 5:
  - [ ] Attach standoffs/spacers at Pi's mounting holes (keeps board off the base, allows airflow under cooler)
  - [ ] Screw Pi board onto standoffs
  - [ ] Confirm active cooler isn't obstructed and has airflow clearance
- [ ] Mount the display:
  - [ ] Position screen at a usable viewing angle (consider a slight tilt/stand if not flat)
  - [ ] Secure using screen's own mounting holes/brackets, or adhesive/velcro if no holes
  - [ ] Route display cable (HDMI) from screen to Pi — leave enough slack for the case to close/open if hinged
- [ ] Mount the keyboard:
  - [ ] Position below or beside screen depending on layout
  - [ ] Secure with screws (if keyboard PCB has mounting holes) or velcro/adhesive standoffs
  - [ ] Route keyboard USB cable to USB hub, avoiding tight bends
- [ ] Mount the trackpad:
  - [ ] Position below/beside keyboard within comfortable thumb/finger reach
  - [ ] Secure with adhesive or printed bracket
  - [ ] Route trackpad USB cable to USB hub
- [ ] Mount the USB hub:
  - [ ] Position somewhere accessible internally (not necessarily exterior-facing)
  - [ ] Connect hub's upstream cable to one of Pi 5's USB ports
  - [ ] Connect keyboard, trackpad, RTL-SDR dongle, and panel-mount USB-A to the 4 hub ports — all 4 ports now permanently allocated
- [ ] Bluetooth mouse needs no mounting — just keep it nearby/clipped to the case exterior if you want it always at hand (e.g. small velcro strap)
- [ ] Mount the battery/power bank:
  - [ ] Place in a location with weight balance in mind (avoid top-heavy/unstable result)
  - [ ] Secure with a strap, velcro, or printed bracket (must not shift when device is moved)
  - [ ] Route USB-C power cable to Pi's power input

### 5.4 Mount Phase 2-4 Components
- [ ] Mount RTL-SDR dongle internally, route antenna cable to an exterior feed-through or SMA panel connector
- [ ] If built, mount the 137MHz preamp board (2.9) internally near the antenna feed-through, in a way that keeps RF cable runs short
- [ ] Mount Camera 1 (star tracker — Pi Camera Module 3 Wide Angle, CSI port 1):
  - [ ] Position at an exterior-facing point with a clear unobstructed sky view (e.g. lid edge, hinged flap, or top panel cutout)
  - [ ] Confirm it is not obstructed when case is closed
  - [ ] Route CSI ribbon cable internally to Pi CSI port 1
- [ ] Mount Camera 2 (webcam — Pi Camera Module 3 Wide Angle, CSI port 2):
  - [ ] Position at top centre of screen bezel, facing forward — same position as a laptop webcam
  - [ ] Cut a small rectangular slot in the bezel for the lens
  - [ ] Route CSI ribbon cable internally to Pi CSI port 2
- [ ] If integrating the antenna rotator (Phase 4) into the case rather than running it standalone: mount the STM32 internally, route UART to Pi, and either cut a mounting point for the az/el assembly or plan its external cable feed-through
- [ ] If building the gesture-triggered pop-up antenna (4.7): cut the hinge opening for the deploy mechanism into the case wall during this design pass, not afterward

### 5.5 Cable Management & Final Fit
- [ ] Cable management pass:
  - [ ] Bundle/tie loose cables away from moving parts or vents
  - [ ] Confirm no cables block the cooler's airflow or screen hinge movement
- [ ] Final fit check:
  - [ ] Close/assemble case fully — confirm it closes without forcing or pinching cables
  - [ ] All ports you need access to (USB, power, antenna feed-through) are reachable from outside the case
  - [ ] Power on fully assembled unit — confirm nothing is loose, screen still works, keyboard/trackpad still work, Bluetooth mouse still pairs, SDR/camera still function

### 5.6 Panel Mounting & External Ports
Plan the exterior layout, then drill/cut and mount everything. The scripts are already tested from Phase 1.7/1.8 — this section is purely physical installation.

- [ ] Plan button/LED/port layout on case exterior (e.g. side panel): mark positions for power button, gesture toggle button, reset button, status LEDs, USB port, audio jack, SD card reader, ethernet port, antenna feed-through, camera mount point
- [ ] Drill/cut holes in case for each panel-mount item, test-fit before final mounting

**Power button**
- [ ] Mount button on case exterior, label clearly (e.g. "PWR")
- [ ] Run wiring from button to Pi 5's power button header (referencing GPIO pin notes from 1.8)
- [ ] Test: press confirms safe shutdown and boot

**Gesture detection toggle button + LED**
- [ ] Mount button + LED on case exterior near where gestures are made, label (e.g. "GESTURE")
- [ ] Run wiring to the GPIO pins noted in 1.8
- [ ] Test: confirm LED toggles and gesture actions stop/start correctly

**Reset/reboot button (optional)**
- [ ] Mount on case, label (e.g. "RESET"), consider recessing slightly to avoid accidental presses
- [ ] Run wiring to GPIO pin noted in 1.8

**USB port (accessible)**
- [ ] Connect panel-mount USB-A extension to one of the USB hub's ports
- [ ] Mount external USB-A socket on case exterior
- [ ] Test: plug in USB drive, confirm Pi detects it (`lsusb`, `df -h`)

**Audio port (accessible)**
- [ ] Connect panel-mount 3.5mm extension to Pi 5's audio output (via USB-audio adapter if needed — confirm Pi 5 audio output method)
- [ ] Mount external 3.5mm socket on case exterior
- [ ] Test: play audio file, confirm output through headphones

**SD card slot (swappable temp storage)**
- [ ] Connect USB-A to microSD card reader to the USB hub
- [ ] Mount card reader slot accessible from case exterior
- [ ] Test: insert spare microSD, confirm it mounts (`lsblk`)
- [ ] Set up script/shortcut to copy data to this card (e.g. SDR recordings, star tracker images)

**Ethernet port (accessible)**
- [ ] Route Pi 5's onboard ethernet to case exterior via RJ45 panel-mount extension, or cutout for direct access
- [ ] Test: connect cable, confirm connection (`ip a`)

**Final I/O check**
- [ ] With case fully assembled, test every button, LED, and port in one pass
- [ ] Confirm build-log.md GPIO pin assignments are up to date

### 5.7 Document Phase 5
- [ ] Update build-log.md with final case design files/photos, fit issues encountered, and fixes
- [ ] Write a public portfolio summary (`/docs/phase5-case-build.md`): photos of the finished deck, brief note on the case-last approach and why it avoided rework

---

## PHASE 6 (Future/Optional): SBC → SoM Custom Carrier

*Only attempt once Phases 1-5 are stable and you know exactly what I/O you need.*

### 6.1 Requirements Gathering
- [ ] List every peripheral currently connected to the Pi (display, SDR, camera, UART to STM32, power)
- [ ] Note connector types and pin counts needed for each
- [ ] Decide on SoM: Raspberry Pi CM5 (recommended — same software as Pi 5)

### 6.2 Carrier Board Design
- [ ] Research CM5 carrier board reference designs (Raspberry Pi publishes schematics)
- [ ] Use PCB design software (e.g. KiCad) to design carrier board with:
  - CM5 connector
  - Display connector (matching your screen interface)
  - USB ports for SDR, camera (or CSI direct for camera)
  - UART header for STM32 connection
  - Power input + regulation circuitry
- [ ] Get design reviewed (university resources, online PCB design communities)

### 6.3 Fabrication & Assembly
- [ ] Order PCB from fabricator (e.g. JLCPCB, PCBWay)
- [ ] Source components (CM5 module, connectors, passives)
- [ ] Assemble board (solder components — consider uni's lab facilities)

### 6.4 Bring-Up
- [ ] Power on carrier board with CM5, confirm boot
- [ ] Test each peripheral connection one at a time (display, SDR, camera, STM32 UART)
- [ ] Debug any issues (check connections, power rails, signal integrity)

### 6.5 Final Integration
- [ ] Transfer all software/configs from Pi 5 setup to CM5
- [ ] Rebuild final case around new carrier board
- [ ] Full system test: all phases (1-5) working on new hardware

### 6.6 Document Phase 6
- [ ] Publish PCB design files, bring-up notes, final build photos to GitHub repo
- [ ] Write a public portfolio summary (`/docs/phase6-custom-carrier.md`) and final project summary/reflection — this becomes the centerpiece of your portfolio, tie together what each phase taught you and how it relates to space industry skills

---