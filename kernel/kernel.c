#include "kernel.h"
#include <stdio.h>
#include <setjmp.h>

TCB *cur_task;
TCB *ready_queue[MAX_PRI];

void dispatch(jmp_buf from, jmp_buf to) {
    if (setjmp(from) == 0) {
        longjmp(to, 1);
    }
}

void ini_task(void);
T_CTSK ini_ctsk = {
    .task = ini_task
};

void ini_task(void) {
    usermain();
}

void init_context(int n);

void make_stack(int n)
{
    char stack[TASK_STACK_SZ];
    init_context(++n);
}

void init_context(int n)
{
    if(n < MAX_TASK) {
        if(setjmp(tcb_tbl[n].context) == 0) {
            make_stack(n);
        } else {
            tcb_tbl[n].task();
        }
    }
}

void tqueue_add_entry(TCB **queue, TCB *tcb) {
    TCB *queue_end;

    if(*queue == NULL) {
        *queue = tcb;
        tcb->pre = tcb;
    }
    else {
        queue_end = (*queue)->pre;
        queue_end->next = tcb;
        tcb->pre = queue_end;
        (*queue)->pre = tcb;
    }
    tcb->next = NULL;
}


ID h_cre_tsk(T_CTSK *pk_ctsk)
{
    uint32_t i;

    for(i = 0; i < MAX_TASK; i++) {
        if (tcb_tbl[i].status == TSKST_NON) {
            tcb_tbl[i].tskid = (ID)(i+1);
            tcb_tbl[i].status = TSKST_READY;
            tcb_tbl[i].task = pk_ctsk->task;
            tcb_tbl[i].tskpri = pk_ctsk->tskpri;
            break;
        }
    }
    if(i < MAX_TASK) {
        PRI priority = tcb_tbl[i].priority;
        if (priority >= MAX_PRI) {
            priority = MAX_PRI - 1;
        }
        tqueue_add_entry(&ready_queue[priority], &tcb_tbl[i]);
        return (ID)(i+1);
    } 
    else {
        return (ID)E_LIMIT;
    }
}

void scheduler(void)
{
    TCB *next_tcb = NULL;
    int i;

    for (i = 0; i < MAX_PRI; i++) {
        if (ready_queue[i] != NULL) {
            next_tcb = ready_queue[i];
            break;
        }
    }
    if (next_tcb == NULL) {
        while(1);
    }

    if (cur_task != next_tcb) {
        TCB *prev_task = cur_task;
        cur_task = next_tcb;
        
        if (prev_task != NULL) {
            dispatch(prev_task->context, cur_task->context);
        } else {
            longjmp(cur_task->context, 1);
        }
    }
}

int main(void) {
    make_stack(-1);
    h_cre_tsk(&ini_ctsk);
    scheduler();
    return 0;
}