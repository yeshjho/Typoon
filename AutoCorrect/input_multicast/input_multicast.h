#pragma once
#include <atomic_queue/atomic_queue.h>


void multicast_input(unsigned short value);
atomic_queue::AtomicQueue<unsigned short, 20, atomic_queue::details::nil<short>(), true, true, true, true>& register_input_listener();
