#pragma once
#include <atomic_queue/atomic_queue.h>


using InputType = wchar_t;
using QueueType = atomic_queue::AtomicQueue<InputType, 20, atomic_queue::details::nil<InputType>(), true, true, true, true>;

// NOTE: Keyboard watcher thread only.
void multicast_input(InputType value);
// NOTE: Main thread only. If you need it in another thread, call it from main thread and pass the queue to that thread.
QueueType& register_input_listener();
