#include <cassert>
#include <cstddef>
#include <stdexcept>
#include "types/thread/thread.hpp"

void Thread::set_ready(int time) {
    if (current_state == NEW || current_state == BLOCKED){
        current_state = READY;
        state_change_time= time;
    }
    else{
        std::cout << "Error: Invalid state change";
    }
}

void Thread::set_running(int time) {
     if (current_state == READY || current_state == BLOCKED){
        current_state = RUNNING;
        state_change_time= time;
    }
    else{
        std::cout << "Error: Invalid state change";
    }
    
}

void Thread::set_blocked(int time) {
    if (current_state ==READY){
        current_state = BLOCKED;
        state_change_time= time;
    }
    else{
        std::cout << "Error: Invalid state change";
    }
}

void Thread::set_finished(int time) {
      if (current_state ==RUNNING){
        current_state = EXIT;
        state_change_time= time;
    }
    else{
        std::cout << "Error: Invalid state change";
    }
}

int Thread::response_time() const {
   return (state_change_time- arrival_time);
}

int Thread::turnaround_time() const {
    return (end_time - arrival_time);
}

void Thread::set_state(ThreadState state, int time) {
   switch (state){
       case READY:
        set_ready(time);
        break;
       case RUNNING:
        set_running(time);
        break;
       case BLOCKED:
        set_blocked(time);
        break;
       case EXIT:
         set_finished(time);
         break;
   }
}

std::shared_ptr<Burst> Thread::get_next_burst(BurstType type) {
   std::shared_ptr<Burst> popped_burst = bursts.front();
   bursts.pop();
   std::shared_ptr<Burst> next_burst = bursts.front();
   bursts.push(popped_burst);
   return next_burst;

}

std::shared_ptr<Burst> Thread::pop_next_burst(BurstType type) {
   bursts.pop();
   return bursts.front();
}