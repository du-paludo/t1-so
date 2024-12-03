# Barrier and FIFO Queue Simulation

This project implements a simulation of a barrier synchronization mechanism and a FIFO queue using shared memory, semaphores, mutexes, and processes in C. The goal is to demonstrate process synchronization and resource management in a concurrent environment.

## How it works

### Key Components

1. **Barrier (`barrier_t`):**

- Ensures that processes wait for all other processes to reach a certain point before proceeding.

- Uses a counter, mutex, and semaphore to synchronize processes.

2. **FIFO Queue (`fifoQ_s`):**

- Manages processes in a first-in, first-out order for accessing a critical section.

- Utilizes a counter and a next pointer, protected by a mutex.

3. **Shared Memory:**

- The barrier and FIFO queue structures are stored in shared memory segments to enable inter-process communication.

4. **Process Lifecycle:**

- Each process executes three stages in a loop (prologue, resource usage, epilogue) with synchronization via the barrier and FIFO queue.

- The process sleeps for random intervals to simulate real-world asynchronous behavior.
