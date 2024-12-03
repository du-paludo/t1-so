# Barrier and FIFO Queue Simulation

This project implements a simulation of a barrier synchronization mechanism and a FIFO queue using shared memory, semaphores, mutexes, and processes in C. The goal is to demonstrate process synchronization and resource management in a concurrent environment.

## How it works

### Key Components

- **Barrier (`barrier_t`):**

  - Ensures that processes wait for all other processes to reach a certain point before proceeding.

  - Uses a counter, mutex, and semaphore to synchronize processes.

- **FIFO Queue (`fifoQ_s`):**

  - Manages processes in a first-in, first-out order for accessing a critical section.

  - Utilizes a counter and a next pointer, protected by a mutex.

- **Shared Memory:**

  - The barrier and FIFO queue structures are stored in shared memory segments to enable inter-process communication.

- **Process Lifecycle:**

  - Each process executes three stages in a loop (prologue, resource usage, epilogue) with synchronization via the barrier and FIFO queue.

  - The process sleeps for random intervals to simulate real-world asynchronous behavior.
 
## How to Compile and Execute

### Prerequisites

Ensure you have the following installed on your system:

- GCC (GNU Compiler Collection)

- POSIX libraries for threads, semaphores, and shared memory

### Compilation

To compile the program, simply run the following command:

```
make
```

### Execution

Run the compiled program with the number of processes as an argument:

```
./main <nTotalProcesses>
```

Replace `<nTotalProcesses>` with the desired number of processes (e.g., 5).

### Example

To run the program with 4 processes:

```
./barrier_fifo 4
```

### Program Output

The program prints log messages to the console, including:

- Process entry and exit from the barrier and FIFO queue.

- Random sleep times for prologue, resource usage, and epilogue stages.

- Time-stamped process activity.

## Notes

- The shared memory keys (2378 and 2379) are hardcoded for simplicity. If conflicts occur, consider changing these values.

- The program uses random sleep intervals to simulate real-world conditions. This can be adjusted for testing.

## Troubleshooting

- If you encounter permission issues with shared memory or semaphores, try running the program with elevated privileges (e.g., using `sudo`).

- Ensure the number of processes (`<nTotalProcesses>`) is reasonable for your system's resources.
