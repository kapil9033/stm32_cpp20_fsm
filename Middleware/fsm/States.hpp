#ifndef FSM_STATES_HPP
#define FSM_STATES_HPP

#include <cstdint>

namespace fsm {

// State representation structs
struct StateIdle {
    uint32_t enter_time_ms{0};
};

struct StateScanning {
    uint32_t scan_count{0};
};

struct StateProcessing {
    uint8_t current_step{0};
};

struct StateFault {
    uint8_t error_code{0};
};

} // namespace fsm

#endif // FSM_STATES_HPP
