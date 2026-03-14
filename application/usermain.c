/**
 * @file usermain.c
 */
#include <trykernel.h>

#define GPIO_TRIG               12      // TRIG信号 P12
#define GPIO_ECHO               13      // ECHO信号 P13
#define GPIO_LED                14      // LED P14

// センサ制御タスクの情報
ID  tskid_sns;                          // タスクID番号
#define STKSZ_SNS   1024                // スタックサイズ
UW  tskstk_sns[STKSZ_SNS/sizeof(UW)];   // スタック領域
void tsk_sns(INT stacd, void *exinf);   // タスク実行関数

// センサ制御タスク生成情報
T_CTSK ctsk_sns = {
    .tskatr     = TA_HLNG | TA_RNG3 | TA_USERBUF,
    .task       = tsk_sns,
    .stksz      = STKSZ_SNS,
    .itskpri    = 5,
    .bufptr     = tskstk_sns,
};

UW      tim1, tim2;     // 時刻計測用グローバル変数

// ECHO信号割り込みハンドラ
#define INTNO_ECHO_EDGE_HIGH    21  // ECHO割り込みエッジHIGH
#define INTNO_ECHO_EDGE_LOW     20  // ECHO割り込みエッジLOW

void echo_inthdr(UW intno) {
    UW  val;

    for (int i = 0; i < 4; i++) {
        val = in_w(INTR(i));                                    // 割り込み要因の取得
        if (i == 1) {
            if (val & (1 << INTNO_ECHO_EDGE_HIGH)) {            // エッジHIGH検出
                tim1 = in_w(TIMER_TIMELR);                      // 開始時刻取得
            } else if (val & (1 << INTNO_ECHO_EDGE_LOW)) {     // エッジLOW検出
                tim2 = in_w(TIMER_TIMELR);                      // 終了時刻取得
                tk_wup_tsk(tskid_sns);                          // タスクへの通知
            }
        }
        out_w(INTR(i), val);                                    // 割り込み要因のクリア
    }
}

// 10マイクロ秒待ち
static void wait_10micro(void) {
    UW      t0, t;
    t0 = in_w(TIMER_TIMELR);
    do {
        t = in_w(TIMER_TIMELR);
    } while (t - t0 <= 10);
}

// 障害物に対するアクションの実行
static void do_action(UW dis) {
    if (dis < 150) {
        // 障害物が近ければLED点灯
        out_w(GPIO_OUT_SET, 1<<GPIO_LED);
    } else {
        out_w(GPIO_OUT_CLR, 1<<GPIO_LED);
    }
}

// センサ制御タスクの実行関数
void tsk_sns(INT stacd, void *exinf) {
    // 割り込み登録情報
    T_DINT  dint = {
        .intatr = TA_HLNG,
        .inthdr = echo_inthdr,
    };

    UW      tim_val;    // 計測した時間
    INT     dis;        // 障害物の距離
    ER      err;        // エラーコード

    // GPIOの設定
    gpio_enable_output(GPIO_LED, 0);        // LED出力設定 初期値LOW
    gpio_enable_output(GPIO_TRIG, 0);       // TRIG信号出力設定 初期値LOW
    gpio_enable_input(GPIO_ECHO);           // ECHO信号入力設定

    gpio_set_intmode(GPIO_ECHO, INTE_MODE_EDGE_LOW | INTE_MODE_EDGE_HIGH);  // ECHO信号割り込みの設定
    tk_def_int(IRQ_BANK0, &dint);                                           // ECHO信号割り込みハンドラの登録

    out_w(WDT_TICK, WDT_TICK_ENABLE | 12);                                  // Timer初期化(1MHz)

    while (1) {
        for (int i = 0; i < 3; i++) {
            out_w(INTR(i), in_w(INTR(i)));  // 割り込み要因の消去
        }
        ClearInt(IRQ_BANK0);
        EnableInt(IRQ_BANK0, 2);            // ECHO信号割り込みを有効(優先度2)

        // TRIG信号の生成
        out_w(GPIO_OUT_SET, 1<<GPIO_TRIG);
        wait_10micro();
        out_w(GPIO_OUT_CLR, 1<<GPIO_TRIG);

        err = tk_slp_tsk(1000);             // 割り込みハンドラからの起床待ち
        DisableInt(IRQ_BANK0);              // ECHO信号割り込みを無効

        if (err >= E_OK) {
            tim_val = tim2 - tim1;
            dis = (tim_val/2) * 340 / 1000; // 障害物の距離を計算
            do_action(dis);                 // 障害物に対するアクションの実行
        }
        tk_dly_tsk(500);
    }
    tk_slp_tsk(TMO_FEVR);
}

int usermain(void) {
    tm_putstring("Start usermain\n");

    tskid_sns = tk_cre_tsk(&ctsk_sns);
    tk_sta_tsk(tskid_sns, 0);

    tk_slp_tsk(TMO_FEVR);
    return 0;
}