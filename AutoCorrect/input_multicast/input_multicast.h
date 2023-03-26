#pragma once
#include <atomic_queue/atomic_queue.h>


using QueueType = atomic_queue::AtomicQueue<wchar_t, 20, atomic_queue::details::nil<wchar_t>(), true, true, true, true>;

// NOTE: Keyboard watcher thread only.
void multicast_input(wchar_t value);
// NOTE: Main thread only. If you need it in another thread, call it from main thread and pass the queue to that thread.
QueueType& register_input_listener();
