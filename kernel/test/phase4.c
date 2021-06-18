#include "horloge.h"

void p1(void)
{
    for (;;)
    {
        printf(".");
        wait_clock(current_clock() + 1 * CLOCKFREQ);
    }
}

void p2(void)
{
    for (;;)
    {
        printf("-");
        wait_clock(current_clock() + 2 * CLOCKFREQ);
    }
}

void p3(void)
{
    for (;;)
    {
        printf("+");
        wait_clock(current_clock() + 5 * CLOCKFREQ);
    }
}

void p4(void)
{
    for (;;)
    {
        printf("*");
        wait_clock(current_clock() + 10 * CLOCKFREQ);
    }
}

void prog_phase4(void)
{
    int pid1 = start((int (*)(void *))(p1), 4000, 128, "p1", NULL);
    int pid2 = start((int (*)(void *))(p2), 4000, 128, "p2", NULL);
    int pid3 = start((int (*)(void *))(p3), 4000, 128, "p3", NULL);
    int pid4 = start((int (*)(void *))(p4), 4000, 128, "p4", NULL);
    wait_clock(current_clock() + 60 * CLOCKFREQ);
    kill(pid1);
    kill(pid2);
    kill(pid3);
    kill(pid4);
    printf("\nend");
}
