#include <stdio.h>
#include <unistd.h>
#include <setjmp.h>

#include "kernel.h"

MBFCB   mbfcb_tbl[MAX_MBF];

static void init_rbuf(RINGBUF *rbuf, uint8_t *bufptr, uint32_t bufsz)
{
    rbuf->bufptr = bufptr;
    rbuf->limit =  bufptr + bufsz;
    rbuf->buf_rp = bufptr;
    rbuf->buf_wp = bufptr;
}

static void store_msg(RINGBUF *rbuf, uint8_t *msg, uint8_t msgsz)
{
    uint8_t     *src, *dst;
    uint32_t    i;

    src = msg;
    dst = rbuf->buf_wp;
    *dst++ = msgsz;

    for( i = msgsz; i > 0; i--) {
        *dst++ = *src++;
        if(dst >= rbuf->limit) dst = rbuf->bufptr;
    }
    rbuf->buf_wp = dst;
}

static uint32_t retrieve_msg( RINGBUF *rbuf,  uint8_t *msg)
{
    uint8_t     *src, *dst;
    uint32_t    msgsz;
    uint32_t    i;

    src = rbuf->buf_rp;
    dst = msg;
    msgsz =  (uint32_t)*src++;

    for( i = msgsz; i > 0 ; i--) {
        *dst++ = *src++;
        if(src >= rbuf->limit) src = rbuf->bufptr;
    }
    rbuf->buf_rp = src;

    return msgsz;
}


ID h_cre_mbf( T_CMBF *pk_cmbf)
{
	uint32_t        i;
	ID				rtncd;

	BEGIN_CRITICAL_SECTION

	for(i = 0; i < MAX_MBF; i++) {
		if(mbfcb_tbl[i].status == OBJST_NON) {
			mbfcb_tbl[i].status = OBJST_EXIST;
			mbfcb_tbl[i].bufsz = pk_cmbf->bufsz;
			mbfcb_tbl[i].maxsz = pk_cmbf->maxsz;
			mbfcb_tbl[i].freesz = pk_cmbf->bufsz;

            init_rbuf(&mbfcb_tbl[i].rbuf, pk_cmbf->bufptr, pk_cmbf->bufsz);
			break;
		}
	}
	if(i < MAX_MBF) rtncd = i + 1;
	else rtncd = (ID)E_LIMIT;

	END_CRITICAL_SECTION
	return rtncd;
}

ER h_snd_mbf( ID mbfid, const void *msg, uint32_t msgsz, TMO tmout)
{
	MBFCB   *mbfcb;
	ER      ercd = E_OK;

	if(!(mbfid >0 && mbfid < MAX_MBF)) return E_ID;
    if(msgsz >= 256) return E_PAR; 

	BEGIN_CRITICAL_SECTION

	mbfcb = &mbfcb_tbl[mbfid - 1];
	if(mbfcb->status == OBJST_EXIST) {
        if(mbfcb->freesz > msgsz + sizeof(uint8_t)) { 
            store_msg(&mbfcb->rbuf, (uint8_t*)msg, (uint8_t)msgsz);
            mbfcb->freesz -= msgsz + sizeof(uint8_t);

            for(TCB *tcb = wait_queue; tcb != NULL; tcb = tcb->next) {
                if(tcb->waitfct == WAITFCT_MBFR && tcb->wobjid == mbfid) {
                    uint32_t rcvsz;
                    rcvsz = retrieve_msg(&mbfcb->rbuf, (uint8_t*)tcb->msg);
                    mbfcb->freesz += rcvsz + sizeof(uint8_t);

                    tqueue_remove_entry(&wait_queue, tcb);

                    tcb->status     = TSKST_READY;
                    tcb->waitfct    = WAITFCT_NON;
                    tcb->waittim    = 0;
                    *tcb->ercd      = rcvsz;

                    tqueue_add_entry(&ready_queue[tcb->tskpri -1 ], tcb);
                    break;
                }
            }
        } else {
			tqueue_remove_entry(&ready_queue[cur_task->tskpri -1 ], cur_task);

			cur_task->status    = TSKST_WAIT;
			cur_task->waitfct   = WAITFCT_MBFS;
			cur_task->waittim   = tmout;
			cur_task->ercd      = &ercd;
			cur_task->wobjid    = mbfid;
            cur_task->msgsz     = msgsz;
            cur_task->msg       = msg;

			tqueue_add_entry(&wait_queue, cur_task);
        }
        scheduler();
	} else {
		ercd = E_NOEXS;
    }

	END_CRITICAL_SECTION
    return ercd;
}


uint32_t h_rcv_mbf( ID mbfid, void *msg, TMO tmout)
{
	MBFCB       *mbfcb;
	uint32_t    rtncd = 0;

	if(!(mbfid >0 && mbfid < MAX_MBF)) return E_ID;

	BEGIN_CRITICAL_SECTION

	mbfcb = &mbfcb_tbl[mbfid - 1];
	if(mbfcb->status == OBJST_EXIST) {
        if(mbfcb->freesz != mbfcb->bufsz) {
            rtncd = retrieve_msg(&mbfcb->rbuf, (uint8_t*)msg);
            mbfcb->freesz += rtncd + sizeof(uint8_t);

            for(TCB *tcb = wait_queue; tcb != NULL; tcb = tcb->next) {
                if(tcb->waitfct == WAITFCT_MBFS && tcb->wobjid == mbfid) {
                    if(mbfcb->freesz > tcb->msgsz) {
                        store_msg(&mbfcb->rbuf, (uint8_t*)tcb->msg, tcb->msgsz);
                        mbfcb->freesz -= tcb->msgsz + sizeof(uint8_t);

                        tqueue_remove_entry(&wait_queue, tcb);

                        tcb->status    = TSKST_READY;
                        tcb->waitfct   = WAITFCT_NON;
                        tcb->waittim   = 0;

                        tqueue_add_entry(&ready_queue[tcb->tskpri -1 ], tcb);
                    }
                    break;
                }
            }
        } else {
			tqueue_remove_entry(&ready_queue[cur_task->tskpri -1 ], cur_task);

			cur_task->status    = TSKST_WAIT;
			cur_task->waitfct   = WAITFCT_MBFR;
			cur_task->waittim   = tmout;
			cur_task->ercd      = (ER*)&rtncd;
			cur_task->wobjid    = mbfid;
            cur_task->msg       = msg;

			tqueue_add_entry(&wait_queue, cur_task);
        }
        scheduler();
	} else {
		rtncd = (uint32_t)E_NOEXS;
    }

	END_CRITICAL_SECTION
    return rtncd;
}
