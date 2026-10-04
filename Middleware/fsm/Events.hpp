#ifndef FSM_EVENTS_HPP
#define FSM_EVENTS_HPP

#include <cstdint>

namespace fsm {

// Event payload structures
struct EventButtonPress {};

struct EventMotionDetected {
    uint32_t duration_ms;
};

struct EventTimeout {};

struct EventFault {
    uint8_t error_code;
};

} // namespace fsm

#endif // FSM_EVENTS_HPP
