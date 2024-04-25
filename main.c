#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "config.h"
#include "skier.h"
#include "bus.h"
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
    pid_t *pids = malloc((cfg.skiers + 1) * sizeof(pid_t));
    pr_init();
    if(bs_init(&skibus)) exit(EXIT_FAILURE); // bs probably isn't the best name

    for (size_t i = 0; i <= (unsigned int)cfg.skiers; i++)
    {
        pids[i] = fork();
        if (pids[i] == 0)
        {
            srand(time(NULL) ^ (getpid() << 16)); // totally accurate rng (ironically, I guess)
            free(pids);
            if(i == 0){
                print("bus entry\n");
                print("bus init\n");
                bs_change(skibus);
                print("bus end\n");
                exit(EXIT_SUCCESS);    
            }
            skier sk;
            sk_init(&sk, i, random() % (cfg.stops - 1));
            sk_change(&sk);
            exit(EXIT_SUCCESS);
        }
        else if (pids[i] == -1)
        {
            fprintf(stderr, "fork error\n");
            break;
        }
    }
    for (size_t i = 0; i <= (unsigned int)cfg.skiers; i++)
    {
        pids[i] = wait(&status);
        printf("Child with PID %ld exited with status 0x%x.\n", (long)pids[i], status);
    }
    printf("ST1\tST2\tST3\tST4\tST5\tST6\tST7\tST8\tST9\n%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", 
        skibus->waiting[0],
        skibus->waiting[1],
        skibus->waiting[2],
        skibus->waiting[3],
        skibus->waiting[4],
        skibus->waiting[5],
        skibus->waiting[6],
        skibus->waiting[7],
        skibus->waiting[8]
        );
    free(pids);
    bs_deinit(skibus);
    pr_deinit();
    return 0;
}