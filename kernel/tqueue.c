#include <stdio.h>
#include <setjmp.h>
#include "kernel.h"

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

void tqueue_remove__top(TCB **queue)
{
    TCB *top;

    if (*queue == NULL) return;

    top = *queue;
    *queue = top->next;
    if (*queue != NULL) {
        (*queue)->pre = top->pre;
    }
}

void tqueue_remove_entry(TCB **queue, TCB *tcb)
{
    if(*queue == tcb) { 
        tqueue_remove_top(queue);
    } else {               
        (tcb->pre)->next = tcb->next;
        if(tcb->next != NULL) {
            (tcb->next)->pre = tcb->pre;
        } else {
            (*queue)->pre = tcb->pre;
        }
    }
}