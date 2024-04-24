#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"
#include "skier.h"
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
    set_skiers(&cfg, atoi(argv[1]));
    set_stops(&cfg, atoi(argv[2]));
    set_capacity(&cfg, atoi(argv[3]));
    set_wait(&cfg, atoi(argv[4]));
    set_bus_ride(&cfg, atoi(argv[5]));
    pid_t pids[256];
    pr_init();

    for (size_t i = 0; i < 20; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            srand(time(NULL) ^ (getpid() << 16)); // totally accurate rng (ironically, I guess)
            skier sk;
            sk_init(&sk, i+1, random() % 10);
            sk_change(&sk);
            exit(EXIT_SUCCESS);
        }
        else if (pids[i] == -1)
        {
            fprintf(stderr, "fork error\n");
            break;
        }
    }
    for (size_t i = 0; i < 20; i++)
    {
        pids[i] = wait(&status);
        // printf("Child with PID %ld exited with status 0x%x.\n", (long)pids[i], status);
    }

    pr_deinit();
    return 0;
}