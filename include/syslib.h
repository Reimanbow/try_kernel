/**
 * @file syslib.h
 * @brief Try Kernel 共通ライブラリ関数定義
 */
#ifndef SYSLIB_H
#define SYSLIB_H

/**
 * @brief 32bitレジスタからの入力
 */
static inline UW in_w(UW adr) {
    return *(_UW*)adr;
}

/**
 * @brief 32bitレジスタへの出力
 */
static inline void out_w(UW adr, UW data) {
    *(_UW*)adr = data;
}

/**
 * @brief 32bitレジスタへの出力(ビットクリア)
 */
#define OP_CLR  0x3000
static inline void clr_w(UW adr, UW data) {
    *(_UW*)(adr + OP_CLR) = data;
}

/**
 * @brief 32bitレジスタへの出力(ビットセット)
 */
#define OP_SET  0x2000
static inline void set_w(UW adr, UW data) {
    *(_UW*)(adr + OP_SET) = data;
}

/**
 * @brief 32bitレジスタへの出力(ビット排他的論理和)
 */
#define OP_XOR  0x1000
static inline void xset_w(UW adr, UW data) {
    *(_UW*)(adr + OP_XOR) = data;
}

/* PRIMASKレジスタ制御インライン関数 */
/**
 * @brief PRIMASKレジスタに値をセットする
 */
static inline void set_primask(INT pm) {
    __asm__ volatile("msr primask, %0":: "r"(pm));
}

/**
 * @brief PRIMASKレジスタの値を取得する
 */
static inline UW get_primask(void) {
    UW pm;
    __asm__ volatile("mrs %0, primask": "=r"(pm));
    return pm;
}

/* 割り込み禁止マクロ */
#define DI(intsts)  (intsts=get_primask(), set_primask(1))

/* 割り込み許可マクロ */
#define EI(intsts)  (set_primask(intsts))

/* デバッグ用シリアル通信 */

/**
 * @brief UARTの初期化
 */
void tm_com_init(void);

/**
 * @brief デバッグ用UART出力
 */
UINT tm_putstring(char* str);

#endif /* SYSLIB_H */