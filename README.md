# STM32F446RE Conveyor and Inspection System

## C++20 Firmware Architecture and State Machine Design

This project describes a type-safe, allocation-conscious firmware architecture for an STM32F446RE-based automated conveyor and inspection system. The design uses modern C++20 abstractions and a state machine to coordinate sensors, actuators, and status displays.

## Table of Contents

- [Project Overview](#project-overview)
- [Hardware Connections and Pinouts](#hardware-connections-and-pinouts)
  - [Component-by-Component Wiring](#component-by-component-wiring)
  - [Power and Logic-Level Notes](#power-and-logic-level-notes)
  - [STM32F446RE Board Pinout Reference](#stm32f446re-board-pinout-reference)
    - [CN5 Digital Header](#cn5-digital-header)
    - [CN6 Power Header](#cn6-power-header)
    - [CN7 Morpho Header](#cn7-morpho-header)
    - [CN8 Analog Header](#cn8-analog-header)
    - [CN9 Digital Header](#cn9-digital-header)
    - [CN10 Morpho Header](#cn10-morpho-header)
- [Firmware Architecture](#firmware-architecture)
- [C++20 Trade-offs for Embedded Firmware](#c20-trade-offs-for-embedded-firmware)

## Project Overview

### System Goal

The planned system uses an ultrasonic sensor to measure distance, a ULN2003 driver and stepper motor to move a tray, an SG90 servo to drop or flag items, and a MAX7219 LED matrix to display state and telemetry. These components are coordinated by a hierarchical finite state machine (HFSM).

### Design Objectives

1. **Avoid dynamic allocation:** Do not use heap allocation (`new`/`malloc`) or dynamically allocating containers such as `std::vector` and `std::string`.
2. **Use C++20 hardware abstractions:** Apply compile-time type safety, `std::string_view`, concepts/type traits, `constexpr`, and strongly typed enums where appropriate.
3. **Use event-driven state transitions:** Organize behavior into states such as `Idle`, `Scanning`, `Processing`, and `Fault`.
4. **Integrate peripheral interfaces:** Use SPI for the MAX7219, timer PWM for the servo, and GPIO/timer input capture or EXTI for sensors and buttons.

## Hardware Connections and Pinouts

The STM32F446RE uses 3.3 V logic. Motors, the LED matrix, and sensors may need a separate power supply. Connect the external supply ground to the STM32 ground.

### Component-by-Component Wiring

#### MAX7219 LED Matrix

| Matrix pin | Connect to | Notes |
| --- | --- | --- |
| VCC | External 5 V supply | Do not power the matrix from 3.3 V. |
| GND | Common ground | Connect to STM32 and external-supply ground. |
| DIN | PA7 (SPI1_MOSI) | SPI data output. |
| CS | PB6 (GPIO output) | Chip select, toggled by firmware. |
| CLK | PA5 (SPI1_SCK) | SPI clock. |

#### HC-SR04 Ultrasonic Sensor

| Sensor pin | Connect to | Notes |
| --- | --- | --- |
| VCC | 5 V supply | Check the sensor module's supply requirements. |
| GND | Common ground | Connect to STM32 and external-supply ground. |
| Echo | PA8, through a resistor divider | Echo is 5 V; reduce it to a 3.3 V-safe level first. |
| Trigger | Not assigned here | Select and document a GPIO output before wiring this signal. |

#### SG90 Servo Motor

| Servo wire | Connect to | Notes |
| --- | --- | --- |
| Red (VCC) | External 5 V supply | Allow for motor current/surge requirements. |
| Black/brown (GND) | Common ground | Connect to STM32 and external-supply ground. |
| Yellow/orange (PWM) | PB0 (TIM3_CH3) | Intended 50 Hz PWM, approximately 1–2 ms pulse width. |

#### ULN2003 Stepper Driver

| Driver pin | Connect to | Notes |
| --- | --- | --- |
| VCC / `+` | External 5 V supply | For the 28BYJ-48 stepper setup. |
| GND / `-` | Common ground | Connect to STM32 and external-supply ground. |
| IN1 | PC0 (GPIO output) | Stepper phase A. |
| IN2 | PC1 (GPIO output) | Stepper phase B. |
| IN3 | PC2 (GPIO output) | Stepper phase C. |
| IN4 | PC3 (GPIO output) | Stepper phase D. |

#### User Button

| Button connection | Connect to | Notes |
| --- | --- | --- |
| Onboard user button | PC13 | Uses the board's onboard button. |
| External button terminal A | GPIO input | Optional external button. |
| External button terminal B | GND | The external-button arrangement assumes an internal pull-up. |

### Power and Logic-Level Notes

- Use a suitable external supply for the LED matrix and motors; do not draw motor current from the STM32 3.3 V rail.
- Connect all grounds together so the signal references are shared.
- **Protect the STM32 input from the HC-SR04 Echo signal.** Echo is 5 V, but PA8 is a 3.3 V input. A divider with 1 kΩ between Echo and PA8 and 2 kΩ between PA8 and ground produces approximately 3.3 V from a 5 V signal. Confirm the divider and voltage levels before powering the circuit.

### STM32F446RE Board Pinout Reference

Each header is listed independently. Pin numbers identify the board's physical header positions.

#### CN5 Digital Header

| Pin | Signal | STM32 pin / peripheral |
| ---: | --- | --- |
| 1 | D8 | PA9 |
| 2 | D9 | PC7 (TIM3_CH2 / TIM8_CH2) |
| 3 | CS / D10 | PB6 (SPI1 chip select) |
| 4 | MOSI / D11 | PA7 (SPI1_MOSI / PWM) |
| 5 | MISO / D12 | PA6 (SPI1_MISO) |
| 6 | SCK / D13 | PA5 (SPI1_SCK / onboard LD2) |
| 7 | GND | Ground |
| 8 | AVDD | Analog VDD reference |
| 9 | SDA / D14 | PB9 (I2C1_SDA) |
| 10 | SCL / D15 | PB8 (I2C1_SCL) |

#### CN6 Power Header

| Pin | Signal | Description |
| ---: | --- | --- |
| 1 | NC | Not connected |
| 2 | IOREF | 3.3 V reference |
| 3 | RESET | NRST MCU reset |
| 4 | +3.3 V | 3.3 V power rail |
| 5 | +5 V | 5 V power rail |
| 6 | GND | Ground |
| 7 | GND | Ground |
| 8 | VIN | External input voltage (7–12 V) |

#### CN7 Morpho Header

| Pin | Signal |
| ---: | --- |
| 1 | PC10 |
| 2 | PC11 |
| 3 | PC12 |
| 4 | PD2 |
| 5 | VDD |
| 6 | E5V |
| 7 | BOOT0 |
| 8 | GND |
| 9 | NC |
| 10 | NC |
| 11 | NC |
| 12 | IOREF |
| 13 | NRST |
| 14 | RESET |
| 15 | 3V3 |
| 16 | 3V3 |
| 17 | 5V |
| 18 | 5V |
| 19 | GND |
| 20 | GND |
| 21 | GND |
| 22 | GND |
| 23 | VIN |
| 24 | NC |
| 25 | NC |
| 26 | PA0 |
| 27 | PA1 |
| 28 | PA4 |
| 29 | PA4 |
| 30 | PB0 |
| 31 | PB0 |
| 32 | PC1 |
| 33 | PC1 |
| 34 | PC0 |
| 35 | PC0 |
| 36 | PD2 |
| 37 | PD2 |
| 38 | PH0 |

#### CN8 Analog Header

| Pin | Signal | STM32 pin / peripheral |
| ---: | --- | --- |
| 1 | A0 | PA0 (ADC1_IN0) |
| 2 | A1 | PA1 (ADC1_IN1) |
| 3 | A2 | PA4 (ADC1_IN4) |
| 4 | A3 | PB0 (ADC1_IN8) |
| 5 | A4 | PC1 (ADC1_IN11) |
| 6 | A5 | PC0 (ADC1_IN10) |

#### CN9 Digital Header

| Pin | Signal | STM32 pin / peripheral |
| ---: | --- | --- |
| 1 | D0 (RX) | PA3 (USART2_RX / ST-LINK VCP) |
| 2 | D1 (TX) | PA2 (USART2_TX / ST-LINK VCP) |
| 3 | D2 | PA10 |
| 4 | D3 | PB3 (TIM2_CH2) |
| 5 | D4 | PB5 |
| 6 | D5 | PB4 (TIM3_CH1) |
| 7 | D6 | PB10 (TIM2_CH3) |
| 8 | D7 | PA8 |

#### CN10 Morpho Header

| Pin | Signal |
| ---: | --- |
| 1 | PC9 |
| 2 | PC8 |
| 3 | PB8 |
| 4 | PC6 |
| 5 | PB9 |
| 6 | PC5 |
| 7 | AVDD |
| 8 | U5V |
| 9 | GND |
| 10 | NC |
| 11 | PA5 |
| 12 | PA12 |
| 13 | PA6 |
| 14 | PA11 |
| 15 | PA7 |
| 16 | PB12 |
| 17 | PB6 |
| 18 | PB11 |
| 19 | PC7 |
| 20 | GND |
| 21 | PA9 |
| 22 | PB2 |
| 23 | PA8 |
| 24 | PB1 |
| 25 | PB10 |
| 26 | PB15 |
| 27 | PB4 |
| 28 | PB14 |
| 29 | PB5 |
| 30 | PB13 |
| 31 | PB3 |
| 32 | AGND |
| 33 | PA10 |
| 34 | PC4 |
| 35 | PA2 |
| 36 | NC |
| 37 | PA3 |
| 38 | NC |

## Firmware Architecture

The firmware is organized around interface-based hardware access and a value-based state machine, rather than procedural `switch` statements tied directly to hardware.

```text
Application Layer
└── Conveyor System FSM Controller
    └── Abstraction Layer
        ├── cxx_hal::IGpio
        ├── cxx_hal::ISpi
        └── cxx_hal::IPwm
            └── Hardware Drivers (STM32 LL/HAL)
                ├── Stm32GpioDriver
                ├── Stm32SpiDriver
                └── Stm32PwmDriver
```

1. **C++ hardware abstractions (`Drivers/cxx_hal/`):** Header-only interfaces such as `IGpio`, `ISpi`, and `IPwm` decouple application logic from hardware details and allow host-side tests with mocks.
2. **Device drivers (`Drivers/devices/`):** Concrete device logic for the MAX7219 display, ultrasonic sensor, and stepper motor.
3. **State machine (`Middleware/fsm/`):** Uses `std::variant` and `std::visit` for type-safe state representation and transitions without requiring dynamic allocation.
4. **Application (`Application/`):** Coordinates the conveyor workflow and its hardware abstractions.

## C++20 Trade-offs for Embedded Firmware

| Advantages | Trade-offs |
| --- | --- |
| **Zero-cost abstractions:** Templates and `constexpr` can move work to compile time and produce efficient code. | **Learning curve:** Templates, concepts, and move semantics add complexity compared with plain C. |
| **Type safety:** Strongly typed enums and types can prevent subtle register-assignment mistakes. | **Toolchain integration:** STM32CubeMX generates C code, so a modern C++ project may need deliberate integration and organization. |
| **Clearer architecture:** Interfaces, RAII, and encapsulation help reduce global state. | **Code-size risk:** Some template use and runtime features can increase binary size. |
| **Host-side testing:** Interfaces make it possible to mock hardware and test state-machine logic on a development machine. | **Embedded constraints:** Features such as `std::function` and `std::vector` may allocate dynamically and should be avoided where heap use is prohibited. |
