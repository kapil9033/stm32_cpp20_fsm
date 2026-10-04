#ifndef CXX_HAL_IGPIO_HPP
#define CXX_HAL_IGPIO_HPP

#include <cstdint>

namespace cxx_hal {

enum class GpioState : uint8_t {
    Low = 0,
    High = 1
};

enum class GpioMode : uint8_t {
    Input,
    OutputPushPull,
    OutputOpenDrain,
    AlternateFunction,
    Analog
};

class IGpio {
public:
    virtual ~IGpio() = default;

    [[nodiscard]] virtual GpioState read() const noexcept = 0;
    virtual void write(GpioState state) noexcept = 0;
    virtual void toggle() noexcept = 0;
};

} // namespace cxx_hal

#endif // CXX_HAL_IGPIO_HPP
