
## Mobile Robot (Real-Time Embedded Systems)

This repository contains the code and documentation for the Proto1 project of the ELE543 course *Systèmes ordonnés en temps réel* (Real-Time Embedded Systems). The project's goal is to program an **STM32F051R8 microcontroller** to control a small mobile robot, enabling it to operate in driving mode with **real-time obstacle detection** using sonar sensors.

### Objectives

* Program and use a microcontroller in a real-time embedded system.
* Learn the architecture of programs that handle synchronous and asynchronous events.
* Familiarize with key STM32 peripherals: Timers, ADC, UART, I2C, and GPIO.
* Implement communication protocols and motor control in a mobile robot.

---

### System Description

The robot platform features a compact design with four motorized wheels. Its core is an STM32F0DISCOVERY board powered by an STM32F051R8 MCU. The robot interacts with its environment using two ultrasonic sonar sensors and a remote control via an RS232 module.

* **Robot Platform:** A mobile base with four motorized wheels.
* **Microcontroller:** STM32F0DISCOVERY board (STM32F051R8 MCU).
* **Sensors:** Two ultrasonic sonars for obstacle detection (I2C communication).
* **Communication:** An RS232 module for remote control (teleoperation).

---

### Software Architecture

The software is structured into distinct modules, each responsible for a specific function, which allows for a clean separation of concerns and facilitates real-time operation.

#### Peripheral Drivers

These modules handle the low-level interaction with the robot's hardware components.

* **`adc.c`**: Manages the **A**nalog-to-**D**igital **C**onverter, measuring motor speed by reading voltage feedback from the motors.
* **`pwm.c`**: Controls the robot's locomotion by generating **P**ulse-**W**idth **M**odulation signals for the motors and setting their direction.
* **`i2c.c`**: Implements the **I**nter-**I**ntegrated **C**ircuit driver for communication with I2C-based sensors, such as the sonar modules.
* **`usart.c`**: Handles serial communication for receiving wireless commands from a remote control via a state machine.

#### Central Control

The `control.c` module serves as the robot's brain. It processes data from the sonar sensors to perform **basic obstacle avoidance**. It also uses motor speed data from the ADC to adjust motor output and ensure the robot moves as intended. All critical information, such as desired speed and sensor readings, is managed in a central data structure to maintain a consistent state.

#### Main Loop & Execution

The project's execution is managed by the `main.c` file. It initializes all hardware and then enters an infinite loop. This loop's timing is critical and is controlled by a **5ms timer interrupt**. This ensures that all sensor readings, command processing, and motor updates happen at a precise and consistent rate, making the robot's behavior predictable and reliable.

---

### Features & Specifications

* **Motor Control:** PWM-based motor speed control regulated by ADC feedback.
* **Obstacle Detection:** Alternating sonar PINGs over I2C.
* **Remote Control:** UART communication at 9600 Baud (8N1 protocol).
* **Calibration:** Automatic motor calibration at startup using ADC feedback.
* **Control Loop:** A 5 ms tick for all control and regulation tasks.
* **Sonar Timing:** A ~50 ms alternation rate with a dynamic detection range of 1–2 meters.
* **Status Indicators:** LED status indicators for system state and obstacle detection.

---

### Requirements

* **Hardware:** STM32F051R8 MCU (Nucleo / custom board)
* **Toolchain:** ARM GCC / STM32CubeIDE / VS Code with Cortex-Debug
* **Debugger:** ST-Link
