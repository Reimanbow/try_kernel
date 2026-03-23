/**
 * @file icc_msg.c
 * @brief Try Kernel CPUコア間メッセージ
 */
#include <trykernel.h>
#include <knldef.h>

// CPUコア間メッセージ管理ブロック
ICMCB   icmcb_tbl[CNF_MAX_ICMID];

ID icc_cre_msg(const T_CICM *pk_cicm) {
    ID      icmid;
    UINT    intsts;

    // パラメータチェック
    if ((pk_cicm->bufsz <= 0) || (pk_cicm->maxmsz <= 0) || (pk_cicm->maxmsz >= 256)
            || (pk_cicm->bufsz < pk_cicm->maxmsz + sizeof(UB)) || (pk_cicm->bufptr == NULL)) {
        return E_PAR;
    }

    // クリティカルセクション開始
    DI(intsts);
    icc_loc_spin(SPINLOCK_ICM);
    for (icmid = 0; icmcb_tbl[icmid].state != KS_NONEXIST; icmid++);

    if (icmid < CNF_MAX_FLGID) {
        icmcb_tbl[icmid].state  = KS_EXIST;
        icmcb_tbl[icmid].icmatr = pk_cicm->icmatr;
        icmcb_tbl[icmid].bufsz  = pk_cicm->bufsz;
        icmcb_tbl[icmid].maxmsz = pk_cicm->maxmsz;
        icmcb_tbl[icmid].bufptr = pk_cicm->bufptr;
        icmcb_tbl[icmid].freesz = pk_cicm->bufsz;
        icmcb_tbl[icmid].buf_rp = pk_cicm->bufptr;
        icmcb_tbl[icmid].buf_wp = pk_cicm->bufptr;
    } else {
        icmid = E_LIMIT;
    }
    // クリティカルセクション終了
    icc_unl_spin(SPINLOCK_ICM);
    EI(intsts);
    return icmid;
}

/* バッファへのメッセージの格納 */
static void store_msg( ICMCB *icmcb,  const void *msg, INT msgsz)
{
    UB      *src, *dst, *limit;
    INT     i;

    src = (UB*)msg;
    dst = icmcb->buf_wp;
    limit = icmcb->bufptr + icmcb->bufsz;

    *dst++ = (UB)msgsz;
    for( i = msgsz; i > 0; i--) {
        *dst++ = *src++;
        if(dst >= limit) dst = icmcb->bufptr;
    }
    
    icmcb->freesz -= msgsz + sizeof(UB);
    icmcb->buf_wp = dst;
}

/* バッファからメッセージを取得 */
static INT retrieve_msg( ICMCB *icmcb,  const void *msg)
{
    UB      *src, *dst, *limit;
    INT     msgsz;
    INT     i;

    src = icmcb->buf_rp;
    dst = (UB*)msg;
    limit = icmcb->bufptr + icmcb->bufsz;

    msgsz =  *src++;
    for( i = msgsz; i > 0 ; i--) {
        *dst++ = *src++;
        if(src >= limit) src = icmcb->bufptr;
    }

    icmcb->freesz += msgsz + sizeof(UB);
    icmcb->buf_rp = src;

    return msgsz;
}

// CPUコア間メッセージの送信API
ER icc_snd_msg(ID icmid, const void *msg, INT msgsz, TMO tmout) {
    ICMCB   *icmcb;
    UINT    intsts;
    ER      err     = E_OK;

    if (icmid <= 0 || icmid > CNF_MAX_ICMID) return E_ID;

    // クリティカルセクション開始
    DI(intsts);
    icc_loc_spin(SPINLOCK_ICM);
    icmcb = &icmcb_tbl[--icmid];

    // メッセージ送信方向のチェック
    if ((CPU_CORE == 0 && icmcb->icmatr == TA_ICM_TO_C0) || (CPU_CORE == 1 && icmcb->icmatr == TA_ICM_TO_C1)) {
        err = E_CTX;
        goto EXIT;
    }

    if (icmcb->state == KS_EXIST) {
        if (icmcb->freesz > msgsz + sizeof(UB)) {
            // バッファに空きがある
            store_msg(icmcb, msg, msgsz);           // バッファへメッセージを格納
            icc_ras_int(ICCINT_ICM_SND<<24 | icmid); // 他CPUコアへのCPUコア間割り込みの発生
        } else {
            // バッファに空きがないので送信待ち状態に変更
            tqueue_remove_top(&ready_queue[CPU_CORE][cur_task[CPU_CORE]->itskpri]);     // タスクをレディキューから外す

            // TCBの各種情報を変更する
            cur_task[CPU_CORE]->state   = TS_WAIT;
            cur_task[CPU_CORE]->waifct  = TWFCT_ICMS;
            cur_task[CPU_CORE]->waiobj  = icmid;
            cur_task[CPU_CORE]->waitim  = ((tmout == TMO_FEVR) ? tmout : tmout + TIMER_PERIOD);
            cur_task[CPU_CORE]->msgsz   = msgsz;
            cur_task[CPU_CORE]->msg     = msg;
            cur_task[CPU_CORE]->waierr  = &err;

            tqueue_add_entry(&wait_queue[CPU_CORE], cur_task[CPU_CORE]);    // タスクをウェイトキューにつなぐ
            scheduler();
        }
    } else {
        err = E_NOEXS;
    }

EXIT:
    // クリティカルセクション終了
    icc_unl_spin(SPINLOCK_ICM);
    EI(intsts);
    return err;
}

