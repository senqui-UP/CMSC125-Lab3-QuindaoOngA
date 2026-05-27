# Concurrent Banking Database Simulator

---
## CMSC 125-1 Lab-3 Laboratory 3
Authors: Ong, Andy Dominic X. & Quindao, Hansen Maeve C.

---
A multithreaded banking database simulator written in C implementing concurrent transaction execution, deadlock detection, bounded buffer management, and synchronization using POSIX threads, reader-writer locks, semaphores, mutexes, and condition variables.

## Features

* Concurrent transaction execution using pthreads
* Reader-Writer Locks for account synchronization
* Deadlock Detection using Wait-For Graph cycle detection
* Semaphore-Based Buffer Pool
* Timer Thread with Global Tick Synchronization
* Transaction Metrics and Throughput Reporting
* Transaction Abort and Recovery Handling
* Trace File Parser
* Account File Parser
* Execution Logging
* Balance Conservation Validation

---

## Installation

#### **1. Clone**
```bash
git clone https://github.com/senqui-UP/CMSC125-Lab3-QuindaoOngA
cd CMSC125-Lab3-QuindaoOngA
```

#### **2. Makefile**
A Makefile is provided. From the project root directory, run:
```bash
make            # compile bankdb
make debug      # compile with ThreadSanitizer
make clean      # remove binary
make test       # run all test traces
make run        # run default trace
```

---
## Project Structure
```bash
CMSC125-Lab3-QuindaoOngA/
├── include/                     # Header files
│   ├── bank.h                   # Bank and account structures
│   ├── transaction.h            # Transaction and operation structures
│   ├── timer.h                  # Timer synchronization
│   ├── lock_mgr.h               # Wait-for graph and deadlock detection
│   ├── buffer_pool.h            # Semaphore-based buffer pool
│   ├── metrics.h                # Metrics collection
│   └── utils.h                  # Parsing helpers
│
├── src/                         # Source files
│   ├── main.c                   # Entry point and initialization
│   ├── bank.c                   # Account operations
│   ├── transaction.c            # Transaction execution thread
│   ├── timer.c                  # Timer thread implementation
│   ├── lock_mgr.c               # Deadlock detection logic
│   ├── buffer_pool.c            # Bounded buffer implementation
│   ├── metrics.c                # Metrics reporting
│   └── utils.c                  # File parsers
│
├── tests/                       # Testing resources
│   ├── accounts.txt             # Initial account balances
│   ├── trace.txt                # Standard transaction trace
│   ├── trace_simple.txt         # Sequential operations
│   ├── trace_readers.txt        # Concurrent readers test
│   ├── trace_deadlock.txt       # Deadlock scenario
│   ├── trace_abort.txt          # Insufficient funds abort test
│   └── trace_buffer.txt         # Buffer saturation test
│
├── docs/
│   └── design.md                # Design discussion and justification
│  
├── Makefile
└── README.md
```
## Command Line Options

| Flag | Description |
|---|---|
| `--accounts=<file>` | Account initialization file |
| `--trace=<file>` | Transaction trace file |
| `--tick-ms=<n>` | Tick interval in milliseconds |
| `--help`, `-h` | Show help message |

---

## Usage

Default execution:

```bash
./bankdb
```

Specify custom account and trace files:

```bash
./bankdb \
--accounts=tests/accounts.txt \
--trace=tests/trace.txt \
--tick-ms=100
```

Run deadlock test:

```bash
./bankdb \
--trace=tests/trace_deadlock.txt
```

Run abort handling test:

```bash
./bankdb \
--trace=tests/trace_abort.txt
```

Run buffer saturation test:

```bash
./bankdb \
--trace=tests/trace_buffer.txt
```

---

## Synchronization Design

The banking simulator uses multiple synchronization primitives:

| Primitive | Usage |
|---|---|
| pthread mutex | Metrics and wait-for graph |
| pthread rwlock | Per-account synchronization |
| semaphores | Buffer pool management |
| condition variables | Timer tick synchronization |

Each bank account maintains its own reader-writer lock, allowing concurrent reads while protecting writes.

Mutexes protect shared structures such as:

* metrics
* global tick
* wait-for graph
* console logging

Condition variables are used for timer synchronization between threads.

---

## Deadlock Handling

The simulator implements deadlock detection using a wait-for graph.

When a transaction fails to acquire a lock:

1. A dependency edge is added to the wait-for graph
2. DFS cycle detection is performed
3. If a cycle exists:
    * one transaction is aborted
    * locks are released
    * graph entries are cleared

The simulator uses deadlock detection and transaction abort recovery to resolve circular waits between transactions.

---

## Buffer Pool Design

The buffer pool simulates limited in-memory database pages.

A bounded buffer implementation is used with POSIX semaphores:

* `empty_slots` tracks available slots
* `full_slots` tracks occupied slots

Transactions block when the buffer pool becomes full and resume once slots become available.

---

## Metrics Collected

The simulator collects the following metrics:

* Total transactions
* Committed transactions
* Aborted transactions
* Total simulation ticks
* Throughput
* Buffer pool statistics
* Deadlock statistics
* Transaction execution timing

---

## Testing

The simulator is tested using multiple trace workloads.

### Current Test Coverage

* Sequential transaction execution
* Concurrent reader access
* Deadlock detection and recovery
* Transaction abort handling
* Buffer pool saturation and unblocking
* Balance conservation validation
* ThreadSanitizer race-condition checking
* Multi-operation transaction parsing

---

## Known Limitations

* The simulator currently implements deadlock detection only
* Database is memory-only and non-persistent
* Buffer pool size is statically configured
* Logging output ordering may vary slightly due to thread scheduling
* Thread timing may differ slightly across systems due to OS scheduling behavior
---