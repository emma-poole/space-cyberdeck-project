# Space-Themed Cyberdeck Project Guide
> **Note:** This guide was generated with AI assistance and is intended as a starting point for learning purposes. Steps will be updated and verified as the build progresses.

A full step-by-step build guide: SBC prototype → daily-use deck → satellite ground station → CV star tracker → (optional) custom SoM carrier + antenna rotator.

This guide includes both a private `build-log.md` (your detailed working notes) and prompts to write public-facing portfolio summaries — suitable for sharing on GitHub/LinkedIn.

---

## PHASE 1: Core Cyberdeck Build (SBC)

### 1.1 Order Parts
- [x] Raspberry Pi 5 (8GB) — **purchased** (Scorptec bundle w/ official power supply)
- [x] Active cooler for Pi 5 — **purchased**
- [ ] microSD card, 128GB, A2 rated (~$20 AUD) — Jaycar/Officeworks/Amazon AU (Sandisk or Samsung)
- [ ] Display — size/type TBD (see decision note below)
- [ ] USB-C PD power bank — **must explicitly list 5V/5A (25W) output**, not just high mAh (~$40-60 AUD) — look for "100W PD power bank" listings and check the spec sheet for the 5V/5A profile before buying; standard phone power banks (5V/2-3A) will undervolt the Pi 5 under load
- [ ] Mechanical keyboard (60% USB) or salvaged keyboard, wired (~$0-40 AUD) — Jaycar/Facebook Marketplace/JB Hi-Fi
- [ ] USB trackpad module (small, panel-mountable, built into case) (~$20-30 AUD) — AliExpress, search "USB trackpad module DIY"
- [ ] Bluetooth mini mouse (~$15-25 AUD) — for portable/away-from-deck use; pairs to Pi 5's built-in Bluetooth, no USB port used
- [ ] Powered USB hub, 4+ port (~$15-20 AUD) — keyboard + trackpad + SDR + card reader use the Pi 5's native USB ports; hub gives expansion room for future peripherals (mouse is Bluetooth, so doesn't compete for a USB slot)
- [ ] USB-A to USB-C cable (if power bank doesn't include one) (~$5 AUD)
- [ ] Momentary push buttons x3-4 (panel-mount, ~$1-2 each) — AliExpress, search "panel mount momentary push button"
- [ ] Status LEDs x2-3 + 330Ω resistors (~$2 total) — AliExpress, search "5mm LED assorted kit"
- [ ] Panel-mount USB-A extension (~$5-8 AUD) — AliExpress
- [ ] Panel-mount 3.5mm audio extension cable (~$3-5 AUD) — AliExpress
- [ ] USB-A to microSD card reader, low-profile (~$8-10 AUD) — AliExpress/Jaycar
- [ ] (Optional) RJ45 panel-mount extension cable (~$5-10 AUD) — Pi 5 has onboard Gigabit Ethernet; this routes it to case exterior
- [ ] ToF sensor (confirm model, e.g. VL53L1X or VL53L5CX) — already owned

**Screen size decision (do this before ordering display):**
- [ ] Decide: more laptop-like (7-10", easier to read/code, less portable) vs. more compact/handheld (5-7", easier to carry, smaller text)
- [ ] Once size is chosen, confirm it's HDMI input (simplest, universal) rather than DSI-only
- [ ] Order display once size/interface confirmed

### 1.2 Initial Software Setup
- [ ] Download Raspberry Pi Imager on your laptop (raspberrypi.com/software)
- [ ] Flash Raspberry Pi OS (64-bit, Desktop version) onto the microSD card
  - In Imager: click "Edit Settings" before flashing — set hostname, enable SSH, set username/password, configure wifi (saves doing this on first boot)
- [ ] Insert microSD into Pi 5, connect display, keyboard, power
- [ ] Boot the Pi — confirm it reaches the desktop
- [ ] Connect to wifi (if not pre-configured)
- [ ] Open terminal, run:
  ```
  sudo apt update && sudo apt full-upgrade -y
  sudo reboot
  ```

### 1.3 Install Core Software
- [ ] Install VS Code:
  ```
  sudo apt install code -y
  ```
  (if not found, follow official VS Code ARM64 .deb install instructions)
- [ ] Confirm Chromium is installed (usually pre-installed): `chromium-browser --version`
- [ ] Install Git:
  ```
  sudo apt install git -y
  ```
- [ ] Configure Git identity:
  ```
  git config --global user.name "Your Name"
  git config --global user.email "your@email.com"
  ```
- [ ] (Optional) Install LibreOffice:
  ```
  sudo apt install libreoffice -y
  ```

### 1.4 Test the Build
- [ ] Open VS Code, create a test file, confirm editing works smoothly
- [ ] Open Chromium, browse a few sites, confirm performance is acceptable
- [ ] Test touchscreen responsiveness (if using touchscreen)
- [ ] Test keyboard — all keys register correctly
- [ ] Test trackpad — registers correctly, no conflicts
- [ ] Pair Bluetooth mouse: Pi desktop → Bluetooth icon → scan → select mouse → confirm pairing; test click/movement
- [ ] Check battery runtime with power bank (note down approx hours)

### 1.5 Case (Initial)
- [ ] Sketch rough layout on paper: top-down view showing screen position, keyboard position, trackpad position, Pi location, battery location, USB hub location, cable routing paths
- [ ] Measure physical dimensions of each component (screen, Pi board incl. cooler height, keyboard, trackpad, battery pack, USB hub) — note width/length/height in mm for each
- [ ] Decide enclosure approach: 3D printed (design in Fusion360/Tinkercad/FreeCAD), laser-cut panels, or repurposed enclosure (e.g. old hardware case/toolbox)
- [ ] If 3D printing: design/find a base plate with mounting holes matching Pi 5's hole pattern (49mm x 58mm spacing) and screen's mounting holes
- [ ] Print or cut a test/prototype version first (doesn't need to be final material — cardboard mockup is fine for layout check)
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
  - [ ] Connect keyboard, trackpad, and leave remaining hub ports free for SDR/card reader/future peripherals
- [ ] Bluetooth mouse needs no mounting — just keep it nearby/clipped to the case exterior if you want it always at hand (e.g. small velcro strap)
- [ ] Mount the battery/power bank:
  - [ ] Place in a location with weight balance in mind (avoid top-heavy/unstable result)
  - [ ] Secure with a strap, velcro, or printed bracket (must not shift when device is moved)
  - [ ] Route USB-C power cable to Pi's power input
- [ ] Cable management pass:
  - [ ] Bundle/tie loose cables away from moving parts or vents
  - [ ] Confirm no cables block the cooler's airflow or screen hinge movement
- [ ] Final fit check:
  - [ ] Close/assemble case fully — confirm it closes without forcing or pinching cables
  - [ ] All ports you need access to (USB for SDR/camera later, power) are reachable from outside the case
  - [ ] Power on fully assembled unit — confirm nothing is loose, screen still works, keyboard/trackpad still work, Bluetooth mouse still pairs

### 1.6 Gesture Detection & Power Management (ToF Sensor)
- [ ] Confirm ToF sensor model (e.g. VL53L1X/VL53L5CX) and connect via I2C to Pi
- [ ] Enable I2C on Pi:
  ```
  sudo raspi-config
  ```
  → Interface Options → I2C → Enable
- [ ] Install Python I2C libraries (e.g. `pip install vl53l1x` or manufacturer-specific library)
- [ ] Write test script: read raw distance values, confirm sensor responds to hand movement
- [ ] Build basic gesture logic: detect swipe (distance changes across zones/over time) and hand near/far
- [ ] Map gestures to actions (e.g. swipe = switch desktop, hand near = wake screen, hand far + hold = sleep)
- [ ] Test gestures reliably trigger correct actions

**Other gesture ideas (software-only, no motors — easy additions once core gestures work):**
- [ ] Gesture-controlled volume/brightness: swipe up/down to adjust system volume or screen brightness
- [ ] Gesture-triggered status display: wave to cycle a small status readout (battery %, current/next satellite pass time) — can be shown on the main screen or a small secondary OLED if you add one later
- [ ] Gesture-controlled antenna manual nudge (Phase 4): once the rotator exists, use a gesture as a manual override to nudge az/el a few degrees without waiting for auto-tracking

**Stretch: MCU-based wake controller (power management)**
- [ ] Move ToF sensor to a small always-on MCU (e.g. low-power STM32 or similar)
- [ ] MCU continuously polls ToF sensor in low-power mode (µA-level draw)
- [ ] On gesture detection, MCU triggers Pi wake (e.g. via GPIO pulling Pi's run pin, or signalling power bank to enable output)
- [ ] Pi normally sits in suspend/low-power state when not in use, only fully powers on when woken
- [ ] Document power draw comparison: Pi always-on vs MCU-watching + Pi suspended
- [ ] Note in build-log: this is a real spacecraft power-budget pattern — low-power "housekeeping" controller manages when higher-power systems wake up

### 1.7 Physical Buttons, LEDs & External Ports
- [ ] Plan button/LED/port layout on case exterior (e.g. side panel): mark positions for power button, gesture toggle button, reset button, status LEDs, USB port, audio jack, SD card reader, ethernet port
- [ ] Drill/cut holes in case for each panel-mount item, test-fit before final mounting

**Power button (safe shutdown + boot)**
- [ ] Wire a momentary push button to Pi 5's power button header (GPIO pins labelled for power, check Pi 5 documentation for exact pins — typically a 2-pin header near USB-C)
- [ ] Test: single press while off → boots Pi; press while on → triggers safe shutdown (default Pi 5 behavior, no extra software needed for basic on/off)
- [ ] Mount button on case exterior, label clearly (e.g. "PWR")

**Gesture detection toggle button**
- [ ] Wire a second momentary push button to a free Pi GPIO pin (e.g. GPIO17) with a pull-up resistor (or use Pi's internal pull-up in software)
- [ ] In your gesture detection script (from 1.6), add logic:
  - [ ] On startup, set a flag `gesture_enabled = True`
  - [ ] Poll the button GPIO; on each press (debounced), toggle `gesture_enabled`
  - [ ] Only run gesture logic when `gesture_enabled` is True
- [ ] Wire a status LED (with appropriate resistor, e.g. 330Ω) to another GPIO pin
  - [ ] LED on = gesture detection active, LED off = disabled
  - [ ] Update LED state in script whenever toggle changes
- [ ] Mount button + LED on case exterior near where gestures are made, label (e.g. "GESTURE")
- [ ] Test: press button, confirm LED toggles and gesture actions stop/start accordingly

**Reset/reboot button (optional but useful)**
- [ ] Wire a third momentary button to another GPIO pin
- [ ] Write a small background script that triggers `sudo reboot` when held 2+ seconds (avoids accidental reboots)
- [ ] Mount on case, label (e.g. "RESET"), consider recessing slightly

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
- [ ] With case assembled, test every button, LED, and port in one pass
- [ ] Update build-log.md with GPIO pin assignments for all buttons/LEDs (reference for later)

### 1.8 Documentation Setup (Private + Public)
- [x] Create a GitHub repository (e.g. "space-cyberdeck")
- [x] Write README.md with project goal (1 paragraph) — this is the first thing anyone (including LinkedIn visitors) will see, so keep it clear and welcoming
- [x] Create a `build-log.md` file — your private/detailed working notes, commit here after each session (what you did, what broke, how you fixed it)
- [x] Create a `/docs` or `/portfolio` folder for public-facing summaries — shorter, polished write-ups per phase (what it does, why it's interesting, a photo/demo) suitable for linking from LinkedIn
- [x] Create a `/photos` folder for build photos as you go
- [ ] Add a short "Project Overview" section to README covering: what the deck does, what skills it demonstrates, and links to each phase's portfolio write-up (you'll fill these in as you complete each phase)

---

## PHASE 2: Satellite Tracking & SDR Ground Station

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

---

## PHASE 3: CV Star Tracker

### 3.1 Order Parts
- [ ] Raspberry Pi Camera Module (wide FOV preferred) (~$25-35 AUD)
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

### 4.1 Order Parts
- [ ] STM32 Nucleo board (reuse from ENGG3800 if available)
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
*A small, contained motor project — antenna sits stowed flat when not in use, gesture (from your Phase 1 ToF sensor) deploys it upright.*
- [ ] Design simple deploy mechanism: a single hinge point with a small servo or DC motor providing the lift force (much lighter/simpler than a full rotator — just one motion, stowed to upright)
- [ ] Add a limit switch or basic position sensing at the "deployed" end-stop, so the motor knows when to stop (avoids over-driving and stalling the motor)
- [ ] Wire motor + limit switch to STM32 (can reuse Phase 4.3's STM32 setup, or run on a separate small MCU if you want it independent of the rotator)
- [ ] Add a new FreeRTOS task (or simple state machine if not using RTOS here): listens for "deploy" signal from Pi (triggered by gesture sensor), drives motor to deployed position, stops at limit switch
- [ ] Add a "stow" trigger too (second gesture, or a software command) — reverses motor to flat position
- [ ] Test: trigger gesture, confirm antenna reliably deploys and stows without binding or stalling
- [ ] Document in build-log.md: mechanism design choices, what you learned about motor control/torque sizing for a real moving mechanism

---

## PHASE 5 (Future/Optional): SBC → SoM Custom Carrier

*Only attempt once Phases 1-4 are stable and you know exactly what I/O you need.*

### 5.1 Requirements Gathering
- [ ] List every peripheral currently connected to the Pi (display, SDR, camera, UART to STM32, power)
- [ ] Note connector types and pin counts needed for each
- [ ] Decide on SoM: Raspberry Pi CM5 (recommended — same software as Pi 5)

### 5.2 Carrier Board Design
- [ ] Research CM5 carrier board reference designs (Raspberry Pi publishes schematics)
- [ ] Use PCB design software (e.g. KiCad) to design carrier board with:
  - CM5 connector
  - Display connector (matching your screen interface)
  - USB ports for SDR, camera (or CSI direct for camera)
  - UART header for STM32 connection
  - Power input + regulation circuitry
- [ ] Get design reviewed (university resources, online PCB design communities)

### 5.3 Fabrication & Assembly
- [ ] Order PCB from fabricator (e.g. JLCPCB, PCBWay)
- [ ] Source components (CM5 module, connectors, passives)
- [ ] Assemble board (solder components — consider uni's lab facilities)

### 5.4 Bring-Up
- [ ] Power on carrier board with CM5, confirm boot
- [ ] Test each peripheral connection one at a time (display, SDR, camera, STM32 UART)
- [ ] Debug any issues (check connections, power rails, signal integrity)

### 5.5 Final Integration
- [ ] Transfer all software/configs from Pi 5 setup to CM5
- [ ] Rebuild final case around new carrier board
- [ ] Full system test: all phases (1-4) working on new hardware

### 5.6 Document Phase 5
- [ ] Publish PCB design files, bring-up notes, final build photos to GitHub repo
- [ ] Write a public portfolio summary (`/docs/phase5-custom-carrier.md`) and final project summary/reflection — this becomes the centerpiece of your portfolio, tie together what each phase taught you and how it relates to space industry skills

---

## Ongoing Throughout Project
- [ ] Commit to build-log.md after every session (even short notes)
- [ ] Take photos at each stage for documentation
- [ ] Keep a running "issues encountered" list with solutions — this is valuable for write-ups and interviews later
- [ ] After each phase's portfolio summary is written, consider a short LinkedIn post linking to it — builds a visible trail of progress over time, not just one big reveal at the end
