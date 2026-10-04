#ifndef APPLICATION_FSM_TASK_HPP
#define APPLICATION_FSM_TASK_HPP

#include "StateMachine.hpp"
//#include "FsmEventQueue.hpp"
#include "FreeRTOS.h"
#include "task.h"

namespace app {

class FsmTask {
public:
    FsmTask(fsm::StateMachine& state_machine, fsm::EventQueue& event_queue) noexcept
        : state_machine_(state_machine), event_queue_(event_queue) {}

    // Static C-compatible entry point for xTaskCreate
    static void task_entry(void* pvParameters) noexcept {
        auto* self = static_cast<FsmTask*>(pvParameters);
        self->run();
    }

    void start(const char* name = "FSM_Task", 
               configSTACK_DEPTH_TYPE stack_depth = 512, 
               UBaseType_t priority = tskIDLE_PRIORITY + 2) noexcept {
        xTaskCreate(
            &FsmTask::task_entry,
            name,
            stack_depth,
            this,
            priority,
            &task_handle_
        );
    }

private:
    fsm::StateMachine& state_machine_;
    fsm::EventQueue& event_queue_;
    TaskHandle_t task_handle_{nullptr};

    void run() noexcept {
        fsm::EventVariant received_event;

        while (true) {
            // Block until an event is pushed into the queue
            if (event_queue_.receive(received_event, portMAX_DELAY)) {
                // Visit the variant and dispatch to the FSM engine
                std::visit([this](const auto& event) {
                    state_machine_.dispatch(event);
                }, received_event);
            }
        }
    }
};

} // namespace app

#endif // APPLICATION_FSM_TASK_HPP