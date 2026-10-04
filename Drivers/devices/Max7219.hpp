#ifndef DRIVERS_DEVICES_MAX7219_HPP
#define DRIVERS_DEVICES_MAX7219_HPP

#include "ISpi.hpp"
#include "IGpio.hpp"
#include <cstdint>
#include <array>
#include <span>

namespace drivers::devices {

class Max7219 {
public:
    // MAX7219 Command Registers
    enum class Register : uint8_t {
        NoOp        = 0x00,
        Digit0      = 0x01,
        Digit1      = 0x02,
        Digit2      = 0x03,
        Digit3      = 0x04,
        Digit4      = 0x05,
        Digit5      = 0x06,
        Digit6      = 0x07,
        Digit7      = 0x08,
        DecodeMode  = 0x09,
        Intensity   = 0x0A,
        ScanLimit   = 0x0B,
        Shutdown    = 0x0C,
        DisplayTest = 0x0F
    };

    Max7219(cxx_hal::ISpi& spi_bus, cxx_hal::IGpio& cs_pin) noexcept;

    // Device Initialization
    bool init() noexcept;

    // Configuration Methods
    void set_intensity(uint8_t level) noexcept; // 0 (min) to 15 (max)
    void clear() noexcept;
    void power_save(bool enable) noexcept;

    // Display rendering
    void set_row(uint8_t row_index, uint8_t byte_pattern) noexcept;
    void display_buffer(std::span<const uint8_t, 8> buffer) noexcept;

private:
    cxx_hal::ISpi& spi_bus_;
    cxx_hal::IGpio& cs_pin_;
    std::array<uint8_t, 8> frame_buffer_{};

    void write_register(Register reg, uint8_t data) noexcept;
};

} // namespace drivers::devices

#endif // DRIVERS_DEVICES_MAX7219_HPP
