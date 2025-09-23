#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

ID tskid_1, tskid_2, tskid_3, tskid_4;

#define	MBFBUFSZ	100
#define MAX_MSGSZ	12
ID mbfid_1;

#if 1
void task_1(void)
{
	const char	msg[] = "I am Task-1";
	while(1) {
		h_snd_mbf(mbfid_1, msg, sizeof(msg), TMO_FEVR);
		h_dly_tsk(1000);
	}
}

void task_2(void)
{
	const char	msg[] = "I am Task-2";
	while(1) {
		h_snd_mbf(mbfid_1, msg, sizeof(msg), TMO_FEVR);
		h_dly_tsk(1000);
	}
}

void task_3(void)
{
	const char	msg[] = "I am Task-3";
	while(1) {
		h_snd_mbf(mbfid_1, msg, sizeof(msg), TMO_FEVR);
		h_dly_tsk(1000);
	}
}

void task_4(void)
{
	uint32_t	msgsz;

	uint8_t		msg[MAX_MSGSZ];
	while(1) {
		msgsz = h_rcv_mbf(mbfid_1, msg, TMO_FEVR);
		printf("Message size = %d  \"%s\"\n", msgsz, msg);
	}
}
#endif
#if 0
void task_1(void)
{
	const char	msg[] = "I am Task-1";
	while(1) {
		h_snd_mbf(mbfid_1, msg, sizeof(msg), TMO_FEVR);
		h_dly_tsk(1000);
	}
}

void task_2(void)
{
	const char	msg[] = "I am Task-2";
	while(1) {
		h_snd_mbf(mbfid_1, msg, sizeof(msg), TMO_FEVR);
		h_dly_tsk(1000);
	}
}

void task_3(void)
{
	const char	msg[] = "I am Task-3";
	while(1) {
		h_snd_mbf(mbfid_1, msg, sizeof(msg), TMO_FEVR);
		h_dly_tsk(1000);
	}
}

void task_4(void)
{
	uint32_t	msgsz;

	uint8_t		msg[MAX_MSGSZ];
	while(1) {
		msgsz = h_rcv_mbf(mbfid_1, msg, TMO_FEVR);
		printf("Message size = %d  \"%s\"\n", msgsz, msg);
	}
}
#endif

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

uint8_t		mbfbuf_1[MBFBUFSZ];
T_CMBF cmbf_1 = {
	.bufsz	= MBFBUFSZ,
	.maxsz	= MAX_MSGSZ,
	.bufptr	= mbfbuf_1,
};

void usermain(void)
{
	mbfid_1 = h_cre_mbf(&cmbf_1);

	tskid_1 = h_cre_tsk(&ctsk_1);
	tskid_2 = h_cre_tsk(&ctsk_2);
	tskid_3 = h_cre_tsk(&ctsk_3);
	tskid_4 = h_cre_tsk(&ctsk_4);

	while(1) {
		printf("I am usermain\n");
		h_slp_tsk(TMO_FEVR);
	}
}
