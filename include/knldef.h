/**
 * @file knldef.h
 * Try Kernel カーネル内部共通定義
 */
#ifndef KNLDEF_H
#define KNLDEF_H

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

/**
 * @brief OSメイン関数
 */
extern int main(void);

#endif /* KNLDEF_H */