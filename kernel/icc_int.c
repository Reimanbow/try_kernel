/**
 * @file icc_int.c
 * @brief CPUコア間割り込み
 */
#include <trykernel.h>
#include <knldef.h>

FP  icc_int_tbl[CPU_CORE_NUM][16];      // CPUコア間割り込みハンドラテーブル

// CPUコア間割り込みハンドラ
void icc_int_hdr(UINT intno) {
    UW  data;
    FP  fp;

    while (in_w(FIFO_ST) & FIFO_ST_VLD) {       // インターコアFIFOにデータのある間
        data = in_w(FIFO_RD);                   // インターコアFIFOからデータを取得
        fp = icc_int_tbl[CPU_CORE][data>>24];   // CPUコア間割り込みハンドラのアドレス取得
        if (fp) {
            (*fp)(data);                        // CPUコア間割り込みハンドラの実行
        }
    }
    out_w(FIFO_ST, 0);                          // インターコアFIFOの状態クリア
}

// CPUコア間割り込みハンドラの登録関数
ER icc_def_int(UINT intno, FP inthdr) {
    UINT    intsts;

    if (intno >= 16) return E_ID;

    DI(intsts);                             // 割り込み禁止
    icc_int_tbl[CPU_CORE][intno] = inthdr;  // CPUコア間割り込みハンドラを登録
    EI(intsts);                             // 割り込み許可
    return E_OK;
}

// CPUコア割り込み生成関数
ER icc_ras_int(UW code) {
    UINT    intsts;
    ER      err = E_OK;

    DI(intsts);                         // 割り込み禁止
    if (in_w(FIFO_ST) & FIFO_ST_RDY) {  // インターコアFIFOが書き込み可能か?
        out_w(FIFO_WR, code);           // インターコアFIFOにデータを書き込む
    } else {
        err = E_QOVR;                   // 書き込み不可ならエラー
    }
    EI(intsts);                         // 割り込み許可

    return err;
}

// CPUコア間割り込み初期化
void init_icc_int(void) {
    _UW     dummy;
    T_DINT  dint = {TA_HLNG, icc_int_hdr};

    dummy = in_w(FIFO_ST);                          // インターコアFIFOの状態をクリア
    (dummy);

    tk_def_int(IRQ_SIOPR0 + CPU_CORE, &dint);       // IRQ_SIORP*割り込みハンドラの登録
    EnableInt(IRQ_SIOPR0 + CPU_CORE, INTLEVEL_0);   // IRQ_SIOPR*割り込みの有効化
}