#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"
#include "print.h"

// ./proj2 L Z K TL TB
// L    (argv[1]) skiers
// Z    (argv[2]) stops
// K    (argv[3]) capacity
// TL   (argv[4]) time skier
// TB   (argv[5]) time between
int status;
int main(int argc, char **argv)
{
    if (argc != 6)
        return EXIT_FAILURE;
    config config;
    set_skiers(&config, atoi(argv[1]));
    set_stops(&config, atoi(argv[2]));
    set_capacity(&config, atoi(argv[3]));
    set_wait(&config, atoi(argv[4]));
    set_bus_ride(&config, atoi(argv[5]));
    pid_t pids[256];
    pid_t rpids[256];
    long random_delay[256];
    for(size_t i = 0; i < 256; i++){
        random_delay[i] = random() % 20000000;
    }

    for(size_t i = 0; i < 20; i++){
        printf("forking child %ld\n", i);
        pids[i] = fork();
        if(pids[i] == 0){
            char msg[64];
            usleep(random_delay[i]);
            sprintf(msg, "hello from %d with delay %ld\n", getpid(), random_delay[i]);
            printf(msg);
            exit(EXIT_SUCCESS);
        } else if(pids[i] == -1) {
            printf("fork error");
            break;
        }
    }

    for(size_t i = 0; i < 20; i++){
        rpids[i] = wait(&status);
        printf("Child with PID %ld exited with status 0x%x.\n", (long)rpids[i], status);
    }
    pr_init();
    
    usleep(100000);
    pr_deinit();
    return 0;
}