#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/shm.h>
#include "errors.h"

#ifndef PRINT_H
#define PRINT_H

static unsigned int *pr_count;
// print semaphore
sem_t sem_print;
static int shm_pr_count;

// print semaphore initialisation
error pr_init()
{
    
    // semaphore init & check
    if (sem_init(&sem_print, 1, 1))
    {
        fprintf(stderr, "%s:%d:\tsem_print init failed!\n", __FILE__, __LINE__);
        sem_close(&sem_print);
        return SEM_ERR;
    }
    
    if ((shm_pr_count = shmget(IPC_PRIVATE, sizeof(unsigned int), IPC_CREAT | 0666)) == SHMG_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print!\n", __FILE__, __LINE__);
        sem_close(&sem_print);
        return SHM_ERR;
    }
    if ((pr_count = shmat(shm_pr_count, NULL, 0)) == SHMA_ERR)
    {
        fprintf(stderr, "%s:%d:\tCannot allocate shm_sem_print\n", __FILE__, __LINE__);
        sem_close(&sem_print);
        return SHM_ERR;
    }
    return N_ERR;
}
// semaphored print
void print(char *msg, ...)
{
    va_list args;
    va_start(args, msg);
    sem_wait(&sem_print);
    printf("%d: ", (*pr_count)++);
    vprintf(msg, args);
    fflush(stdout);
    sem_post(&sem_print);
    va_end(args);
}

// print semaphore deinitialisation
void pr_deinit()
{
    sem_close(&sem_print);
    shmdt(pr_count);
}

#endif