#ifndef __LED_H
#define __LED_H

#include "my_config.h"

#define STMR2_PRE (SYSCLK / 1 / (15992) - 1) // 值 约为2000

/*
    配置灯光闪烁时，需要先 确保熄灭了不闪烁的LED，
    点亮需要闪烁的LED，	填写好 cur_sel_led 、
    cur_ctl_led_blink_cnt ，再置位 flag_ctl_led_blink
*/

// 记录当前使用的led，0--初始状态，1--当前是红灯，2--当前是绿灯
enum
{
    CUR_SEL_LED_NONE = 0,
    CUR_SEL_LED_RED,
    CUR_SEL_LED_GREEN,
};
extern volatile u8 cur_sel_led;
// 定义灯光闪烁的各个次数：
enum
{
    CUR_CTL_LED_BLINK_NONE = 0,
    CUR_CTL_LED_BLINK_1, // 闪烁1次，对应1档
    CUR_CTL_LED_BLINK_2, // 闪烁2次，对应2档
    CUR_CTL_LED_BLINK_3, // 闪烁3次，对应3档
};
extern volatile u8 cur_ctl_led_blink_cnt; // 记录当前要控制灯光闪烁的次数

void led_config(void);
void led_red_on(void);
void led_red_off(void);
void led_green_on(void);
void led_green_off(void);

// 在PCB上，红灯和绿灯都是高电平点亮
// #define LED_RED_PIN P12   // 控制红色LED的引脚
// #define LED_GREEN_PIN P05 // 控制绿色LED的引脚
#define LED_RED_ON()      \
    {                     \
        do                \
        {                 \
            led_red_on(); \
        } while (0);      \
    }
#define LED_RED_OFF()        \
    {                        \
        do                   \
        {                    \
            led_red_off(); \
        } while (0);         \
    }
#define LED_GREEN_ON()         \
    {                          \
        do                     \
        {                      \
            led_green_on(); \
        } while (0);           \
    }
#define LED_GREEN_OFF()        \
    {                          \
        do                     \
        {                      \
            led_green_off(); \
        } while (0);           \
    }

#endif