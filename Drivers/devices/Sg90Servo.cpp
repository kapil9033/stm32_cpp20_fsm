#include "Sg90Servo.hpp"

namespace drivers::devices {

Sg90Servo::Sg90Servo(cxx_hal::IPwm& pwm_channel) noexcept
    : pwm_channel_(pwm_channel) {}

void Sg90Servo::init() noexcept {
    pwm_channel_.start();
    set_angle(0); // Move to home position
}

void Sg90Servo::set_angle(uint8_t angle_degrees) noexcept {
    if (angle_degrees > 180) {
        angle_degrees = 180;
    }

    current_angle_ = angle_degrees;

    // Linear mapping: Angle (0..180) -> Pulse Width (500us..2500us)
    const uint32_t pulse_us = MIN_PULSE_US + 
        ((static_cast<uint32_t>(angle_degrees) * (MAX_PULSE_US - MIN_PULSE_US)) / 180);

    pwm_channel_.set_pulse_width_us(pulse_us);
}

void Sg90Servo::disable() noexcept {
    pwm_channel_.stop();
}

} // namespace drivers::devices
