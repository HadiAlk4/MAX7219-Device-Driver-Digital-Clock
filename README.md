# MAX7219 Digital Clock & Device Driver

A Linux character device driver (`/dev/max_screen`) and interactive user-space digital clock for the MAX7219 8-digit display on a Raspberry Pi 4.

## Features
* **Kernel Driver:** Bit-banged SPI, hardware brightness control via IOCTLs, and automatic decimal-point cursor tracking.
* **Digital Clock:** Real-time display, flashing seconds, live brightness adjustment (`<`/`>`), switch between `time` and `date`, and a scrolling text alarm (`alarm X`, silenced with `#`).

## Build & Run

### 1. Compile
```bash
# Compile the driver and user applications
make
make digital_clock
make program_test_1
