# STM32-Based EV Controller and ADAS Firmware

A modular automotive embedded firmware project developed for the **STM32F103C8T6 (Blue Pill)** microcontroller using **STM32 HAL**, **STM32CubeIDE**, and **PICSimLab 1.19.0**.

This project implements a simulated Electric Vehicle (EV) control system integrated with an Advanced Driver Assistance System (ADAS), vehicle state management, fault supervision, ultrasonic obstacle detection, and UART-based telemetry.

The firmware demonstrates how multiple automotive software subsystems can be integrated into a single real-time embedded application while maintaining deterministic execution through timer-driven scheduling.

All functionality is validated through simulation using PICSimLab and the STM32F103C8T6 platform.

---

# Overview

The system continuously processes driver inputs, vehicle operating conditions, and obstacle measurements to emulate the behavior of an electric vehicle equipped with modern driver assistance features.

Core capabilities include:

* Physics-based EV drivetrain simulation
* Drive mode management
* Regenerative braking
* Battery State of Charge (SOC) estimation
* Range prediction
* Forward collision monitoring
* Blind-spot detection
* Parking assistance
* Vehicle state transitions
* Fault detection and protection
* UART telemetry and diagnostics

---
# Simulation Demonstartion

## STM32CubeIDE .ioc Pinout & Configuration

<img width="586" height="575" alt="Screenshot 2026-06-22 183327" src="https://github.com/user-attachments/assets/8d31e66c-7fea-4974-b4a4-58caf713c9b2" />

## STM32CubeIDE .ioc Clock Configuration

<img width="1013" height="726" alt="Screenshot 2026-06-22 183411" src="https://github.com/user-attachments/assets/1a99e800-9bb4-47a9-bf17-4f0e2a30c671" />

## PicSimLab Board

<img width="1918" height="1020" alt="Screenshot 2026-06-22 183443" src="https://github.com/user-attachments/assets/e425d86f-1ba4-4117-ac48-30b0a1b1907b" />

## PicSimLab Components

<img width="1612" height="692" alt="Screenshot 2026-06-22 185238" src="https://github.com/user-attachments/assets/4959787e-8a53-4a0f-b720-8bac497d1254" />

## PicSimLab Output Terminal

### Packet 1 and 2

<img width="112" height="815" alt="Screenshot 2026-06-22 183504" src="https://github.com/user-attachments/assets/2518a248-3d71-4273-99b2-64499e297213" />

### Packet 3 and 4

<img width="220" height="302" alt="Screenshot 2026-06-22 184537" src="https://github.com/user-attachments/assets/3f6d9972-8892-4f0d-a16d-69a5b883988e" />

---

# Firmware Architecture

The firmware is organized into independent modules to improve maintainability, scalability, and separation of responsibilities.

## Driver Modules

### ultrasonic.c / ultrasonic.h

HC-SR04 ultrasonic sensor driver.

Responsibilities:

* Trigger generation
* Echo pulse measurement
* Distance calculation
* Sensor timeout detection
* Sequential multi-sensor scanning
* Cross-talk prevention between sensors

Provides:

```c
HCSR04_Init()
HCSR04_Read()
HCSR04_ReadAll()
```

---

### ev_control.c / ev_control.h

Electric Vehicle control module.

Responsibilities:

* Vehicle speed computation
* Torque calculation
* Motor power estimation
* SOC estimation
* Range prediction
* Drive mode management
* Regenerative braking calculations

Provides real-time EV metrics used by both the state machine and UART telemetry.

---

### adas.c / adas.h

Advanced Driver Assistance System engine.

Responsibilities:

* Front collision detection
* Time-To-Collision (TTC) calculation
* Blind-spot monitoring
* Parking assist logic
* Alarm hysteresis filtering

Processes ultrasonic sensor data and generates ADAS alerts.

---

### fault.c / fault.h

Vehicle fault management module.

Responsibilities:

