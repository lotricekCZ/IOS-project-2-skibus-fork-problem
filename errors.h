#ifndef ERRORS_H
#define ERRORS_H

#define SHMA_ERR (void *)-1
#define SHMG_ERR -1
#define SHMD_ERR -1

enum error
{
    N_ERR,   // NO ERROR
    SHM_ERR, // shared memory error
    SEM_ERR  // semaphore error
};
typedef enum error error;

#endif