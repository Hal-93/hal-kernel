#include "kernel.h"
#include <stdio.h>
#include <setjmp.h>

TCB *cur_task;

void scheduler(void) {
    uint32_t next;

    if(cur_task != NULL) {
        next = cur_task->tskid;
        if(next >= MAX_TASK) next = 0;
    }
    else {
        next = 0;
    }
    while(tcb_tbl[next].status != TSKST_READY) {
        if(++next >= MAX_TASK) next = 0;
    }
    if(cur_task == NULL) {
        cur_task = &tcb_tbl[next];
        longjmp(&tcb_tbl[next].context, 1);
    }
    else if(next != cur_task->tskid-1) {
        TCB *temp = cur_task;
        cur_task = &tcb_tbl[next];
        dispatch(temp->context, cur_task->context);
    }
}

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


ID h_cre_tsk(T_CTSK *pk_ctsk)
{
    uint32_t i;

    for(i = 0; i < MAX_TASK; i++) {
        if (tcb_tbl[i].status == TSKST_NON) {
            tcb_tbl[i].tskid = (ID)(i+1);
            tcb_tbl[i].status = TSKST_READY;
            tcb_tbl[i].task = pk_ctsk->task;
            break;
        }
    }
    return (i < MAX_TASK)? (ID)(i+1): (ID)E_LIMIT;
}

int main(void) {
    make_stack(-1);
    h_cre_tsk(&ini_ctsk);
    scheduler();
    return 0;
}