# Operating-Systems
Operating Systems projects

Simulator of a cpu scheduling system that can assume the scheduling algorithm of one of four distinct types:

1.) First Come First Serve
  - Non-Preemptive scheduler in which processes are serviced based on the order in which they arrive
  
2.) Shortest Remaining Time First
  - Preemptive Scheduler that chooses to assign to the cpu that process with the least amount of time required for execution remaining
  
3.) Highest Response Ratio Next
  - Non-Preemptive Scheduler that determines processes to execute based on the amount of time they have been waiting and the amount of
    time required for execution remaining: (Tw + Ts)/ Ts => prevents starvation so that processes with high service times (low priority
    as in the case of SRTF) are not constantly neglected.
    
4.) Round Robin
  - Non-Preemptive Scheduler that services processes based on a fixed amount of time (quantum) in the FCFS fashion.

Compile using g++ *.cpp -o executable name and run using ./executable name schedule_type lambda avg. service time  quantum interval.
- quantum interval only is relevant for Round Robin scheduler

Consider running on different sizes of lambda (avg. arrival rate to system or cpu) to observe its effects on cpu utilization,
amount of processes waiting in ready queue, throughput and turnaround time.

  a.) cpu utilization: percentage of use of cpu while system is in use (avg.service time affects this and in fact determines an upper bound for utilization: suppose avg.service time == 0.06 seconds. Then, lambda can be no larger than approximately 16 before cpu is constantly running (not idle)).
      
  b.) Ready Queue Size: amount of processes waiting to be serviced, either due to arriving and there being high utilization, the process
      has previously been preempted in the context of preemptive schedulers such as SRTF, or has been serviced for some quanta and requires further service time.
      
  c.) Throughput: Total amount of processes serviced given the amount of time in which they are processed (rate of work)
  
  d.) Turnaround Time: Average amount of time a process spends in the system beginning with when it arrives and ending with when it has
      been serviced to completion.
