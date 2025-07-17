#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

ID tskid_1, tskid_2, tskid_3;
ID semid_1;

void task_1(void)
{
	while(1) {
		h_wai_sem(semid_1, 2, TMO_FEVR);
		printf("I am task_1\n");
		h_dly_tsk(1000);
		h_sig_sem(semid_1, 2);
	}
}

void task_2(void)
{
	while(1) {
		h_wai_sem(semid_1, 1, TMO_FEVR);
		printf("I am task_2\n");
		h_dly_tsk(1000);
		h_sig_sem(semid_1, 1);
	}
}

void task_3(void)
{
	while(1) {
		h_wai_sem(semid_1, 1, TMO_FEVR);
		printf("I am task_3\n");
		h_dly_tsk(1000);
		h_sig_sem(semid_1, 1);
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

T_CSEM csem_1 = {
	.isemcnt = 2,
	.maxsem = 2,
};

void usermain(void)
{
	semid_1 = h_cre_sem(&csem_1);

	tskid_1 = h_cre_tsk(&ctsk_1);
	tskid_2 = h_cre_tsk(&ctsk_2);
	tskid_3 = h_cre_tsk(&ctsk_3);

	while(1) {
		printf("I am usermain\n");
		h_slp_tsk(TMO_FEVR);
	}
}
