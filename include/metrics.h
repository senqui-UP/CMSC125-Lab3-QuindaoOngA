#ifndef CMSC125_LAB3_QUINDAOONGA_METRICS_H
#define CMSC125_LAB3_QUINDAOONGA_METRICS_H

#include <pthread.h>

typedef struct {
    int total_transactions;
    int committed;
    int aborted;
    pthread_mutex_t lock;
} Metrics;

/* Global metrics instance */
extern Metrics metrics;

void print_metrics();

#endif //CMSC125_LAB3_QUINDAOONGA_METRICS_H