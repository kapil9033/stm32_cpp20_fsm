#ifndef CXX_HAL_ISPI_HPP
#define CXX_HAL_ISPI_HPP

#include <cstdint>
#include <span>

namespace cxx_hal {

class ISpi {
public:
    virtual ~ISpi() = default;

    virtual bool transmit(std::span<const uint8_t> tx_data) noexcept = 0;
    virtual bool receive(std::span<uint8_t> rx_data) noexcept = 0;
    virtual bool transfer(std::span<const uint8_t> tx_data, std::span<uint8_t> rx_data) noexcept = 0;
};

} // namespace cxx_hal

#endif // CXX_HAL_ISPI_HPP
