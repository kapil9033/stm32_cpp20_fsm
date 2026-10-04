#ifndef DRIVERS_DEVICES_HCSR501_HPP
#define DRIVERS_DEVICES_HCSR501_HPP

#include "IGpio.hpp"

namespace drivers::devices {

class HcSr501 {
public:
    explicit HcSr501(const cxx_hal::IGpio& pin) noexcept : pin_(pin) {}

    // Check raw pin state
    [[nodiscard]] bool is_motion_detected() const noexcept {
        return pin_.read() == cxx_hal::GpioState::High;
    }

    // Edge-detection helper for FSM transition dispatching
    [[nodiscard]] bool poll_motion_trigger() noexcept {
        const bool current_state = is_motion_detected();
        const bool triggered = current_state && !last_state_;
        last_state_ = current_state;
        return triggered;
    }

private:
    const cxx_hal::IGpio& pin_;
    bool last_state_{false};
};

} // namespace drivers::devices

#endif // DRIVERS_DEVICES_HCSR501_HPP
