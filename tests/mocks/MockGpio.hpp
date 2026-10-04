#ifndef TESTS_MOCKS_MOCK_GPIO_HPP
#define TESTS_MOCKS_MOCK_GPIO_HPP

#include "IGpio.hpp"
#include <gmock/gmock.h>

class MockGpio : public cxx_hal::IGpio {
public:
    MOCK_METHOD(cxx_hal::GpioState, read, (), (const, noexcept, override));
    MOCK_METHOD(void, write, (cxx_hal::GpioState state), (noexcept, override));
    MOCK_METHOD(void, toggle, (), (noexcept, override));
};

#endif // TESTS_MOCKS_MOCK_GPIO_HPP
