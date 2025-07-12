#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

void task_1(void)
{
	while(1) {
		printf("I am task_1\n");
		h_dly_tsk(500);
	}
}

void task_2(void)
{
	while(1) {
		printf("I am task_2\n");
		h_dly_tsk(500);
	}
}

void task_3(void)
{
	while(1) {
		printf("I am task_3\n");
		h_dly_tsk(500);
	}
}

void task_4(void)
{
	while(1) {
		printf("I am task_4\n");
		h_dly_tsk(500);
	}
}

T_CTSK ctsk_1 = {
	.task = task_1,
	.tskpri = 1,
};
T_CTSK ctsk_2 = {
	.task = task_2,
	.tskpri = 2,
};
T_CTSK ctsk_3 = {
	.task = task_3,
	.tskpri = 3,
};
T_CTSK ctsk_4 = {
	.task = task_4,
	.tskpri = 4,
};

void usermain(void)
{
	h_cre_tsk(&ctsk_1);
	h_cre_tsk(&ctsk_2);
	h_cre_tsk(&ctsk_3);
	h_cre_tsk(&ctsk_4);

	while(1) {
		printf("I am usermain\n");
		h_dly_tsk(500);
	}
}
