#include "input_multicast.h"

#include <array>


using QueueType = atomic_queue::AtomicQueue<unsigned short, 20, atomic_queue::details::nil<unsigned short>(), true, true, true, true>;
std::array<std::pair<QueueType, bool>, 5> queues;

void multicast_input(unsigned short value)
{
    for (auto& [queue, isTaken] : queues)
    {
        if (isTaken)
        {
            queue.push(value);
        }
    }
}

QueueType& register_input_listener()
{
    for (auto& [queue, isTaken] : queues)
    {
        if (!isTaken)
        {
            isTaken = true;
            return queue;
        }
    }

    throw std::exception{ "No left input queue" };
}