// CPUコア間メッセージの受信API
INT icc_rcv_msg(ID icmid, void *msg, TMO tmout) {
    ICMCB   *icmcb;
    UINT    intsts;
    INT     msgsz   = 0;
    ER      err     = E_OK;

    if (icmid <= 0 || icmid > CNF_MAX_ICMID) return E_ID;

    // クリティカルセクション開始
    DI(intsts);
    icc_loc_spin(SPINLOCK_ICM);
    icmcb = &icmcb_tbl[--icmid];

    // メッセージ送信方向のチェック
    if ((CPU_CORE == 0 && icmcb->icmatr == TA_ICM_TO_C1) || (CPU_CORE == 1 && icmcb->icmatr == TA_ICM_TO_C0)) {
        err = E_CTX;
        goto EXIT;
    }

    if (icmcb->state == KS_EXIST) {
        if (icmcb->buf_rp != icmcb->buf_wp) {
            // メッセージが格納されている
            msgsz = retrieve_msg(icmcb, msg);           // バッファからメッセージ取得
            icc_ras_int(ICCINT_ICM_RCV<<24 | icmid);    // 他CPUコアへのCPUコア間割り込みの発生
        } else {
            tqueue_remove_top(&ready_queue[CPU_CORE][cur_task[CPU_CORE]->itskpri]);

            // TCBの各種情報を変更する
            cur_task[CPU_CORE]->state   = TS_WAIT;
            cur_task[CPU_CORE]->waifct  = TWFCT_ICMR;
            cur_task[CPU_CORE]->waiobj  = icmid;
            cur_task[CPU_CORE]->waitim  = ((tmout == TMO_FEVR) ? tmout : tmout + TIMER_PERIOD);
            cur_task[CPU_CORE]->msg     = msg;
            cur_task[CPU_CORE]->waierr  = &err;
            
            tqueue_add_entry(&wait_queue[CPU_CORE], cur_task[CPU_CORE]);
            scheduler();
        }
    } else {
        err = E_NOEXS;
    }

EXIT:
    // クリティカルセクション終了
    icc_unl_spin(SPINLOCK_ICM);
    EI(intsts);
    return (msgsz ? msgsz : (INT)err);
}

// CPU割り込みハンドラ CPUコア間メッセージ送信
void icm_snd_inthdr(UW intdat) {
    ICMCB   *icmcb;
    TCB     *tcb;
    ID      icmid;
    INT     msgsz;
    UINT    intsts;

    // クリティカルセクション開始
    DI(intsts);
    icc_loc_spin(SPINLOCK_ICM);

    icmid = intdat & 0x0FFF;
    icmcb = &icmcb_tbl[icmid];
    for (tcb = wait_queue[CPU_CORE]; tcb != NULL; tcb = tcb->next) {
        if ((tcb->waifct == TWFCT_ICMR) && (tcb->waiobj == icmid)) {
            msgsz = retrieve_msg(icmcb, tcb->msg);

            tqueue_remove_entry(&wait_queue[CPU_CORE], tcb);
            tcb->state  = TS_READY;
            tcb->waifct = TWFCT_NON;
            *(tcb->waierr) = msgsz;
            tqueue_add_entry(&ready_queue[CPU_CORE][tcb->itskpri], tcb);
            scheduler();
            break;
        }
    }

    // クリティカルセクション終了
    icc_unl_spin(SPINLOCK_ICM);
    EI(intsts);
}

// CPUコア間割り込みハンドラ CPUコア間メッセージ受信
void icm_rcv_inthdr(UW intdat) {
    ICMCB   *icmcb;
    TCB     *tcb;
    ID      icmid;
    UINT    intsts;

    // クリティカルセクション開始
    DI(intsts);
    icc_loc_spin(SPINLOCK_ICM);
    
    icmid = intdat & 0x0FFF;
    icmcb = &icmcb_tbl[icmid];
    for (tcb = wait_queue[CPU_CORE]; tcb != NULL; tcb = tcb->next) {
        if ((tcb->waifct == TWFCT_MBFR) && (tcb->waiobj == icmid)) {
            if (tcb->msgsz <= icmcb->freesz) {
                store_msg(icmcb, tcb->msg, tcb->msgsz);

                tqueue_remove_entry(&wait_queue[CPU_CORE], tcb);
                tcb->state  = TS_READY;
                tcb->waifct = TWFCT_NON;
                tqueue_add_entry(&ready_queue[CPU_CORE][tcb->itskpri], tcb);
                scheduler();
            }
            break;
        }
    }

    // クリティカルセクション終了
    EI(intsts);
    icc_unl_spin(SPINLOCK_ICM);
}

// CPUコア間メッセージの初期化
void init_icc_msg(void) {
    icc_def_int(ICCINT_ICM_SND, (FP)icm_snd_inthdr);
    icc_def_int(ICCINT_ICM_RCV, (FP)icm_rcv_inthdr);
}