/**
 * @file inittsk.c
 * @brief Try Kernel 初期タスク
 */
#include <trykernel.h>
#include <knldef.h>

// 初期タスクの実行プログラム
void initsk(INT stacd, void *exinf);

// 初期タスクのスタック
UW tskstk_ini[256/sizeof(UW)];

// 初期タスクのID番号
ID tskid_ini;

T_CTSK  ctsk_ini = {
    .tskatr     = TA_HLNG | TA_RNG0 | TA_USERBUF,   // タスク属性
    .task       = initsk,                           // タスクの実行関数
    .itskpri    = 1,                                // タスクの優先度
    .stksz      = sizeof(tskstk_ini),               // スタックサイズ
    .bufptr     = tskstk_ini,                       // スタックへのポインタ
};

void initsk(INT stacd, void *exinf) {
    // シリアル通信の初期化
    tm_com_init();
    tm_putstring("Start Try Kernel\n");

    usermain();
    tk_ext_tsk();
}

int main(void) {
    // 初期タスク生成
    tskid_ini = tk_cre_tsk(&ctsk_ini);
    // 初期タスク実行
    tk_sta_tsk(tskid_ini, 0);

    while (1);
}