# STM32F446RE Conveyor and Inspection System

## C++20 Firmware Architecture and State Machine Design

This project describes a type-safe, allocation-conscious firmware architecture for an STM32F446RE-based automated conveyor and inspection system. The design uses modern C++20 abstractions and a state machine to coordinate sensors, actuators, and status displays.

## Table of Contents

- [Project Overview](#project-overview)
- [Hardware Connections and Pinouts](#hardware-connections-and-pinouts)
  - [System Wiring Summary](#system-wiring-summary)
  - [Detailed Component Connections](#detailed-component-connections)
  - [Power and Logic-Level Notes](#power-and-logic-level-notes)
  - [STM32F446RE Board Pinout Reference](#stm32f446re-board-pinout-reference)
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

### System Wiring Summary

| Component | STM32F446RE signal pin(s) | Supply and notes |
| --- | --- | --- |
| MAX7219 LED matrix | PA7 (SPI1 MOSI), PA5 (SPI1 SCK), PB6 (chip select) | 5 V supply; share ground with the STM32 |
| Ultrasonic sensor (HC-SR04) | PA8 (Echo input) | 5 V supply; level-shift Echo before connecting to PA8. A Trigger pin is not assigned in this wiring reference. |
| SG90 servo motor | PB0 (TIM3_CH3 PWM) | 5 V supply |
| ULN2003 stepper driver | PC0, PC1, PC2, PC3 (GPIO) | 5 V supply for the driver/motor |
| User button | PC13 (onboard button) | Internal pull-up |

### Detailed Component Connections

| Component | Component connection | STM32F446RE connection | Notes |
| --- | --- | --- | --- |
| MAX7219 matrix | VCC | External 5 V supply | Do not power the matrix from 3.3 V. |
| MAX7219 matrix | GND | Common ground | Connect to STM32 and external-supply ground. |
| MAX7219 matrix | DIN | PA7 (SPI1_MOSI) | SPI data output. |
| MAX7219 matrix | CS | PB6 (GPIO output) | Chip select, toggled by firmware. |
| MAX7219 matrix | CLK | PA5 (SPI1_SCK) | SPI clock. |
| SG90 servo | Red wire (VCC) | External 5 V supply | Allow for motor current/surge requirements. |
| SG90 servo | Black/brown wire (GND) | Common ground | Connect to STM32 and external-supply ground. |
| SG90 servo | Yellow/orange wire (PWM) | PB0 (TIM3_CH3) | Intended 50 Hz PWM, approximately 1–2 ms pulse width. |
| ULN2003 stepper driver | VCC / `+` | External 5 V supply | For the 28BYJ-48 stepper setup. |
| ULN2003 stepper driver | GND / `-` | Common ground | Connect to STM32 and external-supply ground. |
| ULN2003 stepper driver | IN1 | PC0 (GPIO output) | Stepper phase A. |
| ULN2003 stepper driver | IN2 | PC1 (GPIO output) | Stepper phase B. |
| ULN2003 stepper driver | IN3 | PC2 (GPIO output) | Stepper phase C. |
| ULN2003 stepper driver | IN4 | PC3 (GPIO output) | Stepper phase D. |
| User button | Button terminal A | PC13 (onboard user button) | An external button can also be connected to a GPIO. |
| User button | Button terminal B | GND | The listed external-button arrangement assumes an internal pull-up. |
| HC-SR04 ultrasonic sensor | VCC | 5 V supply | Check the sensor module's supply requirements. |
| HC-SR04 ultrasonic sensor | GND | Common ground | Connect to STM32 and external-supply ground. |
| HC-SR04 ultrasonic sensor | Echo | PA8, through a resistor divider | Echo is 5 V; reduce it to a 3.3 V-safe level first. |
| HC-SR04 ultrasonic sensor | Trigger | Not assigned here | Select and document a GPIO output before wiring this signal. |

### Power and Logic-Level Notes

- Use a suitable external supply for the LED matrix and motors; do not draw motor current from the STM32 3.3 V rail.
- Connect all grounds together so the signal references are shared.
- **Protect the STM32 input from the HC-SR04 Echo signal.** Echo is 5 V, but PA8 is a 3.3 V input. A divider with 1 kΩ between Echo and PA8 and 2 kΩ between PA8 and ground produces approximately 3.3 V from a 5 V signal. Confirm the divider and voltage levels before powering the circuit.

### STM32F446RE Board Pinout Reference

The following reference lists the board-header signal names and pin positions used by the wiring tables above.

#### ST Morpho Headers CN7 and CN10

```text
ST MORPHO HEADER CN7                   |                   ST MORPHO HEADER CN10
---------------------------------------------------------+---------------------------------------------------------
   Pin Name    | Pin # | Left Side  || Right Side | Pin # | Pin Name    | Pin # | Left Side  || Right Side | Pin # | Pin Name
---------------+-------+------------++------------+-------+-------------+-------+------------++------------+-------+------------
     PC10      |   1   |   [ O ]    ||   [ O ]    |   2   |    PC11     |  PC9  |   1   |   [ O ]    ||   [ O ]    |   2   |    PC8
     PC12      |   3   |   [ O ]    ||   [ O ]    |   4   |    PD2      |  PB8  |   3   |   [ O ]    ||   [ O ]    |   4   |    PC6
      VDD      |   5   |   [ O ]    ||   [ O ]    |   6   |    E5V      |  PB9  |   5   |   [ O ]    ||   [ O ]    |   6   |    PC5
     BOOT0     |   7   |   [ O ]    ||   [ O ]    |   8   |    GND      | AVDD  |   7   |   [ O ]    ||   [ O ]    |   8   |    U5V
       NC      |   9   |   [ O ]    ||   [ O ]    |  10   |     NC      |  GND  |   9   |   [ O ]    ||   [ O ]    |  10   |     NC
       NC      |  11   |   [ O ]    ||   [ O ]    |  12   |    IOREF    |  PA5  |  11   |   [ O ]    ||   [ O ]    |  12   |    PA12
      NRST     |  13   |   [ O ]    ||   [ O ]    |  14   |    RESET    |  PA6  |  13   |   [ O ]    ||   [ O ]    |  14   |    PA11
      3V3      |  15   |   [ O ]    ||   [ O ]    |  16   |    3V3      |  PA7  |  15   |   [ O ]    ||   [ O ]    |  16   |    PB12
       5V      |  17   |   [ O ]    ||   [ O ]    |  18   |     5V      |  PB6  |  17   |   [ O ]    ||   [ O ]    |  18   |    PB11
      GND      |  19   |   [ O ]    ||   [ O ]    |  20   |    GND      |  PC7  |  19   |   [ O ]    ||   [ O ]    |  20   |    GND
      GND      |  21   |   [ O ]    ||   [ O ]    |  22   |    GND      |  PA9  |  21   |   [ O ]    ||   [ O ]    |  22   |    PB2
      VIN      |  23   |   [ O ]    ||   [ O ]    |  24   |     NC      |  PA8  |  23   |   [ O ]    ||   [ O ]    |  24   |    PB1
       NC      |  25   |   [ O ]    ||   [ O ]    |  26   |    PA0      | PB10  |  25   |   [ O ]    ||   [ O ]    |  26   |    PB15
      PA1      |  27   |   [ O ]    ||   [ O ]    |  28   |    PA4      |  PB4  |  27   |   [ O ]    ||   [ O ]    |  28   |    PB14
      PA4      |  29   |   [ O ]    ||   [ O ]    |  30   |    PB0      |  PB5  |  29   |   [ O ]    ||   [ O ]    |  30   |    PB13
      PB0      |  31   |   [ O ]    ||   [ O ]    |  32   |    PC1      |  PB3  |  31   |   [ O ]    ||   [ O ]    |  32   |    AGND
      PC1      |  33   |   [ O ]    ||   [ O ]    |  34   |    PC0      | PA10  |  33   |   [ O ]    ||   [ O ]    |  34   |    PC4
      PC0      |  35   |   [ O ]    ||   [ O ]    |  36   |    PD2      |  PA2  |  35   |   [ O ]    ||   [ O ]    |  36   |     NC
      PD2      |  37   |   [ O ]    ||   [ O ]    |  38   |    PH0      |  PA3  |  37   |   [ O ]    ||   [ O ]    |  38   |     NC
```

#### Arduino-Style Headers CN6, CN8, CN5, and CN9

| Header | Pin | Signal | STM32 pin / peripheral or description |
| --- | ---: | --- | --- |
| CN6 | 1 | NC | Not connected |
| CN6 | 2 | IOREF | 3.3 V reference |
| CN6 | 3 | RESET | NRST MCU reset |
| CN6 | 4 | +3.3 V | 3.3 V power rail |
| CN6 | 5 | +5 V | 5 V power rail |
| CN6 | 6 | GND | Ground |
| CN6 | 7 | GND | Ground |
| CN6 | 8 | VIN | External input voltage (7–12 V) |
| CN8 | 1 | A0 | PA0 (ADC1_IN0) |
| CN8 | 2 | A1 | PA1 (ADC1_IN1) |
| CN8 | 3 | A2 | PA4 (ADC1_IN4) |
| CN8 | 4 | A3 | PB0 (ADC1_IN8) |
| CN8 | 5 | A4 | PC1 (ADC1_IN11) |
| CN8 | 6 | A5 | PC0 (ADC1_IN10) |
| CN5 | 1 | D8 | PA9 |
| CN5 | 2 | D9 | PC7 (TIM3_CH2 / TIM8_CH2) |
| CN5 | 3 | CS / D10 | PB6 (SPI1 chip select) |
| CN5 | 4 | MOSI / D11 | PA7 (SPI1_MOSI / PWM) |
| CN5 | 5 | MISO / D12 | PA6 (SPI1_MISO) |
| CN5 | 6 | SCK / D13 | PA5 (SPI1_SCK / onboard LD2) |
| CN5 | 7 | GND | Ground |
| CN5 | 8 | AVDD | Analog VDD reference |
| CN5 | 9 | SDA / D14 | PB9 (I2C1_SDA) |
| CN5 | 10 | SCL / D15 | PB8 (I2C1_SCL) |
| CN9 | 1 | D0 (RX) | PA3 (USART2_RX / ST-LINK VCP) |
| CN9 | 2 | D1 (TX) | PA2 (USART2_TX / ST-LINK VCP) |
| CN9 | 3 | D2 | PA10 |
| CN9 | 4 | D3 | PB3 (TIM2_CH2) |
| CN9 | 5 | D4 | PB5 |
| CN9 | 6 | D5 | PB4 (TIM3_CH1) |
| CN9 | 7 | D6 | PB10 (TIM2_CH3) |
| CN9 | 8 | D7 | PA8 |

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
