#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include "kernel.h"

ID tskid_1, tskid_2, tskid_3;

void task_1(void)
{
	while(1) {
		printf("I am task_1\n");
		h_dly_tsk(500);
		h_wup_tsk(tskid_2);
		h_slp_tsk(TMO_FEVR);
	}
}

void task_2(void)
{
	while(1) {
		printf("I am task_2\n");
		h_dly_tsk(500);
		h_wup_tsk(tskid_1);
		h_slp_tsk(TMO_FEVR);
	}
}

void task_3(void)
{
	ER		ercd;

	while(1) {
		ercd = h_slp_tsk(500);
		printf("I am task_3  ercd = %d\n", ercd);
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

void usermain(void)
{
	tskid_1 = h_cre_tsk(&ctsk_1);
	tskid_2 = h_cre_tsk(&ctsk_2);
	tskid_3 = h_cre_tsk(&ctsk_3);

	while(1) {
		printf("I am usermain\n");
		h_slp_tsk(TMO_FEVR);
	}
}
