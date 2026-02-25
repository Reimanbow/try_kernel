/**
 * @file context.c
 * Try Kernel コンテキスト管理
 */
#include <typedef.h>
#include <sysdef.h>
#include <syslib.h>
#include <knldef.h>

// スタック上の実行コンテキスト情報。SPレジスタの値は含まない
typedef struct {
    UW  r_[8];      // R4-R11 レジスタの値(ディスパッチャにより退避)
    UW  r[4];       // R0-R3 レジスタの値
    UW  ip;         // R12 レジスタの値
    UW  lr;         // LR レジスタの値
    UW  pc;         // PC レジスタの値
    UW  xpsr;       // XPSR レジスタの値
} StackFrame;

/**
 * @brief 初期実行コンテキストの作成
 * @param sp    タスク用スタック領域の先頭アドレス（低アドレス側）
 * @param ssize スタック領域のサイズ（バイト単位）
 * @param fp    タスクのエントリ関数（開始アドレス）
 * @return      作成した実行コンテキスト（＝初期SP値）
 */
void *make_context(UW *sp, UINT ssize, void (*fp)())
{
    StackFrame *sfp;

    /*
     * Cortex-Mのスタックは「高アドレス → 低アドレス」に向かって伸びる。
     *
     * なので、まずスタック領域の一番上（高アドレス側）へ移動する。
     *
     *   sp                : スタック領域の先頭（低アドレス）
     *   (UW)sp + ssize    : スタック領域の末尾（高アドレス）
     */
    sfp = (StackFrame*)((UW)sp + ssize);

    /*
     * いま sfp は「スタックの末尾（まだ未使用）」を指している。
     *
     * ここから StackFrame 1個分だけ下げることで、
     * 「スタック上に初期コンテキストを配置する場所」を確保する。
     *
     *        高アドレス
     *        ────────────
     *        未使用領域
     *        ──────────── ← (UW)sp + ssize
     *        ↓ ここにStackFrameを置く
     *        ──────────── ← sfp（初期SPになる）
     *        スタック領域
     *        ────────────
     *        低アドレス
     */
    sfp--;

    /*
     * ここから「例外復帰時にCPUがpopする形」を人工的に作る。
     *
     * Cortex-Mでは例外復帰時に以下が自動popされる：
     *   R0-R3, R12, LR, PC, xPSR
     *
     * なので、あらかじめその並びになるよう初期化する。
     */

    /*
     * xPSR の初期値
     * 0x01000000 は Thumbビット(Tビット=bit24)を1にする。
     * Cortex-MはThumb命令しか実行できないため必須。
     */
    sfp->xpsr = 0x01000000;

    /*
     * PC にタスク開始関数のアドレスを設定。
     * bit0はThumbフラグなのでクリアしておく。
     * （実際の実行時は内部的にThumbとして扱われる）
     */
    sfp->pc = (UW)fp & ~0x00000001UL;

    /*
     * 他のレジスタ（R0-R3, R4-R11, R12, LR）は
     * 必要なら初期値を設定してもよいが、
     * 最初の実行では特に値は使われないため未初期化でも可。
     */

    /*
     * sfp をそのまま返す。
     *
     * この値が「そのタスクの初期SP値」になる。
     *
     * ディスパッチ時に：
     *     SP = ctx_tbl[i];
     *
     * と設定し、例外復帰を行うことで、
     * あたかも割込みから戻るようにタスクが開始される。
     */
    return (void*)sfp;
}