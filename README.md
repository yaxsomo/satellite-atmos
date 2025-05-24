# Satellite Atmos Software

Welcome to the **Atmos Software** repository!  
This software powers the **Atmos** module, a compact, high-precision atmospheric monitoring system designed for satellites, high-altitude balloons, and scientific missions.  

It enables real-time data acquisition, storage, and transmission from a suite of integrated and external sensors, providing critical atmospheric insights for research, engineering, and environmental applications.

---

## ✨ Features

- 📦 Modular software architecture for easy extension and maintenance
- 📡 Support for both onboard and external sensor integration
- 📂 Data storage on SD card and optional telemetry transmission
- 🔌 Plug-and-play connection system for external sensors
- 🛡️ Optimized for low-power, embedded STM32 microcontroller systems

---

## 🧪 Supported Sensors

**Onboard (PCB-integrated):**
- MS5607 (barometric pressure & altitude sensor)
- MICS-5524 (gas sensor)

**External (via connectors):**
- SDS011 (particulate matter sensor)
- ENS160 (air quality sensor)
- AHT21 (temperature & humidity sensor)

---

## ⚙️ Requirements

- STM32L476RGT6 microcontroller  
- SD card (FAT32 formatted)  
- Connected sensors (as needed for your mission)  
- STM32CubeIDE or equivalent development environment  

---

## 🏗️ Building & Usage

1. Clone the repository:
   ```bash
   git clone https://github.com/YourOrganization/satellite-atmos-software.git
   ```
2. Open the project in STM32CubeIDE.
3. Configure the project settings if needed (check `configuration.h` for hardware-specific options).
4. Compile and flash the firmware to your Atmos module.
5. Connect your sensors (if external) following the provided pinout.
6. Power the module and monitor data via SD card or UART interface.

---

## 📍 Interfaces

![Interfaces](path/to/your/pinout-image.png)


---

## 📝 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
