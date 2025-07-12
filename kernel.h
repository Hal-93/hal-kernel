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

#define MAX_TASK 10
#define TASK_STACK_SZ 2048

typedef uint32_t    ID;
typedef void (*FP)(void);

typedef enum {
    TSKST_NON,
    TSKST_DORMANT,
    TSKST_READY,
} TASK_ST;

typedef struct {
    ID tskid;
    TASK_ST status;
    jmp_buf context;
    FP task;
} TCB;

TCB tcb_tbl[MAX_TASK];

typedef struct {
    FP task;
} T_CTSK;


extern void dispatch(jmp_buf from, jmp_buf to);
extern void scheduler();
extern ID h_cre_tsk(T_CTSK *pk_ctsk);
extern void usermain(void);