* Fault monitoring
* Alarm prioritization
* Safety supervision
* Vehicle protection logic
* State transition requests

Supported fault categories:

* Overtemperature Fault
* Low SOC Fault
* Collision Fault
* Sensor Fault
* Communication Fault

---

### uart_shell.c / uart_shell.h

UART communication and diagnostics module.

Responsibilities:

* Telemetry packet generation
* Packet parsing
* Command processing
* Vehicle diagnostics
* Runtime parameter control

Provides a command shell through USART1 operating at 115200 baud.

---

# STM32 Peripheral Utilization

The firmware utilizes the STM32F103C8T6 peripherals as follows:

```text
                  ┌──────────────────────┐
                  │    STM32F103C8T6     │
                  │     (Blue Pill)      │
                  └──────────┬───────────┘
                             │
     ┌───────────────────────┼───────────────────────┐
     ▼                       ▼                       ▼
 [Timers]                 [ADCs]                  [UART]

 TIM1_CH1 (PA8)          ADC1_IN0 (PA0)         USART1 (PA9/PA10)
 └─ Motor PWM Logic      └─ Accelerator         └─ 115200 Baud Shell

 TIM2 (Internal)         ADC1_IN1 (PA1)
 └─ 1 µs Time Base       └─ Brake Pedal

 TIM3 (Internal)         ADC1_IN2 (PA2)
 └─ Scheduler Timer      └─ Drive Mode Selector

 TIM4_CH1 (PB6)          ADC1_IN3 (PA3)
 └─ Reserved PWM         └─ Motor Temperature
```

---

# Features

## Real-Time Cooperative Scheduler

The firmware uses timer-based scheduling to separate time-critical and background tasks.

### Periodic Control Tasks

Responsible for:

* EV calculations
* State machine execution
* Fault supervision
* Safety monitoring

### Background Tasks

Responsible for:

* Ultrasonic measurements
* UART telemetry
* Command shell processing

This architecture ensures predictable execution without requiring an RTOS.

---

## Electric Vehicle Controller

### Drive Modes

| Mode   | Torque Multiplier |
| ------ | ----------------- |
| ECO    | 0.6×              |
| NORMAL | 1.0×              |
| SPORT  | 1.3×              |

### Vehicle Dynamics

The EV model calculates:

* Vehicle speed
* Torque output
* Motor power
* Battery SOC
* Remaining driving range

Vehicle speed is limited to:

```text
200 km/h
```

### Regenerative Braking

Activated whenever brake input exceeds 5%.

Features:

* Negative torque generation
* Energy recovery simulation
* SOC restoration

---

## Advanced Driver Assistance System (ADAS)

### Forward Collision Detection

Warning Condition:

```text
Distance < 50 cm
OR
TTC < 3.0 s
```

Critical Condition:

```text
Distance < 20 cm
OR
TTC < 1.5 s
```

### Blind Spot Detection

Enabled when:

```text
Vehicle Speed > 20 km/h
```

and

```text
Side Distance < 30 cm
```

### Parking Assist

Automatically activated when:

```text
Vehicle Speed < 10 km/h
```

All three ultrasonic sensors remain active for obstacle monitoring.

### Hysteresis Protection

A 300 ms verification window prevents alarm chatter and rapid state oscillation.

---

## Vehicle State Machine

Implemented vehicle states:

```text
PARKED
   ↓
READY
   ↓
DRIVING
   ↔
REGEN
   ↓
FAULT
```

The state machine controls:

* Vehicle startup
* Driving operation
* Regenerative braking
* Fault recovery
* System shutdown

---

## Fault Management

Supported fault types:

* Motor Overtemperature
* Low Battery SOC
* Collision Risk
* Sensor Failure
* Communication Failure

Fault handling actions:

* Vehicle enters FAULT state
* Operation is suspended
* Alarm LEDs are activated
* Telemetry reports the active fault

---

## Alarm Indication System

Visual indicators:

