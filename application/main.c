/**
 * @file main.c
 */
#include "typedef.h"
#include "sysdef.h"
#include "syslib.h"

/**
 * @brief 時間待ち関数
 */
static void delay_ms(UINT ms) {
    UINT cnt = ms / TIMER_PERIOD;

    while (cnt) {
        // TIMER_PERIOD経過するとフラグがリセット
        if ((in_w(SYST_CSR) & SYST_CSR_COUNTFLAG) != 0) {
            cnt--;
        }
    }
}

/**
 * @brief メイン関数
 */
int main(void) {
    while (1) {
        out_w(GPIO_OUT_XOR, (1<<25));   // LEDの表示反転
        delay_ms(500);                  // 0.5秒待ち
    }
    return 0;
}