#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

FLGCB flgcb_tbl[MAX_FLG];

_Bool check_flgptn(uint32_t flgptn, uint32_t waiptn, uint32_t wfmode)
{
	_Bool   rtncd;

	switch (wfmode) {
	case TWF_ANDW:
		rtncd = ((flgptn & waiptn) == waiptn)?1:0;
		break;
	case TWF_ORW:
		rtncd = (flgptn & waiptn)?1:0;
		break;
	default:
		rtncd = 0;
		break;
	}
	return rtncd;
}


ID h_cre_flg(T_CFLG *pk_cflg)
{
    uint32_t i;
    ID rtncd;

    BEGIN_CRITICAL_SECTION

    for(i = 0; i < MAX_FLG; i++) {
        if (flgcb_tbl[i].status == OBJST_NON) {
            flgcb_tbl[i].status = OBJST_EXIST;
            flgcb_tbl[i].flgptn = pk_cflg->iflgptn;
            break;
        }
    }
    if(i < MAX_FLG) rtncd = i + 1;
    else rtncd = (ID)E_LIMIT;

    END_CRITICAL_SECTION
    return rtncd;
}

ER h_clr_flg(ID flgid, uint32_t clrptn)
{
	FLGCB   *flgcb;
	ER      ercd = E_OK;

	if(!(flgid >0 && flgid < MAX_FLG)) {
		return E_ID;
	}

	BEGIN_CRITICAL_SECTION

	flgcb = &flgcb_tbl[flgid - 1];
	if(flgcb->status == OBJST_EXIST) {
		flgcb->flgptn &= ~clrptn;
	} else {
		ercd = E_NOEXS;
	}

	END_CRITICAL_SECTION
	return ercd;
}

ER h_set_flg(ID flgid, uint32_t setptn)
{
	FLGCB   *flgcb;
	ER      ercd = E_OK;

	if(!(flgid >0 && flgid < MAX_FLG)) {
		return E_ID;
	}

	BEGIN_CRITICAL_SECTION

	flgcb = &flgcb_tbl[flgid - 1];
	if(flgcb->status == OBJST_EXIST) {
		flgcb->flgptn |= setptn;

		for(TCB *tcb = wait_queue; tcb != NULL; tcb = tcb->next) {
			if(tcb->waitfct == WAITFCT_FLG && tcb->wobjid == flgid 
				&& check_flgptn(flgcb->flgptn, tcb->waiptn, tcb->wfmode)) {

				tqueue_remove_entry(&wait_queue, tcb);

				tcb->status    = TSKST_READY;
				tcb->waitfct   = WAITFCT_NON;
				tcb->waittim   = 0;
				*tcb->p_flgptn  = flgcb->flgptn;

				flgcb->flgptn &= ~(tcb->waiptn);

				tqueue_add_entry(&ready_queue[tcb->tskpri -1 ], tcb);
				scheduler();
				break;
			}
		}
	} else {
		ercd = E_NOEXS;
	}

	END_CRITICAL_SECTION
	return ercd;
}

ER h_wai_flg(ID flgid, uint32_t waiptn, uint32_t wfmode, uint32_t *p_flgptn, TMO tmout)
{
    FLGCB *flgcb;
    ER ercd = E_OK;

    if(!(flgid > 0 && flgid < MAX_FLG)) {
        return E_ID;
    }

    BEGIN_CRITICAL_SECTION

    flgcb = &flgcb_tbl[flgid - 1];
    if(flgcb->status != OBJST_EXIST) {
        ercd = E_NOEXS;
    }
    else {
        if (check_flgptn(flgcb->flgptn, waiptn, wfmode)) {
            *p_flgptn = flgcb->flgptn;
            flgcb->flgptn &= ~waiptn;
        }
        else if(tmout == TMO_POL) {
            ercd = E_TMOUT;
        }
        else {
            tqueue_remove_entry(&ready_queue[cur_task->tskpri - 1], cur_task);

            cur_task->status = TSKST_WAIT;
            cur_task->waitfct = WAITFCT_FLG;
            cur_task->waittim = tmout;
            cur_task->ercd = &ercd;
            cur_task->wobjid = flgid;
            cur_task->waiptn = waiptn;
            cur_task->wfmode = wfmode;
            cur_task->p_flgptn = p_flgptn;

            tqueue_add_entry(&wait_queue, cur_task);
            scheduler();
        }
    }

    END_CRITICAL_SECTION
    return ercd;
}