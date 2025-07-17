#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

SEMCB  semcb_tbl[MAX_SEM];

ID h_cre_sem( T_CSEM *pk_csem )
{
	uint32_t        i;
	ID				rtncd;

	BEGIN_CRITICAL_SECTION

	for(i = 0; i < MAX_SEM; i++ ) {
		if(semcb_tbl[i].status == OBJST_NON) {
			semcb_tbl[i].status = OBJST_EXIST;
			semcb_tbl[i].semcnt = pk_csem->isemcnt;
			semcb_tbl[i].maxsem	= pk_csem->maxsem;
			break;
		}
	}
	if(i < MAX_SEM) rtncd = i + 1;
	else rtncd = (ID)E_LIMIT;

	END_CRITICAL_SECTION
	return rtncd;
}

ER h_wai_sem( ID semid, uint32_t cnt, TMO tmout )
{
	SEMCB	*semcb;
	ER		ercd = E_OK;

	if(!(semid > 0 && semid < MAX_SEM)) {
		return E_ID;
	}

	BEGIN_CRITICAL_SECTION

	semcb = &semcb_tbl[semid - 1];
	if(semcb->status != OBJST_EXIST) {
		ercd = E_NOEXS;
	} else {
		if(semcb->semcnt >=  cnt) {
			semcb->semcnt -= cnt;
		} else if(tmout == TMO_POL) {
			ercd = E_TMOUT;
		} else {
			tqueue_remove_entry(&ready_queue[cur_task->tskpri -1 ], cur_task);

			cur_task->status    = TSKST_WAIT;
			cur_task->waitfct   = WAITFCT_SEM;
			cur_task->waittim   = tmout;
			cur_task->ercd      = &ercd;
			cur_task->wobjid    = semid;
			cur_task->semcnt	= cnt;

			tqueue_add_entry(&wait_queue, cur_task);
			scheduler();
		}
	}

	END_CRITICAL_SECTION
	return ercd;
}

ER h_sig_sem( ID semid, uint32_t cnt)
{
	SEMCB		*semcb;
	ER			ercd = E_OK;

	if(!(semid > 0 && semid < MAX_SEM)) {
		return E_ID;
	}

	BEGIN_CRITICAL_SECTION

	semcb = &semcb_tbl[semid - 1];
	if(semcb->status == OBJST_EXIST) {
		if(semcb->semcnt + cnt > semcb->maxsem) {
			ercd = E_QOVR;
		} else {
			semcb->semcnt += cnt;

			TCB		*next;
			for(TCB *tcb = wait_queue; tcb != NULL; tcb = next) {
				next = tcb->next;
				if(tcb->waitfct == WAITFCT_SEM && tcb->wobjid == semid && semcb->semcnt >= tcb->semcnt) {
					tqueue_remove_entry(&wait_queue, tcb);

					tcb->status    = TSKST_READY;
					tcb->waitfct   = WAITFCT_NON;
					tcb->waittim   = 0;

					semcb->semcnt -= tcb->semcnt;

					tqueue_add_entry(&ready_queue[tcb->tskpri -1 ], tcb);
				}
			}
			scheduler();
		}
	} else {
		ercd = E_NOEXS;
	}

	END_CRITICAL_SECTION
	return ercd;
}