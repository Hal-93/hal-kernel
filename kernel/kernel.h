#include <stdint.h>
#include <setjmp.h>

#define E_OK        (0)

#define E_SYS       (-5)
#define E_NOCOP     (-6)
#define E_NOSPT     (-9)
#define E_RSFN      (-10)
#define E_RSATR     (-11)
#define E_PAR       (-17)
#define E_ID        (-18)
#define E_CTX       (-25)
#define E_MACV      (-26)
#define E_OACV      (-27)
#define E_ILUSE     (-28)
#define E_NOMEM     (-33)
#define E_LIMIT     (-34)
#define E_OBJ       (-41)
#define E_NOEXS     (-42)
#define E_QOVR      (-43)
#define E_RLWAI     (-49)
#define E_TMOUT     (-50)
#define E_DLT       (-51)
#define E_DISWAI    (-52)
#define E_IO        (-57)

typedef uint32_t    ID;
typedef uint32_t PRI;
typedef int32_t ER;
typedef int32_t RELTIM;

typedef void (*FP)(void);

#define MAX_TASK 10
#define MAX_PRI 10
#define TASK_STACK_SZ 1024*4

typedef enum {
    TSKST_NON,
    TSKST_DORMANT,
    TSKST_READY,
    TSKST_WAIT
} TASK_ST;

typedef enum {
    WAITFCT_NON,
    WAITFCT_DLY
} WAIT_FCT;

typedef struct st_tcb{
    struct st_tcb *pre;
    struct st_tcb *next;

    int priority;
    ID tskid;
    PRI tskpri;
    TASK_ST status;
    jmp_buf context;
    FP task;

    WAIT_FCT waitfct;
    RELTIM waittim;
} TCB;
TCB tcb_tbl[MAX_TASK];

/* レディキュー操作 */
void tqueue_add_entry(TCB **queue, TCB *tcb);
void tqueue_remove_top(TCB **queue);
void tqueue_remove_entry(TCB **queue, TCB *tcb);

typedef struct {
    FP task;
    PRI tskpri;
} T_CTSK;

#define TIMER_PERIOD 10

extern void disable_sig(void);
extern void enable_sig(void);
#define BEGIN_CRITICAL_SECTION disable_sig();
#define END_CRITICAL_SECTION enable_sig();

/* kernel API*/
extern ID h_cre_tsk(T_CTSK *pk_ctsk);
extern ER h_dly_tsk(RELTIM h_dly_tsk);
extern void scheduler(void);

/* ユーザーメイン */
extern void usermain(void);