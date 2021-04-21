#include "algorithms/fcfs/fcfs_algorithm.hpp"

#include <cassert>
#include <stdexcept>
#include <queue>

#define FMT_HEADER_ONLY
#include "utilities/fmt/format.h"

/*
    Here is where you should define the logic for the FCFS algorithm.
*/

FCFSScheduler::FCFSScheduler(int slice) {
    if (slice != -1) {
        throw("FCFS must have a timeslice of -1");
    }
}

std::shared_ptr<SchedulingDecision> FCFSScheduler::get_next_thread() {
    std::shared_ptr<Thread> next_thread = ready_queue.front();
    std::shared_ptr<SchedulingDecision> next_decision;
    next_decision->thread = next_thread;
    next_decision->time_slice = next_thread->state_change_time-next_thread->start_time;
    next_decision->explanation = "selected from"+std::to_string(ready_queue.size())+" processes";
    return next_decision;
}

void FCFSScheduler::add_to_ready_queue(std::shared_ptr<Thread> thread) {
    ready_queue.push(thread);
}

size_t FCFSScheduler::size() const {
    return ready_queue.size();
}
