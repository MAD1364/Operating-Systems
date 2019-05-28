Pthreads Practice

  This program is an exercise in the use of mutexes and condition variables to simulate a card game based on the synchronization
  of multiple threads, specifically through the use of pthreads in c. The current program is written for a single cpu.
  
  - Mutexes are synchronization mechanisms that when used appropriately prevent the simultaneous access of shared memory in
    shared memory contexts. cmf.c uses one program whose threads are based on the same memory space, This forces the threads to
    share data structures such as the queue representing the deck of cards and other relevant variables.
    
  - Condition variables are also a synchronization tool that provide a mechanism for communication between multiple threads. This
    communication can be thought of as an alert for threads when to permit them to proceed on the basis of a particular condition
    existing. When threads are waiting, depending on the implementation of pthreads, the threads will spend their time waiting but
    not running as in the case of busy waiting (busy waiting wastes cpu cycles).
    
To Compile: gcc -pthread cmf.c -o executable name
To Run: ./executable_name seed
  - Seed is a command line argument used to implement pseudorandom arrangements of cards for the deck used in the game. 
