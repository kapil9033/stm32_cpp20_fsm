#ifndef DRIVERS_DEVICES_SG90_SERVO_HPP
#define DRIVERS_DEVICES_SG90_SERVO_HPP

#include "IPwm.hpp"
#include <cstdint>

namespace drivers::devices {

class Sg90Servo {
public:
    // Standard SG90 servo pulse width constraints in microseconds at 50 Hz
    static constexpr uint32_t MIN_PULSE_US = 500;  // 0 degrees (~0.5 ms)
    static constexpr uint32_t MAX_PULSE_US = 2500; // 180 degrees (~2.5 ms)

    explicit Sg90Servo(cxx_hal::IPwm& pwm_channel) noexcept;

    void init() noexcept;
    void set_angle(uint8_t angle_degrees) noexcept;
    void disable() noexcept;

    [[nodiscard]] uint8_t get_current_angle() const noexcept { return current_angle_; }

private:
    cxx_hal::IPwm& pwm_channel_;
    uint8_t current_angle_{0};
};

} // namespace drivers::devices

#endif // DRIVERS_DEVICES_SG90_SERVO_HPP
