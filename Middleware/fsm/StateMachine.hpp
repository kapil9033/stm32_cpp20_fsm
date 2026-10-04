#ifndef FSM_STATE_MACHINE_HPP
#define FSM_STATE_MACHINE_HPP

#include "Events.hpp"
#include "States.hpp"
#include "Max7219.hpp"
#include "Sg90Servo.hpp"
#include <variant>
#include <optional>
#include <utility>
#include <array>

namespace fsm {

// Overload pattern helper
template<class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template<class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

using StateVariant = std::variant<StateIdle, StateScanning, StateProcessing, StateFault>;

// Visual 8x8 matrix patterns for MAX7219
constexpr std::array<uint8_t, 8> PATTERN_IDLE = {
    0x00, 0x00, 0x24, 0x00, 0x00, 0x42, 0x3C, 0x00  // Smile pattern
};

constexpr std::array<uint8_t, 8> PATTERN_SCANNING = {
    0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF  // Box border pattern
};

constexpr std::array<uint8_t, 8> PATTERN_PROCESSING = {
    0x18, 0x3C, 0x7E, 0xFF, 0xFF, 0x7E, 0x3C, 0x18  // Diamond pattern
};

constexpr std::array<uint8_t, 8> PATTERN_FAULT = {
    0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81  // Cross 'X' pattern
};

class StateMachine {
public:
    StateMachine(drivers::devices::Max7219& matrix, drivers::devices::Sg90Servo& servo) noexcept
        : matrix_(matrix), servo_(servo), current_state_(StateIdle{}) {
        update_actuators();
    }

    template<typename Event>
    void dispatch(const Event& event) noexcept {
        auto next_state = std::visit(
            Overloaded{
                [&](const StateIdle&) -> std::optional<StateVariant> {
                    if constexpr (std::is_same_v<Event, EventMotionDetected>) {
                        return StateScanning{0};
                    }
                    return std::nullopt;
                },
                [&](const StateScanning&) -> std::optional<StateVariant> {
                    if constexpr (std::is_same_v<Event, EventButtonPress>) {
                        return StateProcessing{0};
                    } else if constexpr (std::is_same_v<Event, EventTimeout>) {
                        return StateIdle{};
                    }
                    return std::nullopt;
                },
                [&](const StateProcessing&) -> std::optional<StateVariant> {
                    if constexpr (std::is_same_v<Event, EventTimeout>) {
                        return StateIdle{};
                    }
                    return std::nullopt;
                },
                [&](const StateFault&) -> std::optional<StateVariant> {
                    if constexpr (std::is_same_v<Event, EventButtonPress>) {
                        return StateIdle{};
                    }
                    return std::nullopt;
                }
            },
            current_state_
        );

        if constexpr (std::is_same_v<Event, EventFault>) {
            current_state_ = StateFault{event.error_code};
            update_actuators();
            return;
        }

        if (next_state.has_value()) {
            current_state_ = std::move(*next_state);
            update_actuators();
        }
    }

    [[nodiscard]] const StateVariant& get_current_state() const noexcept {
        return current_state_;
    }

private:
    drivers::devices::Max7219& matrix_;
    drivers::devices::Sg90Servo& servo_;
    StateVariant current_state_;

    // Synchronize MAX7219 matrix and SG90 servo position with the newly active state
    void update_actuators() noexcept {
        std::visit(
            Overloaded{
                [this](const StateIdle&) {
                    matrix_.display_buffer(PATTERN_IDLE);
                    servo_.set_angle(0);     // Reset servo to 0 degrees
                },
                [this](const StateScanning&) {
                    matrix_.display_buffer(PATTERN_SCANNING);
                    servo_.set_angle(45);    // Turn servo to 45 degrees
                },
                [this](const StateProcessing&) {
                    matrix_.display_buffer(PATTERN_PROCESSING);
                    servo_.set_angle(90);    // Open gate/valve to 90 degrees
                },
                [this](const StateFault&) {
                    matrix_.display_buffer(PATTERN_FAULT);
                    servo_.set_angle(180);   // Move servo to safety stop (180 degrees)
                }
            },
            current_state_
        );
    }
};

} // namespace fsm

#endif // FSM_STATE_MACHINE_HPP
