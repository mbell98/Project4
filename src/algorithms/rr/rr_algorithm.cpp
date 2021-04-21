#include "algorithms/rr/rr_algorithm.hpp"

#include <cassert>
#include <stdexcept>
#include <sstream>

/*
    Here is where you should define the logic for the round robin algorithm.
*/

RRScheduler::RRScheduler(int slice) {    
   time_slice=slice;
}

std::shared_ptr<SchedulingDecision> RRScheduler::get_next_thread() {
    
    queue_holder.at(0).front();
}

void RRScheduler::add_to_ready_queue(std::shared_ptr<Thread> thread) {
    std::queue<std::shared_ptr<Thread>> new_queue;
    if (queue_holder.size()== 0){
        //populate empty container
        queue_holder.push_back(new_queue);
    }
    for (int i = 0; i< queue_holder.size(); i++){
        //insert thread partitions to queues
        if (thread->state_change_time > 0){
        thread->state_change_time-= time_slice;
        }
        std::shared_ptr<Thread> queue_thread = thread;
        queue_thread -> state_change_time = time_slice;
        queue_holder.at(i).push(queue_thread);
    }
    if (thread->state_change_time > 0){
        //queue holder is full but there's still thread time
        new_queue.push(thread);
        queue_holder.push_back(new_queue);
    }

  
}

size_t RRScheduler::size() const {
    int size= 0;
    for (int i = 0; i<queue_holder.size(); i++){
        size += queue_holder.at(i).size();
    }
    return size;
}
