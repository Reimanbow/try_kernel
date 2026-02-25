/**
 * @file dispatch.S
 * Try Kernel 簡易ディスパッチャ
 */

    .align 2
    .thumb_func
    .globl dispatch_entry

dispatch_entry:
    mov     r0, #1
    msr     primask, r0     // 割り込み禁止
    ldr     r1, =disp_running
    str     r0, [r1]

    /**
     * 1. 実行中のタスクの実行コンテキスト情報をスタックに退避
     *    ソフトウェア割込み時にR0~R3、R12、LR、PC、xPSRはスタックに退避されている
     *    pushでは、R0~R7のレジスタとLRレジスタのみが命令の対象
     */
    push    {r4-r7}
    mov     r0, r8
    mov     r1, r9
    mov     r2, r10
    mov     r3, r11
    push    {r0-r3}

    /**
     * 2. 現在実行中のタスクの確認
     */
    ldr     r0, =cur_task
    ldr     r1, [r0]
    cmp     r1, #0
    beq     disp_010        // cur_taskのID番号=0ならばdisp_010へ。起動時最初のタスクなので保存するべき実行コンテキストがないから
    
    /**
     * 3. 実行コンテキスト情報へのポインタを実行中のタスクのTCBに格納
     */
    mov     r2, sp
    str     r2, [r1]

disp_010:
    /**
     * 4. 次に実行するタスクの確認
     */
    ldr     r1, =sche_task
    ldr     r2, [r1]
    cmp     r2, #0
    bne     disp_030

    /**
     * 5. 次に実行するタスクがなかった場合の処理
     */
    str     r2, [r0]
disp_020:
    mov     r3, #0
    msr     primask, r3     // 割り込み許可
    mov     r3, #1
    msr     primask, r3     // 割り込み禁止

    ldr     r2, [r1]
    cmp     r2, #0
    beq     disp_020

    /**
     * 6. 実行するタスクの切り替え
     */
disp_030:
    str     r2, [r0]
    ldr     r0, [r2]
    mov     sp, r0

    /**
     * 7. スタック上のコンテキスト情報の復元
     *    popでは、R0~R7のレジスタとPCレジスタのみが命令の対象
     */
    pop     {r0-r3}
    mov     r11, r3
    mov     r10, r2
    mov     r9, r1
    mov     r8, r0
    pop     {r4-r7}

    /* 割り込み許可 */
    ldr     r0, =disp_running
    mov     r1, #0
    str     r1, [r0]
    msr     primask, r1     // 割り込み許可

    bx      lr