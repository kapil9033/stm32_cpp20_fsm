#include "Max7219.hpp"

namespace drivers::devices {

Max7219::Max7219(cxx_hal::ISpi& spi_bus, cxx_hal::IGpio& cs_pin) noexcept
    : spi_bus_(spi_bus), cs_pin_(cs_pin) {
    cs_pin_.write(cxx_hal::GpioState::High);
}

bool Max7219::init() noexcept {
    // 1. Exit display test mode
    write_register(Register::DisplayTest, 0x00);

    // 2. Set scan limit to display all 8 digits/rows
    write_register(Register::ScanLimit, 0x07);

    // 3. Disable BCD decode mode (raw matrix pixel mapping)
    write_register(Register::DecodeMode, 0x00);

    // 4. Set medium brightness level (5/15)
    set_intensity(5);

    // 5. Clear display buffer
    clear();

    // 6. Enable display (normal operation)
    power_save(false);

    return true;
}

void Max7219::write_register(Register reg, uint8_t data) noexcept {
    // MAX7219 expects a 16-bit frame: [Address (8-bit) | Data (8-bit)]
    const std::array<uint8_t, 2> tx_data = {
        static_cast<uint8_t>(reg),
        data
    };

    cs_pin_.write(cxx_hal::GpioState::Low);
    spi_bus_.transmit(tx_data);
    cs_pin_.write(cxx_hal::GpioState::High);
}

void Max7219::set_intensity(uint8_t level) noexcept {
    if (level > 15) {
        level = 15;
    }
    write_register(Register::Intensity, level);
}

void Max7219::clear() noexcept {
    frame_buffer_.fill(0x00);
    for (uint8_t row = 0; row < 8; ++row) {
        write_register(static_cast<Register>(static_cast<uint8_t>(Register::Digit0) + row), 0x00);
    }
}

void Max7219::power_save(bool enable) noexcept {
    // Shutdown register: 0 = Shutdown Mode, 1 = Normal Operation
    write_register(Register::Shutdown, enable ? 0x00 : 0x01);
}

void Max7219::set_row(uint8_t row_index, uint8_t byte_pattern) noexcept {
    if (row_index < 8) {
        frame_buffer_[row_index] = byte_pattern;
        write_register(static_cast<Register>(static_cast<uint8_t>(Register::Digit0) + row_index), byte_pattern);
    }
}

void Max7219::display_buffer(std::span<const uint8_t, 8> buffer) noexcept {
    for (size_t row = 0; row < 8; ++row) {
        set_row(static_cast<uint8_t>(row), buffer[row]);
    }
}

} // namespace drivers::devices
