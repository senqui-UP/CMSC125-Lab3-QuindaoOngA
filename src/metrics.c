#include <stdio.h>

#include "metrics.h"
#include "timer.h"

Metrics metrics;

void print_metrics() {
    printf("\n=== Summary ===\n");
    printf("Total transactions          : %d\n", metrics.total_transactions);
    printf("Committed                   : %d\n", metrics.committed);
    printf("Aborted                     : %d\n", metrics.aborted);
    printf("Total ticks                 : %d\n", global_tick);

    printf("\n=== Buffer Pool Statistics ===\n");
    printf("Pool size                   : %d\n", BUFFER_POOL_SIZE);
    printf("Total loads                 : %d\n", metrics.buffer_loads);
    printf("Total unloads               : %d\n", metrics.buffer_unloads);
    printf("Blocked operations (pool full): %d\n", metrics.buffer_waits);

    printf("\n=== Deadlock Statistics ===\n");
    printf("Deadlocks prevented         : %d\n", metrics.deadlocks_prevented);
    printf("Deadlocks detected          : %d\n", metrics.deadlocks_detected);
}