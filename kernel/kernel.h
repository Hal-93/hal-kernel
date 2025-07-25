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

typedef int32_t TMO;
#define TMO_POL 0
#define TMO_FEVR (-1)

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
    WAITFCT_DLY,
    WAITFCT_SLP,
    WAITFCT_FLG,
    WAITFCT_BSEM,
    WAITFCT_SEM
} WAIT_FCT;

typedef struct {
	uint32_t	iflgptn;
} T_CFLG;

#define	TWF_ANDW	0
#define	TWF_ORW		1

extern ID h_cre_flg(T_CFLG *pk_cflg);
extern ER h_set_flg(ID flgid, uint32_t setptn);
extern ER h_clr_flg(ID flgid, uint32_t clrptn);
extern ER h_wai_flg(ID flgid, uint32_t waiptn, uint32_t wfmode, uint32_t *p_flgptn, TMO tmout);

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
    uint32_t wupcnt;
    ER *ercd;

    ID wobjid;
    uint32_t waiptn;
    uint32_t wfmode;
    uint32_t *p_flgptn;

    uint32_t		semcnt;
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

extern TCB		tcb_tbl[MAX_TASK];
extern TCB		*ready_queue[MAX_PRI];
extern TCB		*wait_queue;
extern TCB		*cur_task;
extern TCB		*next_task;

#define TIMER_PERIOD 10

extern void disable_sig(void);
extern void enable_sig(void);
#define BEGIN_CRITICAL_SECTION disable_sig();
#define END_CRITICAL_SECTION enable_sig();

#define MAX_FLG 5

typedef enum {
    OBJST_NON,
    OBJST_EXIST
} OBJ_ST;

typedef struct st_flbcb {
    OBJ_ST status;
    uint32_t flgptn;
} FLGCB;

typedef struct {
    uint32_t bsem;
} T_CBSEM;

typedef struct {
	uint32_t	isemcnt;
	uint32_t	maxsem;
} T_CSEM;

#define MAX_BSEM 5
#define MAX_SEM  5

typedef struct st_bsemcb {
    OBJ_ST status;
    uint32_t bsem;
} BSEMCB;

typedef struct st_semcb {
	OBJ_ST			status;		// オブジェクト状態
	uint32_t		semcnt;		// セマフォ値
	uint32_t		maxsem;		// 最大セマフォ値
} SEMCB;




/* kernel API*/
extern ID h_cre_tsk(T_CTSK *pk_ctsk);
extern ER h_dly_tsk(RELTIM h_dly_tsk);
extern ER h_slp_tsk(TMO tmout);
extern ER h_wup_tsk(ID tskid);
extern ID h_cre_bsem(T_CBSEM *pk_cbsem);
extern ER h_wai_bsem(ID bsemid, TMO tmout);
extern ER h_sig_bsem(ID bsemid);
extern ID h_cre_sem( T_CSEM *pk_csem );
extern ER h_wai_sem( ID semid, uint32_t cnt, TMO tmout );
extern ER h_sig_sem( ID semid, uint32_t cnt);
extern void scheduler(void);

/* ユーザーメイン */
extern void usermain(void);