# ESP32-S3 PlatformIO Template (Arduino Framework) <!-- omit in toc -->

This repository is a standardized starting point for ESP32-S3 embedded projects using the Arduino framework within PlatformIO. It integrates a C++ backend with a compiled Svelte/Vite frontend hosted on the internal flash memory. It is pre-configured for high-memory variants (32MB Flash, 8MB PSRAM) and includes essential boilerplate for networking, OTA updates, and hardware debugging.

## Table of Contents <!-- omit in toc -->

- [Hardware Target](#hardware-target)
- [Pre-Packaged Features](#pre-packaged-features)
- [Project Structure](#project-structure)
- [Quick Start (CLI)](#quick-start-cli)
- [Quick Start (VS Code)](#quick-start-vs-code)
- [Web Frontend & Filesystem (LittleFS)](#web-frontend--filesystem-littlefs)
- [Over-The-Air (OTA) Flashing](#over-the-air-ota-flashing)
- [Debugging Crashes](#debugging-crashes)
- [Pin Map & Hardware Restrictions](#pin-map--hardware-restrictions)
- [Hardware Communication Protocols](#hardware-communication-protocols)
- [Hardware Warnings & Gotchas](#hardware-warnings--gotchas)
- [Dependencies](#dependencies)


## Hardware Target
* **Microcontroller:** ESP32-S3 (Specifically variants with Octal SPI memory)
* **Flash:** 32MB (OPI)
* **PSRAM:** 8MB (OPI)
* **Connection:** Native USB CDC (not UART-to-USB)

## Pre-Packaged Features
* **Custom Partitioning:** Pre-configured `partitions.csv` allocating 8MB for the main app, 8MB for OTA staging, and 14MB for SPIFFS/LittleFS.
* **PSRAM Enablement:** PlatformIO environment flags set to initialize and utilize the 8MB Octal PSRAM via `ps_malloc()`.
* **Native USB Serial:** Configuration flags to ensure `Serial.print()` routes correctly through the native USB port without missing initial boot messages.
* **Over-The-Air (OTA) Updates:** Boilerplate C++ code and PlatformIO configurations to flash new firmware over local Wi-Fi.
* **Exception Decoder:** Automated translation of hexadecimal crash backtraces into exact file and line numbers for rapid debugging.
* **RGB LED Control:** Basic non-blocking heartbeat implementation utilizing the onboard WS2812 NeoPixel.

## Project Structure
* `.pio/` - PlatformIO specific settings.
* `.vscode/` - IDE specific settings.
* `data/` - icons, graphics, and the compiled webpage that will be uploaded to the ESP32.
* `frontend/` - Source code for the Webpage. This is too large to upload before compilation.
* `include/` - C++ header files.
* `lib/` - Project-specific local dependencies.
* `src/main.cpp` - Main application entry point containing Wi-Fi and OTA logic.
* `test/` - Location for PlatformIO integrated Unit Test Cases.
* `partitions.csv` - Custom flash memory map.
* `platformio.ini` - Build environment and hardware configuration.
* `.gitignore` - Prevents tracking of `.pio/` build artifacts.

## Quick Start (CLI)
1. Clone this repository.
2. Open a terminal in the project root.
3. Compile the firmware:
   `pio run`
4. Connect the ESP32-S3 via USB and flash:
   `pio run --target upload`
5. Open the serial monitor:
   `pio device monitor`

## Quick Start (VS Code)
1. Install the **PlatformIO IDE** extension.
2. Open the cloned repository folder in VS Code.
3. Update the `ssid` and `password` variables in `src/main.cpp`.
4. Click the **Build (Checkmark)** icon in the bottom blue status bar.
5. Click the **Upload (Right Arrow)** icon to flash via USB.

## Web Frontend & Filesystem (LittleFS)

This project hosts a compiled Svelte web application directly on the ESP32. **Uploading C++ code does not upload the web files.** The process is split.

### 1. Building the Frontend
The raw Svelte files in the `frontend/` directory cannot be read by a browser directly. They must be compiled and compressed.
1. Open a terminal in the `frontend/` directory.
2. Run `npm install` (only needed the first time).
3. Run `npm run build`.
*Vite is configured to compile the files, compress them into `.gz` format, and dump them into the PlatformIO `data/` directory at the root of the project.*

### 2. Flashing the Filesystem
You must flash the `data/` directory to the ESP32's LittleFS partition. 
1. Connect the ESP32 via USB.
2. Open the PlatformIO sidebar.
3. Select your environment -> `Platform` -> `Build Filesystem Image`.
4. Click `Upload Filesystem Image`.
*Note: If the web page returns VFS errors, the filesystem partition was likely wiped by a mismatched `partitions.csv` upload. Re-upload the filesystem image.*

## Over-The-Air (OTA) Flashing
Once the board is running the initial firmware and connected to your local network, you can push updates wirelessly.
1. Find the ESP32's IP address in the serial monitor.
2. Open `platformio.ini`.
3. Uncomment the OTA upload lines and insert the IP address:
   `upload_protocol = espota`
   `upload_port = 192.168.X.X`
4. Click **Upload**. PlatformIO will compile and transmit the binary over Wi-Fi.

## Debugging Crashes
The project is configured with `build_type = debug` and `monitor_filters = esp32_exception_decoder`. 
If the ESP32 crashes (e.g., panic, watchdog timeout), leave the serial monitor open. PlatformIO will automatically intercept the hex backtrace and print the exact file and line number where the code failed.

## Pin Map & Hardware Restrictions

The ESP32-S3 is highly flexible, allowing most peripherals (UART, SPI, I2C) to be routed to almost any GPIO pin via the GPIO matrix. However, several pins have hardcoded hardware restrictions that will cause boot failures or system crashes if used incorrectly.

### Restricted Pins (Do Not Use for General I/O)
| Pin(s) | Function | Restriction / Reason |
| :--- | :--- | :--- |
| **0, 3, 45, 46** | Strapping Pins | Used by the ESP32 during boot to determine the boot mode (Flash vs. Download). Pulling these high/low externally during startup will prevent the board from booting. |
| **35, 36, 37** | Octal PSRAM | Internally wired to the 8MB PSRAM chip. Using these will crash the memory controller. |
| **39, 40, 41, 42** | JTAG | Reserved for hardware debugging. |
| **43, 44** | USB CDC | Dedicated to the native USB D+ and D- lines. Do not touch if you want serial monitor output over USB. |

### ADC (Analog to Digital Converter) Restrictions
The ESP32-S3 has two ADC units: ADC1 and ADC2. 
**CRITICAL:** When Wi-Fi is enabled, the Wi-Fi driver takes exclusive control of ADC2. Any attempt to `analogRead()` an ADC2 pin while Wi-Fi is active will fail or return garbage data.
* **Always use ADC1 pins for analog sensors:** GPIO 1 through 10.

### Project Pinout Template
*Use this table to map the physical connections for this specific deployment.*

| GPIO | Component | Protocol | Notes / Default State |
| :--- | :--- | :--- | :--- |
| 4 | DS18B20 Temp | One-Wire | Requires 4.7kΩ pull-up resistor to 3.3V. |
| 5 | Soil Moisture | ADC1 | Raw range: ~800 (Wet) to ~2700 (Dry). |
| 8 | I2C SDA | I2C | Shared bus. Requires pull-ups if modules lack them. |
| 9 | I2C SCL | I2C | Shared bus. |
| 10 | SD Card CS | SPI | Chip Select for SD module. |
| 11 | BME280 CS | SPI | Chip Select for Environment sensor. |
| 12 | SPI SCK | SPI | Shared clock line. |
| 13 | SPI MOSI | SPI | Shared data out line. |
| 14 | SPI MISO | SPI | Shared data in line. |
| 15 | HC-SR04 Trig | Digital Out | 3.3V logic to trigger ultrasonic pulse. |
| 16 | HC-SR04 Echo | Digital In | **Requires voltage divider.** Steps 5V sensor output to 3.3V. |
| 17 | NEO-6M RX | UART1 | Connects to TX on the GPS module. |
| 18 | NEO-6M TX | UART1 | Connects to RX on the GPS module. |

## Hardware Communication Protocols

### 1. I2C (Inter-Integrated Circuit)
I2C is a synchronous, multi-master, multi-slave packet-switched protocol. It uses only two wires: Serial Data (SDA) and Serial Clock (SCL). 
* **How it works:** Devices are addressed via fixed 7-bit hexadecimal addresses (e.g., `0x68`). The master controls the clock and requests/sends data to specific addresses.
* **Use Case:** Ideal for connecting multiple digital sensors on a single PCB or within a single small enclosure.
* **Common Devices:** Real-Time Clocks (DS3231), OLED Displays (SSD1306), IMUs (MPU6050), Light Sensors (BH1750), Port Expanders (PCF8574).
* **Effective Range:**
  * **Optimal:** < 10 cm (PCB traces or very short jumper wires).
  * **Maximum:** ~1 meter. Because I2C relies on pull-up resistors to return the data line to a HIGH state, long wires introduce parasitic capacitance. This rounds off the sharp square waves of the clock and data signals, causing communication failures. (Can be extended to ~30 meters using active I2C bus extenders like the PCA9615 over Ethernet cable, but this requires additional hardware).
* **Pros:** Highly pin-efficient (dozens of sensors on 2 pins).
* **Cons:** Susceptible to line capacitance over long wires. Hardware address conflicts occur if two identical sensors share the same default address.
* **Hardware Req:** Both SDA and SCL must be pulled high to 3.3V via resistors (typically 4.7kΩ or 10kΩ). Many breakout boards include these.
* **Suggested Pins:** GPIO 8 (SDA), GPIO 9 (SCL).

> `[Placeholder: Image of I2C logic analyzer trace showing address byte + ACK/NACK]`
> `[Placeholder: Diagram of multiple I2C slaves connected to SDA/SCL with pull-up resistors]`

### 2. SPI (Serial Peripheral Interface)
SPI is a synchronous, full-duplex protocol. It requires four wires: Serial Clock (SCK), Master Out Slave In (MOSI), Master In Slave Out (MISO), and a dedicated Chip Select (CS) pin for every slave device.
* **How it works:** The master generates the clock and pulls a specific device's CS pin LOW to wake it up. Data is simultaneously pushed out on MOSI and read in on MISO.
* **Use Case:** High-speed data transfer where bandwidth is prioritized over pin count. 
* **Common Devices:** SD Card modules, TFT/LCD Screens (ILI9341), Ethernet controllers (W5500), high-speed Environment Sensors (BME280), LoRa radio modules (SX1276).
* **Effective Range:**
  * **Optimal:** < 10 cm. SPI is strictly a board-level protocol.
  * **Maximum:** ~20 cm. Because SPI operates at very high frequencies (often 10MHz to 40MHz+), the signals degrade incredibly fast over wires. Attempting to run SPI over long ribbon cables will result in ringing, cross-talk, and data corruption.
* **Pros:** Extremely fast (often 10MHz+). Full-duplex. No address conflicts.
* **Cons:** Pin-heavy. Every new device requires an additional dedicated GPIO pin for its CS line.
* **Suggested Pins:** GPIO 12 (SCK), 13 (MOSI), 14 (MISO). Assign CS pins to any available standard GPIO.

> `[Placeholder: Image of SPI logic analyzer trace showing CS going low, followed by clock pulses and simultaneous MISO/MOSI data]`

### 3. One-Wire
A proprietary protocol created by Dallas Semiconductor that provides low-speed data and signaling over a single conductor.
* **How it works:** Devices have unique, hardcoded 64-bit ROM addresses. The master sends precise microsecond timing pulses to dictate binary 1s and 0s. 
* **Use Case:** Waterproof temperature monitoring across long distances or multiple nodes where running 4-wire cables is impractical. 
* **Common Devices:** DS18B20 waterproof temperature probes, iButton security keys, EEPROM memory tags (DS2431).
* **Effective Range:**
  * **Optimal:** 1 to 10 meters.
  * **Maximum:** Up to 100 meters. To achieve maximum range, you must use a strong pull-up resistor (e.g., 2.2kΩ instead of 4.7kΩ), lower the data rate, and use twisted-pair cable (like Cat5e) where one wire is Data and the twisted pair is GND to mitigate electromagnetic interference. 
* **Pros:** Requires only one GPIO pin for multiple devices. Excellent over long cable runs.
* **Cons:** Very slow. A 12-bit temperature conversion takes ~750ms and blocks the timing bus.
* **Hardware Req:** Requires a single 4.7kΩ pull-up resistor connecting the data line to 3.3V. Without this, the master cannot read the bus.
* **Suggested Pins:** Any available standard GPIO (e.g., GPIO 4).

> `[Placeholder: Image of One-Wire timing slots showing Master Reset, Presence Pulse, and Read/Write timeslots]`

### 4. UART (Universal Asynchronous Receiver-Transmitter)
UART is an asynchronous, point-to-point hardware protocol. It uses two wires: Transmit (TX) and Receive (RX). 
* **How it works:** Unlike SPI or I2C, there is no clock line. Both devices must agree on a specific timing speed (Baud Rate, e.g., 9600 bps) ahead of time. The TX of device A connects to the RX of device B, and vice versa.
* **Use Case:** Streaming continuous data feeds, communicating between two microcontrollers, or interfacing with complex external modems.
* **Common Devices:** GPS Modules (NEO-6M), Bluetooth Modules (HC-05), Cellular Modems (SIM800L), Fingerprint Scanners, RS-485 adapters.
* **Effective Range:**
  * **Optimal:** < 30 cm.
  * **Maximum:** ~15 meters (at standard 3.3V/5V TTL logic levels and low baud rates like 9600). High baud rates (like 115200) shrink this maximum distance significantly. If you need to send UART data over miles, it must be converted to differential signals via an RS-485 transceiver.
* **Pros:** Simple to implement. Does not require a shared clock line.
* **Cons:** Point-to-point only (one device per serial port). Baud rate mismatches cause complete data corruption.
* **Hardware Req:** TX goes to RX. RX goes to TX. Ensure common ground.
* **Suggested Pins:** ESP32-S3 has 3 hardware UARTs. UART0 is tied to the USB serial monitor. Map UART1 or UART2 to any open standard GPIOs (e.g., GPIO 17 for RX, GPIO 18 for TX).
* **IMPORTANT NOTE:** The NEO-6M GPS Module requires 5V on VCC, not 3.3V.

> `[Placeholder: Image of UART logic analyzer trace showing Start Bit, 8 Data Bits, and Stop Bit]`

### 5. ADC (Analog to Digital Converter)
Not a protocol, but a hardware peripheral that measures analog voltage and converts it to a digital number. The ESP32-S3 has a 12-bit ADC, meaning it outputs values from 0 to 4095.
* **How it works:** Compares the voltage on the pin against an internal reference voltage. 
* **Use Case:** Reading variable resistance or capacitance sensors.
* **Common Devices:** Capacitive Soil Moisture Sensors, Photoresistors (LDRs), Analog Joysticks, Potentiometers, Analog Thermistors.
* **Effective Range:**
  * **Optimal:** < 50 cm.
  * **Maximum:** ~2 meters. Analog DC voltages are highly susceptible to voltage drop over wire length and act as antennas for Electromagnetic Interference (EMI). Running an analog wire near a power cable or motor will induce noise that drastically alters the ADC reading. Use shielded cables if extending beyond a meter.
* **Pros:** Simple, passive reading.
* **Cons:** The ESP32 ADC is notoriously non-linear, especially near 0V and 3.3V. It is noisy and requires software averaging/calibration for reliable data.
* **Hardware Req:** Must ensure the analog voltage never exceeds the ESP32's 3.3V limit.
* **Suggested Pins:** ONLY ADC1 pins (GPIO 1 - 10).

## Hardware Warnings & Gotchas

1. **3.3V vs 5V Logic:** The ESP32 is strictly a 3.3V device. It is not 5V tolerant. If you power a module (like the HC-SR04 ultrasonic sensor) with 5V, its data lines will output 5V. You MUST use a voltage divider (e.g., 1kΩ and 2kΩ resistors) on incoming logic lines to drop the voltage to 3.3V before it hits the ESP32.
2. **I2C Address Conflicts:** The DS3231 RTC and the MPU-6050 IMU both share the hardcoded I2C address `0x68`. They cannot exist on the same bus default. You must pull the `AD0` pin on the MPU-6050 breakout board to 3.3V to shift its address to `0x69`.
3. **NTP Blocking:** The system requires real-world time to log data accurately. If Wi-Fi connects but the internet is down, the NTP sync function will block the main loop until it times out. 

## Dependencies

Listed below are the core libraries required in `platformio.ini` and their specific roles in the architecture.

* `bblanchon/ArduinoJson` - Handles serialization of C++ structs into JSON strings for WebSocket transmission, and deserializes incoming web configurations.
* `paulstoffregen/OneWire` - The core timing library for the 1-Wire protocol.
* `milesburton/DallasTemperature` - Wraps the OneWire library specifically to extract Celsius/Fahrenheit data from DS18B20 probes.
* `adafruit/Adafruit BME280 Library` - Handles the SPI register math and calibration data extraction for the BME280 temp/humidity/pressure sensor.
* `adafruit/RTClib` - Interfaces with the DS3231 module and handles leap year/date math natively.
* `claws/BH1750` - Manages I2C requests to the GY-302 light sensor.
* `mikalhart/TinyGPSPlus` - Parses continuous raw NMEA text strings over UART from the NEO-6M GPS module into usable coordinate objects.