#include "kernel.h"
#include <unistd.h>
#include <stdio.h>

void task_1(void)
{
    while (1)
    {
        printf("I am task_1\n");
        sleep(1);
        scheduler();
    }
}

void task_2(void)
{
    while (1)
    {
        printf("I am task_2\n");
        sleep(1);
        scheduler();
    }
}

void task_3(void)
{
    while (1)
    {
        printf("I am task_3\n");
        sleep(1);
        scheduler();
    }
}

void task_4(void)
{
    while (1)
    {
        printf("I am task_4\n");
        sleep(1);
        scheduler();
    }
}

T_CTSK ctsk_1 = { .task = task_1 };
T_CTSK ctsk_2 = { .task = task_2 };
T_CTSK ctsk_3 = { .task = task_3 };
T_CTSK ctsk_4 = { .task = task_4 };

void usermain(void) 
{
    h_cre_tsk(&ctsk_1);
    h_cre_tsk(&ctsk_2);
    h_cre_tsk(&ctsk_3);
    h_cre_tsk(&ctsk_4);

    while(1) {
        printf("I am init-task\n");
        sleep(1);
        scheduler();
    }
}