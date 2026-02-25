/**
 * @file task_mange.c
 * @brief Try Kernel タスク管理
 */
#include <trykernel.h>
#include <knldef.h>

// タスク管理ブロック
TCB tcb_tbl[CNF_MAX_TSKID];

/**
 * タスク生成API
 * 指定されたタスク生成情報に基づいてTCBを初期化する
 * 生成したタスクの状態は休止状態(TS_DORMANT)である
 */
ID tk_cre_tsk(const T_CTSK *pk_ctsk) {
    UINT    intsts;
    ID      tskid;
    INT     i;

    // 引数
    if ((pk_ctsk->tskatr & ~TA_RNG3) != (TA_HLNG|TA_USERBUF)) return E_RSATR;
    if (pk_ctsk->itskpri <= 0 || pk_ctsk->itskpri > CNF_MAX_TSKPRI) return E_PAR;
    if (pk_ctsk->stksz == 0) return E_PAR;

    DI(intsts);     // 割り込みを禁止

    // 未使用のTCBを検索
    for (i = 0; i < CNF_MAX_TSKID; i++) {
        if (tcb_tbl[i].state == TS_NONEXIST) break;
    }

    // TCBの初期化
    if (i < CNF_MAX_TSKID) {
        tcb_tbl[i].state    = TS_DORMANT;
        tcb_tbl[i].pre      = NULL;
        tcb_tbl[i].next     = NULL;

        tcb_tbl[i].tskadr   = pk_ctsk->task;
        tcb_tbl[i].itskpri  = pk_ctsk->itskpri;
        tcb_tbl[i].stksz    = pk_ctsk->stksz;
        tcb_tbl[i].stkadr   = pk_ctsk->bufptr;

        tskid = i+1;
    } else {
        // タスクが既に最大数
        tskid = (ID)E_LIMIT;
    }

    EI(intsts);     // 割り込みを許可
    return tskid;
}

/**
 * タスク実行API
 * 指定された休止状態のタスクを実行可能状態に変更し、
 * 実行コンテキスト情報を作成した後、
 * レディキューに登録してスケジューラを実行する
 */
ER tk_sta_tsk(ID tskid, INT stacd) {
    TCB     *tcb;
    UINT    intsts;
    ER      err = E_OK;

    // 引数チェック
    if (tskid <= 0 || tskid > CNF_MAX_TSKID) return E_ID;
    DI(intsts);     // 割り込みを禁止

    tcb = &tcb_tbl[tskid-1];
    if (tcb->state == TS_DORMANT) {
        // タスクを実行できる状態に変更
        tcb->state = TS_READY;
        tcb->context = make_context(tcb->stkadr, tcb->stksz, tcb->tskadr);
        tqueue_add_entry(&ready_queue[tcb->itskpri], tcb);
        scheduler();
    } else {
        // タスクを実行できない(休止状態ではない)
        err = E_OBJ;
    }

    EI(intsts);     // 割り込みを許可
    return err;
}

/**
 * タスク終了API
 * 実行中のタスクを休止状態(TS_DORMANT)に変更し、
 * レディキューから削除してスケジューラを実行する
 */
void tk_ext_tsk(void) {
    UINT    intsts;

    DI(intsts);     // 割り込み禁止

    // タスクを休止状態へ
    cur_task->state     = TS_DORMANT;
    tqueue_remove_top(&ready_queue[cur_task->itskpri]);

    scheduler();    // スケジューラを実行
    EI(intsts);     // 割込みを許可
}