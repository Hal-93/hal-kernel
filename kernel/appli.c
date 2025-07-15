#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>
#include "kernel.h"

ID tskid_1, tskid_2, tskid_3, tskid_4;
ID flgid_1;

void task_1(void)
{
	uint32_t	count = 0;
	while(1) {
		printf("I am task_1\n");
		
		h_set_flg(flgid_1, 1<<count);
		if(++count>2) count = 0;
		h_dly_tsk(300);
	}
}

void task_2(void)
{
	uint32_t	flgptn;
	while(1) {
		h_wai_flg(flgid_1, 1<<0, TWF_ANDW, &flgptn, TMO_FEVR);
		printf("I am task_2\n");
	}
}

void task_3(void)
{
	uint32_t	flgptn;
	while(1) {
		h_wai_flg(flgid_1, 1<<1, TWF_ANDW, &flgptn, TMO_FEVR);
		printf("I am task_3\n");
	}
}

void task_4(void)
{
	uint32_t	flgptn;
	while(1) {
		h_wai_flg(flgid_1, 1<<2, TWF_ANDW, &flgptn, TMO_FEVR);
		printf("I am task_4\n");
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

T_CFLG cflg_1 = {
	.iflgptn = 0,
};

void usermain(void)
{
	flgid_1 = h_cre_flg(&cflg_1);
	tskid_1 = h_cre_tsk(&ctsk_1);
	tskid_2 = h_cre_tsk(&ctsk_2);
	tskid_3 = h_cre_tsk(&ctsk_3);
	tskid_4 = h_cre_tsk(&ctsk_4);

	while(1) {
		printf("I am usermain\n");
		h_dly_tsk(TMO_FEVR);
	}
}
