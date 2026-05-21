#include <stdio.h>

#include "metrics.h"
#include "timer.h"

Metrics metrics;

void print_metrics()
{
    printf("\n=== Summary ===\n");
    printf("Total transactions : %d\n",
           metrics.total_transactions);

    printf("Committed : %d\n",
           metrics.committed);

    printf("Aborted : %d\n",
           metrics.aborted);

    printf("Total ticks : %d\n",
           global_tick);
}