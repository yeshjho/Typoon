#pragma once
#include <atomic_queue/atomic_queue.h>


struct InputMessage
{
    wchar_t letter = 0;
    bool isBeingComposed = false;

    constexpr bool operator==(const InputMessage& other) const noexcept = default;
};


using InputQueueType = atomic_queue::AtomicQueue<InputMessage, 20, atomic_queue::details::nil<InputMessage>(), true, true, true, true>;

// NOTE: Imm simulator thread only.
void multicast_input(InputMessage value);
// NOTE: Main thread only. If you need it in another thread, call it from main thread and pass the queue to that thread.
InputQueueType& register_input_listener();