| Alarm Level   | Pin  |
| ------------- | ---- |
| Advisory (P3) | PB8  |
| Warning (P2)  | PB9  |
| Critical (P1) | PB10 |
| Critical (P1) | PB11 |

---

## UART Diagnostics & Telemetry

USART1 Configuration:

```text
115200 Baud
8 Data Bits
No Parity
1 Stop Bit
```

### Packet 0x01 — EV Metrics

Contains:

* Speed
* SOC
* Torque
* Power
* Range

### Packet 0x02 — ADAS Alerts

Contains:

* Front Distance
* Left Distance
* Right Distance
* Collision Level
* Blind Spot Status
* Time-To-Collision (TTC)

### Packet 0x03 — Vehicle State

Sent whenever a state transition occurs.

### Packet 0x04 — ACK

Command acknowledgement packet.

### Supported Commands

```text
mode
speed set
soc set
obstacle
fault inject
fault clear
status
reset
```

---

# Hardware Configuration

## Ultrasonic Sensors

| Sensor | Trigger Pin | Echo Pin |
| ------ | ----------- | -------- |
| Front  | PB0         | PB1      |
| Left   | PB2         | PB3      |
| Right  | PB4         | PB5      |

---

## Analog Inputs

| Function            | Pin |
| ------------------- | --- |
| Accelerator Pedal   | PA0 |
| Brake Pedal         | PA1 |
| Drive Mode Selector | PA2 |
| Motor Temperature   | PA3 |

---

## Status LEDs

| Function     | Pin  |
| ------------ | ---- |
| Advisory LED | PB8  |
| Warning LED  | PB9  |
| Critical LED | PB10 |
| Critical LED | PB11 |

---

# Software Requirements

* STM32CubeIDE
* PICSimLab 1.19.0

---

# Building the Firmware

Open the project in STM32CubeIDE.

Enable binary generation:

1. Right-click the project.
2. Select **Properties**.
3. Navigate to:

```text
C/C++ Build → Settings
```

4. Open:

```text
Tool Settings → MCU Post build outputs
```

5. Enable:

```text
Convert to binary file (-O binary)
```

6. Click **Apply and Close**.
7. Build the project using:

```text
Ctrl + B
```

The generated firmware will be located inside:

```text
<Project Folder>/Debug/
```

as a `.bin` file.

---

# Running the Simulation in PICSimLab

## 1. Launch PICSimLab

Select:

```text
Board → Blue Pill
Microcontroller → STM32F103C8T6
```

---

## 2. Load Firmware

```text
File → Load Hex
```

Select the generated `.bin` file.

---

## 3. Open Spare Parts

Add:

### Input

* Potentiometer Module (4 potentiometers)

### Sensors

* HC-SR04 ×3

### Output

* LED ×4

### Virtual

* IO Virtual Terminal

---

## 4. Connect Components

### Potentiometers

| Potentiometer | Pin |
| ------------- | --- |
| Pot 1         | PA0 |
| Pot 2         | PA1 |
| Pot 3         | PA2 |
| Pot 4         | PA3 |

### Ultrasonic Sensors

| Sensor | Trigger | Echo |
| ------ | ------- | ---- |
| Front  | PB0     | PB1  |
| Left   | PB2     | PB3  |
| Right  | PB4     | PB5  |

### LEDs

| LED   | Pin  |
| ----- | ---- |
| LED 1 | PB8  |
| LED 2 | PB9  |
| LED 3 | PB10 |
| LED 4 | PB11 |

### IO Virtual Terminal

| STM32 Pin | Terminal |
| --------- | -------- |
| PA9       | RX       |
| PA10      | TX       |

UART Configuration:

```text
115200 Baud
8-N-1
```

---

# Technologies Used

* STM32F103C8T6 (Blue Pill)
* Embedded C
* STM32 HAL
* STM32CubeIDE
* PICSimLab 1.19.0
* UART Communication
* ADC Peripheral
* Timer Interrupts
* PWM Generation

---

# Author

**Sushant Kumar**
