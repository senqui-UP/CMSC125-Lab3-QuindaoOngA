#include <stdio.h>
#include <pthread.h>

#include "timer.h"

int main()
{
    pthread_t timer;

    int tick_ms = 100;

    pthread_create(&timer,
                   NULL,
                   timer_thread,
                   &tick_ms);

    wait_until_tick(5);

    printf("Reached tick %d\n",
           global_tick);

    simulation_running = false;

    pthread_join(timer, NULL);

    return 0;
}