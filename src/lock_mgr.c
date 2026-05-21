#include <stdbool.h>

#include "lock_mgr.h"

// Wait-for graph used for deadlock detection
// Index = transaction ID
// Value = transaction this transaction is currently waiting for
WaitForEntry wait_graph[MAX_TRANSACTIONS];

// Mutex protecting all accesses/modifications to the wait-for graph
pthread_mutex_t graph_lock =
    PTHREAD_MUTEX_INITIALIZER;

/*
 * Records a dependency in the wait-for graph
 * tx_id       = transaction currently waiting
 * account_id  = account/resource being waited on
 * holder_tx   = transaction currently holding the lock
 */
void record_wait(int tx_id, int account_id, int holder_tx){
    pthread_mutex_lock(&graph_lock);

    wait_graph[tx_id].tx_id = tx_id;
    wait_graph[tx_id].waiting_for_tx = holder_tx;
    wait_graph[tx_id].waiting_for_account = account_id;

    pthread_mutex_unlock(&graph_lock);
}

/*
 * Clears the waiting state of a transaction. Called when:
 * - lock acquisition succeeds
 * - transaction aborts
 * - transaction finishes
 */
void clear_wait(int tx_id){
    pthread_mutex_lock(&graph_lock);

    wait_graph[tx_id].waiting_for_tx = -1;
    wait_graph[tx_id].waiting_for_account = -1;

    pthread_mutex_unlock(&graph_lock);
}

/*
 * Depth-First Search (DFS) helper used for cycle detection. A cycle in the wait-for graph indicates a deadlock
 * visited[]   = tracks already visited transactions
 * rec_stack[] = tracks transactions currently in DFS recursion stack
 */
bool has_cycle(int tx_id, bool visited[], bool rec_stack[]){
    visited[tx_id] = true;
    rec_stack[tx_id] = true;

    // Next transaction this transaction is waiting for
    int next = wait_graph[tx_id].waiting_for_tx;

    if (next != -1) {

        // Continue DFS traversal if next node not yet visited
        if (!visited[next]) {
            if (has_cycle(next, visited, rec_stack)) {
                return true;
            }

        // Back edge found:
        // transaction already exists in recursion stack -> cycle detected
        } else if (rec_stack[next]) {
            return true;
        }
    }

    // Remove transaction from current DFS recursion path
    rec_stack[tx_id] = false;

    return false;
}

/*
 * Detects deadlocks by searching for cycles in the wait-for graph
 * Returns true if deadlock is detected
 */
bool detect_deadlock(){
    pthread_mutex_lock(&graph_lock);

    bool visited[MAX_TRANSACTIONS] = { false };
    bool rec_stack[MAX_TRANSACTIONS] = { false };

    // Run DFS from every transaction node
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {

        if (!visited[i]) {
            if (has_cycle(i, visited, rec_stack)) {

                pthread_mutex_unlock(&graph_lock);
                return true;
            }
        }
    }

    pthread_mutex_unlock(&graph_lock);

    return false;
}