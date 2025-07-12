#include "kernel.h"
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>

TCB *cur_task;
TCB *ready_queue[MAX_PRI];
TCB *wait_queue;

const struct itimerval systim_run = {
    {0, TIMER_PERIOD * 1000},
    {0, TIMER_PERIOD * 1000}
};

void hdr_systim(int signo)
{
    TCB *tcb, *next;

    BEGIN_CRITICAL_SECTION 

    tcb = wait_queue;
    while(tcb!=NULL) {
        next = (TCB*)(tcb->next);

        if(tcb->waittim > TIMER_PERIOD) {
            tcb->waittim -= TIMER_PERIOD;
        }
        else {
            tqueue_remove_entry(&wait_queue, tcb);
            cur_task->status = TSKST_READY;
            cur_task->waitfct = WAITFCT_NON;
            cur_task->waittim = 0;
            tqueue_add_entry(&ready_queue[tcb->tskpri-1], tcb);
        }
        tcb = next;
    }
    scheduler();

    END_CRITICAL_SECTION
}


void start_systim(void){
    signal(SIGALRM, hdr_systim);
    settimer(ITIMER_REAL, &systim_run, NULL);
}

void disable_sig(void)
{
    sigset_t set;

    sigfillset(&set);
    sigpromask(SIG_SETMASK, &set, NULL);
}

void enable_sig(void)
{
    sigset_t set;

    sigemptyset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);
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

TCB		*cur_task;
TCB		*next_task;
int		in_scheduler;

void scheduler(void)
{
	int		i;

	/* 実行するタスクの検索 */
	next_task = NULL;
	for(i = 0; i < MAX_PRI; i++) {
		if(ready_queue[i] != NULL) {
			next_task = ready_queue[i];
			break;
		}
	}

	if(!in_scheduler) {
		in_scheduler = 1;
		while(next_task == NULL) {	// アイドルループ
			sigset_t	set;
			sigemptyset(&set);
			sigsuspend(&set);
		}
		in_scheduler = 0;

		/* 次に実行するタスクを実行 */
		if(cur_task == NULL) {
			cur_task = next_task;
			longjmp(next_task->context, 1);
		} else if(next_task != cur_task) {
			TCB *temp = cur_task;
			cur_task = next_task;
			dispatch(temp->context, cur_task->context);
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

int main(void) {
    make_stack(-1);
    start_systim();
    h_cre_tsk(&ini_ctsk);
    scheduler();
    return 0;
}