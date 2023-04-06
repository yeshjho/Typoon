#include "input_multicast.h"

#include <array>


std::array<std::pair<QueueType, bool>, 5> queues;

void multicast_input(InputMessage value)
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
