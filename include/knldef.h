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
 * @brief OSメイン関数
 */
extern int main(void);

#endif /* KNLDEF_H */