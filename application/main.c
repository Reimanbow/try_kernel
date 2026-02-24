/**
 * @file main.c
 */
#include <trykernel.h>

// タスク管理データ
UINT    cur_task    = 0;    // 実行中のタスクのID番号(current_task)
UINT    next_task   = 0;    // 次に実行するタスクのID番号(next_task)

#define MAX_FNC_ID  2       // タスクの数
void *ctx_tbl[MAX_FNC_ID];  // 保存された実行コンテキストへのポインタ

// タスクのスタック
#define STACK_SIZE  1024
UW  stack_1[STACK_SIZE/sizeof(UW)];
UW  stack_2[STACK_SIZE/sizeof(UW)];

/**
 * @brief 時間待ち関数
 */
static void delay_ms(UINT ms) {
    UINT cnt = ms / TIMER_PERIOD;

    while (cnt) {
        // TIMER_PERIOD経過するとフラグがセット
        if ((in_w(SYST_CSR) & SYST_CSR_COUNTFLAG) != 0) {
            cnt--;
        }
    }
}

/**
 * @brief タスク1
 */
void task_1(void) {
    while (1) {
        out_w(GPIO_OUT_SET, (1<<25));   // LEDの点灯
        delay_ms(500);                  // 0.5秒待ち

        next_task = 2;  // 次に実行するタスクを設定
        dispatch();     // ディスパッチャの実行
    }
}

/**
 * @brief タスク2
 */
void task_2(void) {
    while (1) {
        out_w(GPIO_OUT_CLR, (1<<25));   // LEDの消灯
        delay_ms(500);                  // 0.5秒待ち

        next_task = 1;  // 次に実行するタスクを設定
        dispatch();     // ディスパッチャの実行
    }
}

/**
 * @brief メイン関数
 */
int main(void) {
    ctx_tbl[0] = make_context(stack_1, sizeof(stack_1), task_1);
    ctx_tbl[1] = make_context(stack_2, sizeof(stack_2), task_2);

    next_task = 1;  // ディスパッチにより実行する関数
    dispatch();     // ディスパッチャの実行

    while (1);
    return 0;
}