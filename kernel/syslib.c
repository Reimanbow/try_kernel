/**
 * @file syslib.c
 * @brief Try Kernel システムライブラリ
 */
#include <trykernel.h>
#include <knldef.h>

/**
 * @brief UARTの初期化
 */
void tm_com_init(void) {
    // ボーレート設定
    out_w(UART0_BASE+UARTx_IBRD, 67);
    out_w(UART0_BASE+UARTx_FBRD, 52);
    // データ形式設定
    out_w(UART0_BASE+UARTx_LCR_H, 0x70);
    // 通信イネーブル
    out_w(UART0_BASE+UARTx_CR, UART_CR_RXE|UART_CR_TXE|UART_CR_EN);
}

/**
 * @brief デバッグ用UART出力
 */
UINT tm_putstring(char* str) {
    UINT cnt = 0;

    while (*str) {
        // 送信FIFOの空き待ち
        while ((in_w(UART0_BASE+UARTx_FR) & UART_FR_TXFF) != 0);

        // データ送信
        out_w(UART0_BASE+UARTx_DR, *str++);
        cnt++;
    }
    return cnt;
}