# 📝 Build Log

## 20/06/2026
- Purchased Raspberry Pi 5 8GB (Scorptec bundle with Pi 5 & official power supply, and active cooler)
  - Chosen for its performance, active community, compatibility with project goals, and price point (from a good deal)

## 22/06/2026
- Set up GitHub repo structure (most of 1.2 from project guide)
- Add & start BOM

## 24/06/2026
- Complete the initial hardware setup (1.3 from project guide)
  - Install the active cooler on the Pi 5
  - Set up Pi 5 with spare keyboard, mouse, display & ethernet to test the set up works
- Complete the initial software setup (1.4 from project guide)
- Complete installing the core software (1.5 from project guide)
- Test the software parts of the build & the Bluetooth module (a bit of 1.6 from project guide)

## 25/06/2026
- Flesh out ideas for the system design (e.g. protection circuits, charging circuits, low-power, RTC, LED strips, specific LED statuses, sound, etc.)

## 26/06/2026
- Flesh out ideas for the system design (e.g. sound, USB hub ports, etc.)

## 23/07/2026
- Wire VL53L4CX to STM32 I2C pins (SDA, SCL, XSHUT, GPIO1, VDD, GND) — refer to System Design Reference for pull-up/protection values
- Set up STM32CubeMX project for the ToF sensor, including setting up interrupt on GPIO1 pin — sensor signals data-ready instead of polling
- Get a blinking LED test working on the STM32 to confirm it has been set up correctly
- Set up JLink RTT Viewer for debugging JLink printf output via WSL

## 24/07/2026
- Ensure RTT works by printing a test statement
- Write ToF initialisation and ranging code using the VL53L4CX driver API
