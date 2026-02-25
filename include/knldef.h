/**
 * @file knldef.h
 * Try Kernel カーネル内部共通定義
 */
#ifndef KNLDEF_H
#define KNLDEF_H

// タスクの状態
typedef enum {
    TS_NONEXIST = 0,    // 未登録
    TS_READY    = 1,    // 実行状態 or 実行可能状態
    TS_WAIT     = 2,    // 待ち状態
    TS_DORMANT  = 8     // 休止状態
} TSTAT;

// TCB(Task Control Block)定義
typedef struct st_tcb {
void    *context;       // コンテキスト情報へのポインタ

    // キュー用ポインタ
    struct st_tcb   *pre;   // 1つ前の要素
    struct st_tcb   *next;  // 1つ後の要素

    TSTAT   state;          // タスク状態
    FP      tskadr;         // 実行開始アドレス
    PRI     itskpri;        // 実行優先度
    void    *stkadr;        // スタックのアドレス
    SZ      stksz;          // スタックのサイズ
} TCB;

extern TCB  tcb_tbl[];      // TCBテーブル
extern TCB  *ready_queue[]; // タスクの実行待ち行列(優先度毎)
extern TCB  *cur_task;      // 実行中のタスク
extern TCB  *sche_task;     // 次に実行するタスク

/* グローバル関数 */
/**
 * @brief リセットハンドラ
 */
extern void Reset_Handler(void);

/**
 * @brief ディスパッチャ
 */
extern void dispatch_entry(void);

/* ディスパッチャの呼び出し */
#define SCB_ICSR        0xE000ED04  // 割込み制御ステートレジスタのアドレス
#define ICSR_PENDSVSET  (1<<28)     // PendSV set-pending ビット
/**
 * @brief PendSV例外を発生させる
 */
static inline void dispatch(void) {
    out_w(SCB_ICSR, ICSR_PENDSVSET);
}

/**
 * @brief スケジューラ
 */
extern void scheduler(void);

/**
 * @brief タスクコンテキストの作成
 */
extern void *make_context( UW *sp, UINT ssize, void (*fp)() );

// タスクの待ち行列操作関数
/**
 * @brief エントリ追加関数
 * 
 * タスクの待ち行列の最後に指定したTCBを追加する
 */
extern void tqueue_add_entry(TCB **queue, TCB *tcb);

/**
 * @brief 先頭エントリ削除関数
 * 
 * タスクの待ち行列の先頭のTCBを削除する
 */
extern void tqueue_remove_top(TCB **queue);

/**
 * @brief エントリ削除関数
 * 
 * タスクの待ち行列から指定したTCBを削除する
 */
extern void tqueue_remove_entry(TCB **queue, TCB *tcb);

/**
 * @brief OSメイン関数
 */
extern int main(void);

/**
 * @brief ユーザメイン関数
 */
extern int usermain(void);

#endif /* KNLDEF_H */