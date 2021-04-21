#include <fstream>
#include <iostream>

#include "algorithms/fcfs/fcfs_algorithm.hpp"
#include "algorithms/rr/rr_algorithm.hpp"

#include "simulation/simulation.hpp"
#include "types/enums.hpp"

#include "utilities/flags/flags.hpp"

Simulation::Simulation(FlagOptions flags)
{
    // Hello!
    if (flags.scheduler == "FCFS")
    {
        // Create a FCFS scheduling algorithm
        this->scheduler = std::make_shared<FCFSScheduler>();
    }
    else if (flags.scheduler == "RR")
    {
        // Create a RR scheduling algorithm
        this->scheduler = std::make_shared<RRScheduler>(flags.time_slice);
    }
    this->flags = flags;
    this->logger = Logger(flags.verbose, flags.per_thread, flags.metrics);
}

void Simulation::run()
{
    this->read_file(this->flags.filename);

    while (!this->events.empty())
    {
        auto event = this->events.top();
        this->events.pop();

        // Invoke the appropriate method in the simulation for the given event type.

        switch (event->type)
        {
        case PROCESS_ARRIVED:
            this->handle_process_arrived(event);
            break;

        case PROCESS_DISPATCH_COMPLETED:
            this->handle_dispatch_completed(event);
            break;

        case CPU_BURST_COMPLETED:
            this->handle_cpu_burst_completed(event);
            break;

        case IO_BURST_COMPLETED:
            this->handle_io_burst_completed(event);
            break;
        case PROCESS_COMPLETED:
            this->handle_process_completed(event);
            break;

        case PROCESS_PREEMPTED:
            this->handle_process_preempted(event);
            break;

        case DISPATCHER_INVOKED:
            this->handle_dispatcher_invoked(event);
            break;
        }

        // If this event triggered a state change, print it out.
        if (event->thread && event->thread->current_state != event->thread->previous_state)
        {
            this->logger.print_state_transition(event, event->thread->previous_state, event->thread->current_state);
        }
        this->system_stats.total_time = event->time;
        event.reset();
    }
    // We are done!

    std::cout << "SIMULATION COMPLETED!\n\n";

    for (auto entry : this->processes)
    {
        this->logger.print_per_thread_metrics(entry.second);
    }

    logger.print_simulation_metrics(this->calculate_statistics());
}

//==============================================================================
// Event-handling methods
//==============================================================================

void Simulation::handle_process_arrived(const std::shared_ptr<Event> event)
{   
    /*
     * Set Ready enqueue
     * process IDLE when active_thread is null or 
     * scheduler empty (check w/ DISPATCHER INVOKED)
     * if not idle, preempt
     */  
    if(event->thread != nullptr){
        event->thread->set_state(READY,event->time);
        scheduler ->add_to_ready_queue(active_thread);
    }
    
    if ( (active_thread == nullptr) || (scheduler->empty()) ) {
        //make new DISPATCHER_INVOKED event add to queue
        events.push(std::make_shared<Event>(EventType::DISPATCHER_INVOKED, event->time + process_switch_overhead , event_num, nullptr, nullptr));
        event_num ++;
    }
    else{
         events.push(std::make_shared<Event>(EventType::PROCESS_PREEMPTED, event->time + process_switch_overhead , event_num, nullptr, nullptr));
         event_num ++;
    }
}

void Simulation::handle_dispatch_completed(const std::shared_ptr<Event> event)
{
    /*
    * set running
    * last thread = current thread
    * compare time slice and burst length
    */
   event->thread->set_state(RUNNING,event->time);
   prev_thread = active_thread;
   if (scheduler->time_slice < event->thread->get_next_burst(CPU)->length){
         //Process preempted
         events.push(std::make_shared<Event>(EventType::PROCESS_PREEMPTED, event->time + process_switch_overhead , event_num, nullptr, nullptr));
         event_num ++;
   }else{
       //check for last cpu burst
       std::shared_ptr<Burst> burst_out = event->thread->pop_next_burst(IO);
       if(event->thread->get_next_burst(CPU) == nullptr){
           //CPU burst complete
         events.push(std::make_shared<Event>(EventType::CPU_BURST_COMPLETED, event->time + burst_out->length , event_num, nullptr, nullptr));
         event_num ++;
       }
       else{
           //process complete
           events.push(std::make_shared<Event>(EventType::PROCESS_COMPLETED, event->time + burst_out->length , event_num, nullptr, nullptr));
           event_num ++;
       }
       event->thread->bursts.push(burst_out);
   }
}

