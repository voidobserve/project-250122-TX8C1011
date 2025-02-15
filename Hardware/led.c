#include "led.h"

volatile u8 cur_sel_led = CUR_SEL_LED_NONE;
volatile u8 cur_ctl_led_blink_cnt = CUR_CTL_LED_BLINK_NONE; // 记录当前要控制灯光闪烁的次数

void led_config(void)
{
    // LED引脚配置为输出模式
    P1_MD0 |= 0x01 << 4; // P12
    // P1_MD0 |= 0x01 << 6; // P13
    P0_MD1 |= 0x01 << 2; // P05
}