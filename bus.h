#ifndef BUS_H
#define BUS_H
#include "errors.h"
#include "config.h"

enum bus_state
{
    bs_started,
    bs_en_route,
    bs_arrived,
    bs_leaving,
    bs_finish,
};
typedef enum bus_state bs_state;

struct bus
{
    unsigned filled : 7;
    unsigned stop : 4;
    unsigned transported : 15;
    bs_state state;
    // semaphore used for current number of passengers
    sem_t capacity;
    // semaphore for marking the last passenger getting on board
    sem_t all_aboard;
    unsigned int *waiting;
    // semaphore for incrementation
    sem_t *waiting_sem;
    // semaphore for allowing boarding on a specific station
    sem_t *boarding_sem;
};

typedef struct bus bus;
bus *skibus;

error bs_init(bus **bs)
{
    int shm;
    if ((shm = shmget(IPC_PRIVATE, sizeof(struct bus), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_bus!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }
    if (((*bs) = shmat(shm, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }
    // end
    //  waiting incrementor
    if ((shm = shmget(IPC_PRIVATE, sizeof(unsigned int) * (cfg.stops), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate waiting!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }
    if (((*bs)->waiting = shmat(shm, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }
    // end
    // waiting incrementing semaphores for incrementing on specific bus stop
    if ((shm = shmget(IPC_PRIVATE, sizeof(sem_t) * (cfg.stops), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate waiting!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }

    if (((*bs)->waiting_sem = shmat(shm, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_waiting_sem!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }
    // end
    // semaphore that every passenger needs to pass in order to board, it is station dependent
    if ((shm = shmget(IPC_PRIVATE, sizeof(sem_t) * (cfg.stops), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate waiting!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }

    if (((*bs)->boarding_sem = shmat(shm, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }

    // free capacity semaphore
    if (sem_init(&(*bs)->capacity, 1, cfg.capacity))
    {
        fprintf(stderr, "%s:%d:\tsem_print init failed!\n", __FILE__, __LINE__);
        return SEM_ERR;
    }

    // last passenger entering
    if (sem_init(&(*bs)->all_aboard, 1, 1))
    {
        fprintf(stderr, "%s:%d:\tsem_print init failed!\n", __FILE__, __LINE__);
        return SEM_ERR;
    }

    (*bs)->transported = 0;
    (*bs)->stop = 0;
    (*bs)->filled = 0;
    (*bs)->state = bs_started;

    for (size_t i = 0; i < (unsigned int)(cfg.stops); i++)
    {
        (*bs)->waiting[i] = 0;
        if (sem_init(&((*bs)->waiting_sem[i]), 1, 1) || sem_init(&((*bs)->boarding_sem[i]), 1, 0))
        {
            fprintf(stderr, "%s:%d:\twaiting_sem init failed!\n", __FILE__, __LINE__);
            for (size_t j = 0; j < (i - 1); j++)
            {
                sem_close(&((*bs)->waiting_sem[j]));
                sem_close(&((*bs)->boarding_sem[j]));
            }
            shmdt((*bs)->waiting);
            shmdt((*bs)->waiting_sem);
            shmdt((*bs)->boarding_sem);
            shmdt(*bs);
            return SEM_ERR;
        }
    }

    return N_ERR;
}

// bus semaphore for waiting to the last passenger
void bs_wait(bus *bs)
{
    sem_wait(&(bs->all_aboard));
}

// bus semaphore used for nulling on arrival, locks bs_wait
void bs_arrive(bus *bs)
{
    sem_wait(&(bs->all_aboard));
}

// bus semaphore for indicating last passenger is on board, unlocks bs_wait
void bs_depart(bus *bs)
{
    sem_post(&(bs->all_aboard));
}

// for indicating passenger went aboard
void bs_board(bus *bs)
{
    sem_wait(&(bs->capacity));
}
// for indicating passenger has exited
void bs_exit(bus *bs)
{
    bs->transported++;
    sem_post(&(bs->capacity));
}
// get the number of waiting people on bus stop
unsigned int bs_get(bus *bs, unsigned int index)
{
    return (bs)->waiting[index];
}
// get number of free spaces aboard
int bs_free(bus *bs)
{
    int ret;
    sem_getvalue(&((bs)->capacity), &ret);
    return ret;
}

// get number of people aboard
int bs_aboard(bus *bs)
{
    return cfg.capacity - bs_free(bs);
}
// increment number of people on specific station
void bs_inc(bus *bs, unsigned int index)
{
    sem_wait(&((bs)->waiting_sem[index]));
    (bs)->waiting[index] += 1;
    sem_post(&((bs)->waiting_sem[index]));
}

// decrement number of people on specific station when boarding
void bs_dec(bus *bs, unsigned int index)
{
    sem_wait(&(bs->waiting_sem[index]));
    bs->waiting[index]--;
    sem_post(&(bs->waiting_sem[index]));
}

void bs_change(bus *bs)
{
    while (bs->state != bs_finish)
    {
        switch (bs->state)
        {
        case bs_started:
            print("BUS: started\n");
            bs->state = bs_en_route;
            break;
        case bs_en_route:
            usleep(rand() % (cfg.bus_ride));
            bs_arrive(bs);
            bs->state = bs_arrived;
            break;
        case bs_arrived:
            ++bs->stop;
            if (bs->stop == cfg.stops)
            {
                print("BUS: arrived to final\n");
            }
            else
            {
                // create some mechanism to inform skiers to try enter
                print("BUS: arrived to %d\n", (int)(bs->stop));
            }

            if (((bs->stop == cfg.stops) && bs_aboard(bs)) || (bs->waiting[bs->stop - 1] && bs_free(bs) != 0)){
                sem_post(&bs->boarding_sem[bs->stop - 1]);
                bs_wait(bs); // wait until everyone boards or exits
                bs_depart(bs); // redo in order to negate effects of bs_wait decrementation
                sem_wait(&bs->boarding_sem[bs->stop - 1]); // take back the semaphore when everyone is aboard or has exited
                } else bs_depart(bs); // calls depart on itself when no passengers are on the stop
            bs->state = bs_leaving;
            break;
        case bs_leaving:
            if (bs->stop == cfg.stops)
            {
                print("BUS: leaving final\n");
                bs->transported += bs->filled;
                bs->filled = 0;
                if (bs->transported == cfg.skiers)
                {
                    usleep(rand() % cfg.bus_ride);
                    bs->state = bs_finish;
                }
                else
                {
                    bs->stop = 0;
                    bs->state = bs_en_route;
                }
            }
            else
            {
                print("BUS: leaving %d\n", (int)(bs->stop));
                bs->state = bs_en_route;
            }
            break;
        default:
            fprintf(stderr, "BUS: problem\n");
            break;
        }
    }
    print("BUS: finish\n");
}

error bs_deinit(bus *bs)
{
    sem_close(&bs->capacity);
    sem_close(&bs->all_aboard);
    for (size_t i = 0; i < (unsigned int)cfg.stops; i++)
    {
        sem_close(&bs->waiting_sem[i]);
        sem_close(&bs->boarding_sem[i]);
    }
    shmdt(bs->waiting);
    shmdt(bs->waiting_sem);
    shmdt(bs->boarding_sem);
    shmdt(&bs->capacity);
    shmdt(&bs->all_aboard);
    shmdt(bs);
    return N_ERR;
}

#endif