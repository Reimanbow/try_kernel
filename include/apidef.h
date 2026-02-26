/**
 * @file apidef.h
 * @brief Try Kernel API定義
 */
#ifndef APIDEF_H
#define APIDEF_H

// タイムアウト時間
#define TMO_POL         (0)     // タイムアウト時間 0
#define TMO_FEVR        (-1)    // 無限待ち

// タスク生成情報
typedef struct {
    ATR     tskatr;     // タスク属性
    FP      task;       // タスク起動アドレス
    PRI     itskpri;    // タスク優先度
    SZ      stksz;      // スタックサイズ
    void    *bufptr;    // スタックのバッファポインタ
} T_CTSK;

// タスク属性
#define TA_HLNG         0x0000001
#define TA_USERBUF      0x0000020
#define TA_RNG0         0x0000000
#define TA_RNG1         0x0000100
#define TA_RNG2         0x0000200
#define TA_RNG3         0x0000300

// タスク管理API
/**
 * @brief タスクの生成
 */
ID tk_cre_tsk(const T_CTSK *pk_ctsk);

/**
 * @brief タスクの実行
 */
ER tk_sta_tsk(ID tskid, INT stacd);

/**
 * @brief タスクの終了
 */
void tk_ext_tsk(void);

// タスク付属同期API
/**
 * @brief APIを発行したタスクを、引数で指定した時間、時間待ちの状態にする
 */
ER tk_dly_tsk(RELTIM dlytim);

/**
 * @brief 他のタスクからtk_wup_tskにより起床されるまで起床待ち状態とする
 */
ER tk_slp_tsk(TMO tmout);

/**
 * @brief 指定したタスクに対して起床要求を行う
 */
ER tk_wup_tsk(ID tskid);

#endif /* APIDEF_H */