#ifndef CMSC125_LAB3_QUINDAOONGA_TIMER_H
#define CMSC125_LAB3_QUINDAOONGA_TIMER_H

#include <pthread.h>
#include <stdbool.h>

extern volatile int global_tick;
extern bool simulation_running;

extern pthread_mutex_t tick_lock;
extern pthread_cond_t tick_changed;
extern pthread_mutex_t print_lock;   // <-- add this

void *timer_thread(void *arg);
void wait_until_tick(int target_tick);

#endif //CMSC125_LAB3_QUINDAOONGA_TIMER_H