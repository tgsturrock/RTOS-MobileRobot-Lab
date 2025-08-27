# ELE543 – Proto1: Mobile Robot (Real-Time Embedded Systems)

This repository contains the code and documentation for **Proto1** of the ELE543 course *Systèmes ordonnés en temps réel*.  
The project involves programming an STM32F051R8 microcontroller to control a small mobile robot in **driving mode** with **obstacle detection** using sonars.

---

## Objectives
- Program and use a microcontroller in a real-time embedded system.
- Learn the architecture of programs handling synchronous and asynchronous events.
- Familiarize with STM32 peripherals: timers, ADC, UART, I2C, GPIO, watchdog.
- Implement communication protocols and motor control in a mobile robot.

---

## System Description
- **Robot platform** with 4 motorized wheels.
- **STM32F0DISCOVERY board** (STM32F051R8 MCU).
- **2 ultrasonic sonars** (I2C communication).
- **RS232 module** for remote control (téléguidage).
- **Software environment**: Atollic TrueStudio (with debugging & analysis tools).
- **Simulation & teleoperation software** (PC side).

---

## Features
- PWM-based motor speed control (regulated with ADC feedback).
- Obstacle detection via alternating sonar PINGs over I2C.
- UART-based remote control (9600 Baud, 8N1).
- LED status indicators for system state and obstacle detection.
- Watchdog monitoring for communication link loss.
- Emergency stop and startup switches.

---

## Specifications
- **Telemetry Frames**: 3 bytes → Command, Speed (0–200), Angle (0–180).
- **Motor Calibration**: automatic at startup, using ADC feedback.
- **Control Loop**: 5 ms tick for motor regulation.
- **Sonar Timing**: ~50 ms alternation, range 1–2 m.

---

## Requirements

- **Hardware:** STM32F051R8 MCU (Nucleo / custom board)  
- **Toolchain:** ARM GCC / STM32CubeIDE / VS Code with Cortex-Debug  
- **Debugger:** ST-Link 