void Simulation::handle_cpu_burst_completed(const std::shared_ptr<Event> event)
{
    /*
    * Pop burst, set to blocked
    * to I/O burst complete and dispatcher invoked
    */
   event ->  thread->set_state(BLOCKED,event->time);
   event -> thread -> pop_next_burst(IO);
   events.push(std::make_shared<Event>(EventType::IO_BURST_COMPLETED, event->time + event->thread->bursts.front()->length , event_num, nullptr, nullptr));
   event_num ++;
    events.push(std::make_shared<Event>(EventType::DISPATCHER_INVOKED, event->time + event->thread->bursts.front()->length , event_num, nullptr, nullptr));
   event_num ++;
}

void Simulation::handle_io_burst_completed(const std::shared_ptr<Event> event)
{
  /*
  * Set Ready Enqueue
  * Pop burst
  * Process Idle
  */  
 event->thread->set_state(READY,event->time);
 scheduler->add_to_ready_queue(event->thread);
 event->thread->pop_next_burst(CPU);
 active_thread= nullptr;
}

void Simulation::handle_process_completed(const std::shared_ptr<Event> event)
{
    /*
    * Exit
    */
   event->thread->set_state(EXIT,event->time);
}

void Simulation::handle_process_preempted(const std::shared_ptr<Event> event)
{
    /*
    * Set ready enqueue
    * decrease CPU burst
    * make DISPATCHER_INVOKED event
    */
   event ->thread->set_state(READY, event->time);
   scheduler->add_to_ready_queue(event->thread);
   events.push(std::make_shared<Event>(EventType::PROCESS_COMPLETED, event->time + event->thread->bursts.front()->length-1 , event_num, nullptr, nullptr));
   event_num ++;
}

void Simulation::handle_dispatcher_invoked(const std::shared_ptr<Event> event)
{
   /*
   * Get Scheduling Decision + Set current thread
   * Decide schedule algo
   * 
   * Comes from I/O burst complete and arrival
   */
    std::shared_ptr<Thread> next_thread = scheduler->get_next_thread()->thread; 
    int next_burst_length = next_thread->pop_next_burst(CPU)->length;
    events.push(std::make_shared<Event>(EventType::PROCESS_DISPATCH_COMPLETED, event->time + next_burst_length , event_num, next_thread, nullptr));
    event_num ++;
}

//==============================================================================
// Utility methods
//==============================================================================

SystemStats Simulation::calculate_statistics()
{
    // TODO: Implement functionality for calculating the simulation statistics
    return this->system_stats;
}

void Simulation::add_event(std::shared_ptr<Event> event)
{
    if (event != nullptr)
    {
        this->events.push(event);
    }
}

void Simulation::read_file(const std::string filename)
{
    std::ifstream input_file(filename.c_str());

    if (!input_file)
    {
        std::cerr << "Unable to open simulation file: " << filename << std::endl;
        throw(std::logic_error("Bad file."));
    }

    int num_processes;
    int thread_switch_overhead; // This is discarded for this semester

    input_file >> num_processes >> thread_switch_overhead >> this->process_switch_overhead;

    for (int proc = 0; proc < num_processes; ++proc)
    {
        auto process = read_process(input_file);

        this->processes[process->process_id] = process;
    }
}

std::shared_ptr<Process> Simulation::read_process(std::istream &input)
{
    int process_id, priority;
    int num_threads;

    input >> process_id >> priority >> num_threads;

    auto process = std::make_shared<Process>(process_id, (ProcessPriority)priority);

    // iterate over the threads
    for (int thread_id = 0; thread_id < num_threads; ++thread_id)
    {
        process->threads.emplace_back(read_thread(input, thread_id, process_id, (ProcessPriority)priority));
    }

    return process;
}

std::shared_ptr<Thread> Simulation::read_thread(std::istream &input, int thread_id, int process_id, ProcessPriority priority)
{
    // Stuff
    int arrival_time;
    int num_cpu_bursts;

    input >> arrival_time >> num_cpu_bursts;

    auto thread = std::make_shared<Thread>(arrival_time, thread_id, process_id, priority);

    for (int n = 0, burst_length; n < num_cpu_bursts * 2 - 1; ++n)
    {
        input >> burst_length;

        BurstType burst_type = (n % 2 == 0) ? BurstType::CPU : BurstType::IO;

        thread->bursts.push(std::make_shared<Burst>(burst_type, burst_length));
    }

    this->events.push(std::make_shared<Event>(EventType::PROCESS_ARRIVED, thread->arrival_time, this->event_num, thread, nullptr));
    this->event_num++;

    return thread;
}
