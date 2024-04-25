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
    unsigned capacity : 7;
    unsigned filled : 7;
    unsigned stop : 4;
    unsigned transported : 15;
    bs_state state;
    sem_t sem_inc;
    int shm_bus;
    unsigned int *waiting;
    sem_t *waiting_sem;
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
    (*bs)->shm_bus = shm;
    // waiting malloc
    if ((shm = shmget(IPC_PRIVATE, sizeof(unsigned int) * (cfg.stops - 1), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate waiting!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }
    if (((*bs)->waiting = shmat(shm, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }

    if ((shm = shmget(IPC_PRIVATE, sizeof(sem_t) * (cfg.stops - 1), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate waiting!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }

    if (((*bs)->waiting_sem = shmat(shm, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print!\n", __FILE__, __LINE__);
        return SHM_ERR;
    }

    if (sem_init(&(*bs)->sem_inc, 1, 1))
    {
        fprintf(stderr, "%s:%d:\tsem_print init failed!\n", __FILE__, __LINE__);
        return SEM_ERR;
    }

    (*bs)->capacity = cfg.capacity;
    (*bs)->transported = 0;
    (*bs)->stop = 15;
    (*bs)->filled = 0;
    (*bs)->state = bs_started;
    for (size_t i = 0; i < (unsigned int)(cfg.stops - 1); i++)
    {
        (*bs)->waiting[i] = 0;
        if (sem_init(&((*bs)->waiting_sem[i]), 1, 1))
        {
            fprintf(stderr, "%s:%d:\tsem_print init failed!\n", __FILE__, __LINE__);
            for (size_t j = 0; j < (i - 1); j++)
            {
                sem_close(&((*bs)->waiting_sem[j]));
            }
            free((*bs)->waiting);
            free((*bs)->waiting_sem);
            return SEM_ERR;
        }
    }
    return N_ERR;
}

void bs_inc(bus *bs, unsigned int index)
{
    sem_wait(&((bs)->waiting_sem[index]));
    printf("incrementing %d\n", (bs)->waiting[index]);
    (bs)->waiting[index] += 1;
    printf("%d\n", (bs)->waiting[index]);
    sem_post(&((bs)->waiting_sem[index]));
}

void bs_get(bus *bs, unsigned int index)
{
    sem_wait(&((bs)->waiting_sem[index]));
    printf("incrementing %d\n", (bs)->waiting[index]);
    (bs)->waiting[index] += 1;
    printf("%d\n", (bs)->waiting[index]);
    sem_post(&((bs)->waiting_sem[index]));
}

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
        print("BUS: lol\n");
        switch (bs->state)
        {
        case bs_started:
            print("BUS: started\n");
            bs->state = bs_en_route;
            break;
        case bs_en_route:
            usleep(rand() % (cfg.bus_ride));
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
                return;
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
    sem_close(&bs->sem_inc);
    for (size_t i = 0; i < (unsigned int)cfg.stops - 1; i++)
        sem_close(&bs->waiting_sem[i]);
    print("detach\n");
    shmdt(bs->waiting);
    shmdt(bs->waiting_sem);

    shmdt(bs);
    return N_ERR;
}

#endif