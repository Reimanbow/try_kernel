/**
 * @file gpio.h
 * @brief Try Kernel GPIO制御
 */
#ifndef GPIO_H
#define GPIO_H

/**
 * @brief GPIO入力有効
 */
void gpio_enable_input(UINT no);

/**
 * @brief GPIO出力有効
 */
void gpio_enable_output(UINT no, UINT ival);

/**
 * @brief GPIO Pull-Up有効
 */
void gpio_enable_pullup(UINT no);

/**
 * @brief GPIO Pull-Down有効
 */
void gpio_enable_pulldown(UINT no);

/**
 * @brief GPIO割り込みモード設定
 */
void gpio_set_intmode(UINT no, UW mode);

#endif /* GPIO_H */