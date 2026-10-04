#ifndef TESTS_MOCKS_MOCK_PWM_HPP
#define TESTS_MOCKS_MOCK_PWM_HPP

#include "IPwm.hpp"
#include <gmock/gmock.h>

class MockPwm : public cxx_hal::IPwm {
public:
    MOCK_METHOD(void, start, (), (noexcept, override));
    MOCK_METHOD(void, stop, (), (noexcept, override));
    MOCK_METHOD(void, set_pulse_width_us, (uint32_t pulse_us), (noexcept, override));
    MOCK_METHOD(void, set_duty_cycle, (float percentage), (noexcept, override));
};

#endif // TESTS_MOCKS_MOCK_PWM_HPP
