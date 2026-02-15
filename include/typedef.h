/**
 * @file typedef.h
 * @brief Try Kernel データ型定義
 */
#ifndef TYPEDEF_H
#define TYPEDEF_H

#include <stdint.h>

/* 符号付き整数 */
typedef int8_t  B;
typedef int16_t H;
typedef int32_t W;
typedef int64_t D;

/* 符号無し整数 */
typedef uint8_t     UB;
typedef uint16_t    UH;
typedef uint32_t    UW;
typedef uint64_t    UD;

/* volatile 符号付き整数 */
typedef volatile int8_t     _B;
typedef volatile int16_t    _H;
typedef volatile int32_t    _W;
typedef volatile int64_t    _D;

/* volatile 符号無し整数 */
typedef volatile uint8_t    _UB;
typedef volatile uint16_t   _UH;
typedef volatile uint32_t   _UW;
typedef volatile uint64_t   _UD;

/* サイズ指定なし */
typedef int             INT;
typedef unsigned int    UINT;

/* 特別な意味を持つ整数 */
typedef INT     ID;     // ID番号
typedef UW      ATR;    // 属性
typedef INT     ER;     // エラーコード
typedef INT     PRI;    // 優先順位
typedef W       TMO;    // タイムアウト時間
typedef UW      RELTIM; // 相対時間
typedef W       SZ;     // サイズ

typedef void    (*FP)();    // 関数ポインタ

#define NULL    (0)

typedef UINT    BOOL;
#define TRUE    (1)
#define FALSE   (0)

#endif /* TYPEDEF_H */