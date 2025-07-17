#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

BSEMCB bsemcb_tbl[MAX_BSEM];

ID h_cre_bsem(T_CBSEM *pk_cbsem) {
    uint32_t i;
    ID rtncd;

    BEGIN_CRITICAL_SECTION

    for(i = 0; i < MAX_BSEM; i++) {
        if(bsemcb_tbl[i].status == OBJST_NON) {
            bsemcb_tbl[i].status = OBJST_EXIST;
            bsemcb_tbl[i].bsem = pk_cbsem->bsem;
            break;
        }
    }
    if(i < MAX_BSEM) rtncd = i + 1;
    else rtncd = (ID)E_LIMIT;

    END_CRITICAL_SECTION
    return rtncd;
}

ER h_wai_bsem(ID bsemid, TMO tmout) {
    BSEMCB *bsemcb;
    ER ercd = E_OK;

    if(!(bsemid > 0 && bsemid < MAX_BSEM)) {
        return E_ID;
    }

    BEGIN_CRITICAL_SECTION

    bsemcb = &bsemcb_tbl[bsemid - 1];
    if(bsemcb->status != OBJST_EXIST) {
        ercd = E_NOEXS;
    }
    else {
        if(bsemcb->bsem == 1) {
            bsemcb->bsem = 0;
        }
        else if(tmout == TMO_POL) {
            ercd = E_TMOUT;
        }
        else {
            tqueue_remove_entry(&ready_queue[cur_task->tskpri - 1], cur_task);

            cur_task->status = TSKST_WAIT;
            cur_task->waitfct = WAITFCT_BSEM;
            cur_task->waittim = tmout;
            cur_task->ercd = &ercd;
            cur_task->wobjid = bsemid;

            tqueue_add_entry(&wait_queue, cur_task);
            scheduler();
        }
    }

    END_CRITICAL_SECTION
    return ercd;
}

ER h_sig_bsem(ID bsemid) {
    BSEMCB *bsemcb;
    ER ercd = E_OK;

    if(!(bsemid > 0 && bsemid < MAX_BSEM)) {
        return E_ID;
    }

    BEGIN_CRITICAL_SECTION

    bsemcb = &bsemcb_tbl[bsemid - 1];
    if(bsemcb->status == OBJST_EXIST) {
        if(bsemcb->bsem == 1) {
            ercd = E_QOVR;
        }
        else {
            bsemcb->bsem = 1;

            for(TCB *tcb = wait_queue; tcb != NULL; tcb->next) {
                if(tcb->waitfct == WAITFCT_BSEM && tcb->wobjid == bsemid) {
                    tqueue_remove_entry(&wait_queue, tcb);

                    tcb->status = TSKST_READY;
                    tcb->waitfct = WAITFCT_NON;
                    tcb->waittim = 0;

                    bsemcb->bsem = 0;

                    tqueue_add_entry(&ready_queue[tcb->tskpri - 1], tcb);
                    scheduler();
                    break;
                }
            }
        }
    }
    else {
        ercd = E_NOEXS;
    }

    END_CRITICAL_SECTION
    return ercd;
}