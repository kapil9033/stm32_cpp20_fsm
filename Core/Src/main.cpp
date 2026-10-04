#include <cstdint>
#include <limits>
#include <span>

#include "main.h"

#include "StateMachine.hpp"
#include "Max7219.hpp"
#include "HcSr501.hpp"
#include "Sg90Servo.hpp"

#include "ISpi.hpp"
#include "IGpio.hpp"
#include "IPwm.hpp"

extern "C" {
SPI_HandleTypeDef hspi1{};
}

namespace {

constexpr uint16_t MaxSpiTransferSize = std::numeric_limits<uint16_t>::max();

[[noreturn]] void Error_Handler() {
    __disable_irq();
    while (true) {
    }
}

void MX_GPIO_Init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);

    GPIO_InitTypeDef gpio_init{};
    gpio_init.Pin = GPIO_PIN_8;
    gpio_init.Mode = GPIO_MODE_INPUT;
    gpio_init.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    gpio_init.Mode = GPIO_MODE_AF_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_init.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpio_init);

    gpio_init.Pin = GPIO_PIN_6;
    gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
    gpio_init.Pull = GPIO_NOPULL;
    gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpio_init);
}

void MX_SPI1_Init() {
    __HAL_RCC_SPI1_CLK_ENABLE();

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

} // namespace

// Hardware SPI driver implementing cxx_hal::ISpi contract
class TargetSpi : public cxx_hal::ISpi {
public:
    bool transmit(std::span<const uint8_t> tx_data) noexcept override {
        if (tx_data.empty()) {
            return true;
        }
        if (tx_data.size() > MaxSpiTransferSize) {
            return false;
        }

        const HAL_StatusTypeDef status = HAL_SPI_Transmit(
            &hspi1, 
            const_cast<uint8_t*>(tx_data.data()), 
            static_cast<uint16_t>(tx_data.size()), 
            100
        );
        
        return (status == HAL_OK);
    }

    bool receive(std::span<uint8_t> rx_data) noexcept override {
        if (rx_data.empty()) {
            return true;
        }
        if (rx_data.size() > MaxSpiTransferSize) {
            return false;
        }

        const HAL_StatusTypeDef status = HAL_SPI_Receive(
            &hspi1,
            rx_data.data(),
            static_cast<uint16_t>(rx_data.size()),
            100
        );
        return (status == HAL_OK);
    }

    bool transfer(std::span<const uint8_t> tx_data, std::span<uint8_t> rx_data) noexcept override {
        if (tx_data.empty() && rx_data.empty()) {
            return true;
        }
        if (tx_data.empty() || tx_data.size() != rx_data.size() ||
            tx_data.size() > MaxSpiTransferSize) {
            return false;
        }

        const HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(
            &hspi1, 
            const_cast<uint8_t*>(tx_data.data()), 
            rx_data.data(), 
            static_cast<uint16_t>(tx_data.size()), 
            100
        );
        return (status == HAL_OK);
    }
};

// MAX7219 chip select is connected to PB6.
class TargetGpio : public cxx_hal::IGpio {
public:
    TargetGpio(GPIO_TypeDef* port, uint16_t pin) noexcept : port_(port), pin_(pin) {}

    void write(cxx_hal::GpioState state) noexcept override {
        GPIO_PinState pin_state = (state == cxx_hal::GpioState::High) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(port_, pin_, pin_state);
    }

    [[nodiscard]] cxx_hal::GpioState read() const noexcept override {
        GPIO_PinState state = HAL_GPIO_ReadPin(port_, pin_);
        return (state == GPIO_PIN_SET) ? cxx_hal::GpioState::High : cxx_hal::GpioState::Low;
    }

    void toggle() noexcept override {
        HAL_GPIO_TogglePin(port_, pin_);
    }

private:
    GPIO_TypeDef* port_;
    uint16_t pin_;
};

// Target PWM driver implementing cxx_hal::IPwm contract
class TargetPwm : public cxx_hal::IPwm {
public:
    void start() noexcept override {}
    void stop() noexcept override {}

    void set_pulse_width_us(uint32_t pulse_us) noexcept override { (void)pulse_us; }
    void set_duty_cycle(float percentage) noexcept override { (void)percentage; }
};

int main() {
    // 1. STM32 HAL Low-Level Initialization
    HAL_Init();
    MX_GPIO_Init();
    MX_SPI1_Init();
    
    // 2. Instantiate Hardware Interfaces
    TargetSpi spi;
    TargetGpio cs_pin{GPIOB, GPIO_PIN_6};
    TargetGpio pir_output{GPIOA, GPIO_PIN_8};
    TargetPwm pwm;

    // 3. Initialize Drivers and Clear Matrix Display
    drivers::devices::Max7219 matrix{spi, cs_pin};
    drivers::devices::Sg90Servo servo{pwm};

    matrix.init();
    matrix.clear();

    fsm::StateMachine state_machine{matrix, servo};
    drivers::devices::HcSr501 motion_sensor{pir_output};
    bool previous_motion = false;

    while (true) {
        const bool motion_detected = motion_sensor.is_motion_detected();
        if (motion_detected && !previous_motion) {
            state_machine.dispatch(fsm::EventMotionDetected{0});
        } else if (!motion_detected && previous_motion) {
            state_machine.dispatch(fsm::EventTimeout{});
        }
        previous_motion = motion_detected;
        HAL_Delay(20);
    }

    return 0;
}

extern "C" void SysTick_Handler() {
    HAL_IncTick();
}
