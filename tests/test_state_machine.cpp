#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "StateMachine.hpp"
#include "Max7219.hpp"
#include "Sg90Servo.hpp"
#include "MockSpi.hpp"
#include "MockGpio.hpp"
#include "MockPwm.hpp"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::Return;

class StateMachineTest : public ::testing::Test {
protected:
    MockSpi mock_spi;
    MockGpio mock_cs;
    MockPwm mock_pwm;

    drivers::devices::Max7219 matrix{mock_spi, mock_cs};
    drivers::devices::Sg90Servo servo{mock_pwm};

    void SetUp() override {
        EXPECT_CALL(mock_cs, write(_)).Times(AtLeast(0));
        EXPECT_CALL(mock_spi, transmit(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(mock_pwm, start()).Times(AtLeast(0));
        EXPECT_CALL(mock_pwm, set_pulse_width_us(_)).Times(AtLeast(0));

        matrix.init();
        servo.init();
    }
};

TEST_F(StateMachineTest, InitialStateIsIdleWithZeroDegreeServo) {
    EXPECT_CALL(mock_pwm, set_pulse_width_us(drivers::devices::Sg90Servo::MIN_PULSE_US)).Times(1);

    fsm::StateMachine fsm_engine(matrix, servo);

    EXPECT_TRUE(std::holds_alternative<fsm::StateIdle>(fsm_engine.get_current_state()));
    EXPECT_EQ(servo.get_current_angle(), 0);
}

TEST_F(StateMachineTest, TransitionsFromIdleToScanningOnMotionDetected) {
    fsm::StateMachine fsm_engine(matrix, servo);

    // 45 degrees -> 500 + (45 * 2000 / 180) = 1000 us
    EXPECT_CALL(mock_pwm, set_pulse_width_us(1000)).Times(1);

    fsm_engine.dispatch(fsm::EventMotionDetected{100});

    EXPECT_TRUE(std::holds_alternative<fsm::StateScanning>(fsm_engine.get_current_state()));
    EXPECT_EQ(servo.get_current_angle(), 45);
}

TEST_F(StateMachineTest, TransitionsFromScanningToProcessingOnButtonPress) {
    fsm::StateMachine fsm_engine(matrix, servo);
    fsm_engine.dispatch(fsm::EventMotionDetected{100});

    // 90 degrees = 1500 us
    EXPECT_CALL(mock_pwm, set_pulse_width_us(1500)).Times(1);

    fsm_engine.dispatch(fsm::EventButtonPress{});

    EXPECT_TRUE(std::holds_alternative<fsm::StateProcessing>(fsm_engine.get_current_state()));
    EXPECT_EQ(servo.get_current_angle(), 90);
}

TEST_F(StateMachineTest, DirectTransitionToFaultOnEventFault) {
    fsm::StateMachine fsm_engine(matrix, servo);

    // 180 degrees = 2500 us
    EXPECT_CALL(mock_pwm, set_pulse_width_us(drivers::devices::Sg90Servo::MAX_PULSE_US)).Times(1);

    // Passed valid uint8_t error code (0x01 instead of 404)
    fsm_engine.dispatch(fsm::EventFault{0x01});

    EXPECT_TRUE(std::holds_alternative<fsm::StateFault>(fsm_engine.get_current_state()));
    EXPECT_EQ(servo.get_current_angle(), 180);
}
