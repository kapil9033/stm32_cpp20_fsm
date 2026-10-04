#ifndef FSM_EVENT_QUEUE_HPP
#define FSM_EVENT_QUEUE_HPP

#include "Events.hpp"
#include <variant>
#include "FreeRTOS.h"
#include "queue.h"

namespace fsm {

// Variant of all possible events that can trigger state transitions
using EventVariant = std::variant<
    EventMotionDetected,
    EventButtonPress,
    EventTimeout,
    EventFault
>;

class EventQueue {
public:
    explicit EventQueue(UBaseType_t queue_length = 10) noexcept {
        queue_handle_ = xQueueCreate(queue_length, sizeof(EventVariant));
    }

    ~EventQueue() {
        if (queue_handle_ != nullptr) {
            vQueueDelete(queue_handle_);
        }
    }

    // Thread-safe dispatch from standard FreeRTOS tasks
    bool post(const EventVariant& event, TickType_t ticks_to_wait = portMAX_DELAY) noexcept {
        return xQueueSend(queue_handle_, &event, ticks_to_wait) == pdTRUE;
    }

    // ISR-safe dispatch (e.g., EXTI GPIO interrupt handler for HC-SR501)
    bool post_from_isr(const EventVariant& event, BaseType_t* higher_priority_task_woken) noexcept {
        return xQueueSendFromISR(queue_handle_, &event, higher_priority_task_woken) == pdTRUE;
    }

    // Wait and receive event (called by state machine task)
    bool receive(EventVariant& out_event, TickType_t ticks_to_wait = portMAX_DELAY) noexcept {
        return xQueueReceive(queue_handle_, &out_event, ticks_to_wait) == pdTRUE;
    }

    [[nodiscard]] QueueHandle_t get_handle() const noexcept { return queue_handle_; }

private:
    QueueHandle_t queue_handle_{nullptr};
};

} // namespace fsm

#endif // FSM_EVENT_QUEUE_HPP