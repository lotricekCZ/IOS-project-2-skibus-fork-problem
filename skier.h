#ifndef SKIER_H
#define SKIER_H

#include "print.h"
#include "config.h"
// #include "bus.h"

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
        sleep(rand() % cfg.ski_wait); // should be safe since I initialised srand()
        ski->state = sk_waiting;
        break;
    case sk_waiting:
        print("L %d: arrived to %d\n", ski->id, (int)(ski->bus_stop));
        ski->state = sk_boarding;
        break;
    case sk_boarding:
        print("L %d: boarding\n", ski->id);
        ski->state = sk_aboard;
        break;
    case sk_aboard:
        ski->state = sk_exiting;
        break;
    case sk_exiting:
        print("L %d: going to ski\n", ski->id);
        ski->state = sk_done;
        break;
    default:
        break;
    }
    if (ski->state != sk_done)
        sk_change(ski);
}

#endif