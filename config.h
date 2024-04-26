#ifndef CONFIG_H
#define CONFIG_H
typedef struct config config;

struct config
{
    short skiers;
    unsigned stops : 4;
    unsigned capacity : 7;
    unsigned ski_wait : 14;
    unsigned bus_ride : 10;
    FILE * out;
};
config cfg;

void set_skiers(config *cfg, int skiers)
{
    if (skiers >= 20000 || skiers < 0) // although bottom bound was not defined I am not willing to consider it a "feature".
    {
        fprintf(stderr, "ERROR: Entry SKIERS out of the bounds.\n");
        exit(EXIT_FAILURE);
    }
    cfg->skiers = skiers;
}

void set_capacity(config *cfg, int cap)
{
    if (cap > 100 || cap < 10)
    {
        fprintf(stderr, "ERROR: Entry CAPACITY out of the bounds.\n");
        exit(EXIT_FAILURE);
    }
    cfg->capacity = cap;
}

void set_stops(config *cfg, int stops)
{
    if (stops > 10 || stops <= 0)
    {
        fprintf(stderr, "ERROR: Entry STOPS out of the bounds.\n");
        exit(EXIT_FAILURE);
    }
    cfg->stops = stops;
}

void set_wait(config *cfg, int wait)
{
    if (wait > 10000 || wait < 0)
    {
        fprintf(stderr, "ERROR: Entry WAIT out of the bounds.\n");
        exit(EXIT_FAILURE);
    }
    cfg->ski_wait = wait;
}

void set_bus_ride(config *cfg, int bus_ride)
{
    if (bus_ride > 1000 || bus_ride < 0)
    {
        fprintf(stderr, "ERROR: Entry BUS ride out of the bounds.\n");
        exit(EXIT_FAILURE);
    }
    cfg->bus_ride = bus_ride;
}
#endif