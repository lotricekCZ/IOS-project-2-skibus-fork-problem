#ifndef SKIER_H
#define SKIER_H

#include "print.h"
#include "config.h"
#include "bus.h"

enum skier_state
{
    sk_start,
    sk_breakfast,
    sk_waiting,
    sk_boarding,
    sk_aboard,
    sk_exiting,
    sk_done
};

typedef enum skier_state sk_state;

struct skier
{
    unsigned int id;
    unsigned bus_stop : 4;
    enum skier_state state;
};
typedef struct skier skier;

void sk_init(skier *ski, unsigned int id, unsigned char stop)
{
    ski->id = id;
    ski->state = sk_start;
    ski->bus_stop = stop & 0xf;
}

// wait for specific mutexes
void sk_change(skier *ski)
{
    switch (ski->state)
    {
    case sk_start:
        print("L %d: started\n", ski->id);
        ski->state = sk_breakfast;
        break;
    case sk_breakfast:
        usleep(rand() % cfg.ski_wait); // should be safe since I initialised srand()
        ski->state = sk_waiting;
        break;
    case sk_waiting:
        print("L %d: arrived to %d\n", ski->id, (int)(ski->bus_stop) + 1);
        bs_inc(skibus, (int)(ski->bus_stop));
        ski->state = sk_boarding;
        break;
    case sk_boarding: // sk boarding needs to first assure that it has boarding permission on that specific stop
        sem_wait(&(skibus->boarding_sem)[(int)(ski->bus_stop)]);
        if (bs_free(skibus) == 0){ // inform skier no space is left and to give up the flag
            sem_post(&(skibus->boarding_sem)[(int)(ski->bus_stop)]);
            break;
        }
        bs_board(skibus);
        print("L %d: boarding\n", ski->id);
        bs_dec(skibus, ski->bus_stop);
        if (bs_free(skibus) == 0 || bs_get(skibus, (ski->bus_stop)) == 0){ // inform bus to depart
            bs_depart(skibus);
            }
        sem_post(&(skibus->boarding_sem)[(int)(ski->bus_stop)]);
        ski->state = sk_aboard;
        break;
    case sk_aboard:
        sem_wait(&(skibus->boarding_sem)[cfg.stops - 1]);
        ski->state = sk_exiting;
        break;
    case sk_exiting:
        bs_exit(skibus);
        print("L %d: going to ski\n", ski->id);
        ski->state = sk_done;
        sem_post(&(skibus->boarding_sem)[cfg.stops - 1]);
        if (bs_aboard(skibus) == 0) // inform bus to depart
            bs_depart(skibus);
        break;
    default:
        break;
    }
    if (ski->state != sk_done)
        sk_change(ski);
}

#endif