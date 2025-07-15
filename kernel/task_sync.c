#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

ER h_dly_tsk(RELTIM dlytim)
{
    BEGIN_CRITICAL_SECTION

    tqueue_remove_entry(&ready_queue[cur_task->tskpri-1], cur_task);
    cur_task->status = TSKST_WAIT;
    cur_task->waitfct = WAITFCT_DLY;
    cur_task->waittim = dlytim;
    tqueue_add_entry(&wait_queue, cur_task);
    scheduler();

    END_CRITICAL_SECTION
    return E_OK;
}

ER h_slp_tsk(TMO tmout)
{
    ER ercd = E_OK;

    BEGIN_CRITICAL_SECTION

    if (cur_task->wupcnt > 0) {
        cur_task->wupcnt--;
    }
    else {
        tqueue_remove_entry(&ready_queue[cur_task->tskpri-1], cur_task);
        cur_task->status = TSKST_WAIT;
        cur_task->waitfct = WAITFCT_SLP;
        cur_task->waittim = (RELTIM)tmout;
        cur_task->ercd = &ercd;
        tqueue_add_entry(&wait_queue, cur_task);
        scheduler();
    }
    
    END_CRITICAL_SECTION
    return ercd;
}

ER h_wup_tsk(ID tskid)
{
    ER ercd = E_OK;
    TCB *tcb;

    if(!(tskid > 0 && tskid < MAX_TASK)) {
        return E_ID;
    }

    BEGIN_CRITICAL_SECTION

    tcb = &tcb_tbl[tskid-1];
    if(tcb->status == TSKST_NON || tcb->status == TSKST_DORMANT) {
        ercd = E_OBJ;
    }
    else if(tcb->status == TSKST_WAIT && tcb->waitfct == WAITFCT_SLP) {
        tqueue_remove_entry(&wait_queue, tcb);
        tcb->status = TSKST_READY;
        tcb->waitfct = WAITFCT_NON;
        tcb->waittim = 0;
        tqueue_add_entry(&ready_queue[tcb->tskpri-1], tcb);
        scheduler();
    }
    else {
        tcb->wupcnt++;
    }
    END_CRITICAL_SECTION
    return ercd;
}