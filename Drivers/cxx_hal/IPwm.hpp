#ifndef CXX_HAL_IPWM_HPP
#define CXX_HAL_IPWM_HPP

#include <cstdint>

namespace cxx_hal {

class IPwm {
public:
    virtual ~IPwm() = default;

    virtual void start() noexcept = 0;
    virtual void stop() noexcept = 0;

    // Pulse width setting in microseconds (µs)
    virtual void set_pulse_width_us(uint32_t pulse_us) noexcept = 0;

    // Duty cycle setting in percentage (0.0f - 100.0f)
    virtual void set_duty_cycle(float percentage) noexcept = 0;
};

} // namespace cxx_hal

#endif // CXX_HAL_IPWM_HPP
