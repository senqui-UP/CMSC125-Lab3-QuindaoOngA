# Design Justifications

## 1. Synchronization Design

The banking simulator uses fine-grained synchronization through per-account reader-writer locks.

Each account maintains its own `pthread_rwlock_t` lock, allowing multiple readers while ensuring exclusive access for writers.

Reader-writer locks were chosen instead of regular mutexes because banking systems typically have more read operations than write operations, allowing better concurrency for balance inquiries.

Reader locks are used for balance inquiries while writer locks are used for:

* deposits
* withdrawals
* transfers

Additional mutexes protect shared global structures such as:

* metrics
* wait-for graph
* global tick

Condition variables are used for timer synchronization between transaction threads and the timer thread.

This design minimizes contention while ensuring thread safety across concurrent transactions.

---

## 2. Deadlock Handling

The simulator implements deadlock detection using a wait-for graph.

When a transaction cannot acquire a lock, a dependency edge is added to the graph representing which transaction is waiting for another transaction.

Depth-First Search (DFS) is used to detect cycles in the graph.

If a cycle is detected:

1. one transaction is aborted
2. acquired locks are released
3. wait-for graph entries are cleared

This allows the system to recover from circular wait conditions without permanently blocking transactions.

Deadlock detection was chosen because it allows higher concurrency and more realistic database behavior.



---

## 3. Buffer Pool Design

The simulator models limited database memory using a bounded buffer pool.

The buffer pool uses POSIX semaphores:

* `empty_slots` tracks available slots
* `full_slots` tracks occupied slots

Semaphores were chosen because they naturally model resource availability and blocking behavior in bounded buffer systems.


Transactions attempting to load accounts into a full buffer pool are blocked until a slot becomes available.

This simulates realistic database page loading behavior and demonstrates producer-consumer synchronization concepts.

Each slot stores:

* account identifier
* occupancy status

Mutex protection is used during slot allocation and release operations.


---

## 4. Timer and Metrics System

A dedicated timer thread maintains the global simulation clock.

The timer periodically increments the global tick value and signals waiting transaction threads using a condition variable.

Transactions wait until their assigned start tick before executing operations.

The simulator records runtime metrics including:

* committed transactions
* aborted transactions
* total simulation ticks
* throughput
* transaction execution timing

Metrics are protected using mutex synchronization to ensure thread-safe updates across concurrent transaction threads.


---

## 5. Execution Logging

The simulator includes detailed execution logging to visualize concurrent transaction behavior.

Logs include:

* transaction start and completion
* lock acquisition
* deadlock detection
* transaction aborts
* buffer pool activity

Execution logs help verify synchronization correctness and provide visibility into concurrent execution order.

The logging system was designed primarily for debugging, testing, and demonstrating concurrency behavior during execution.

---
## 6. Screenshots