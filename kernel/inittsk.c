/**
 * @file inittsk.c
 * @brief Try Kernel 初期タスク
 */
#include <trykernel.h>
#include <knldef.h>

/* Core0 メイン関数 & 初期タスク */

// 初期タスクの実行プログラム
void initsk_c0(INT stacd, void *exinf);

// 初期タスクのスタック
UW tskstk_ini_c0[256/sizeof(UW)];

// 初期タスクのID番号
ID tskid_ini_c0;

T_CTSK  ctsk_ini_c0 = {
    .tskatr     = TA_HLNG | TA_RNG0 | TA_USERBUF,       // タスク属性
    .task       = initsk_c0,                            // タスクの実行関数
    .itskpri    = 1,                                    // タスクの優先度
    .stksz      = sizeof(tskstk_ini_c0),                // スタックサイズ
    .bufptr     = tskstk_ini_c0,                        // スタックへのポインタ
};

void initsk_c0(INT stacd, void *exinf) {
    usermain_c0();
    tk_ext_tsk();
}

int main_c0(void) {
    init_icc_int();         // CPUコア間割り込み初期化
    // init_icc_msg();         // CPUコア間メッセージ初期化
    tm_com_init();          // デバッグ用シリアル通信の初期化

    // 初期タスク生成
    tskid_ini_c0 = tk_cre_tsk(&ctsk_ini_c0);
    // 初期タスク実行
    tk_sta_tsk(tskid_ini_c0, 0);

    while (1);
}

/* Core1 メイン関数 & 初期タスク */

// 初期タスクの実行プログラム
void initsk_c1(INT stacd, void *exinf);

// 初期タスクのスタック
UW tskstk_ini_c1[256/sizeof(UW)];

// 初期タスクのID番号
ID tskid_ini_c1;

T_CTSK  ctsk_ini_c1 = {
    .tskatr     = TA_HLNG | TA_RNG0 | TA_USERBUF,       // タスク属性
    .task       = initsk_c1,                            // タスクの実行関数
    .itskpri    = 1,                                    // タスクの優先度
    .stksz      = sizeof(tskstk_ini_c1),                // スタックサイズ
    .bufptr     = tskstk_ini_c1,                        // スタックへのポインタ
};

void initsk_c1(INT stacd, void *exinf) {
    usermain_c1();
    tk_ext_tsk();
}

int main_c1(void) {
    init_icc_int();         // CPUコア間割り込み初期化
    // init_icc_msg();         // CPUコア間メッセージ初期化

    // 初期タスク生成
    tskid_ini_c1 = tk_cre_tsk(&ctsk_ini_c1);
    // 初期タスク実行
    tk_sta_tsk(tskid_ini_c1, 0);

    while (1);
}