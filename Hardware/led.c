#include "led.h"

// 20KHz
// #define STMR2_PRE (SYSCLK / 1 / (20000 - 100) - 1) // 值 == 1608

volatile u8 cur_sel_led = CUR_SEL_LED_NONE;
volatile u8 cur_ctl_led_blink_cnt = CUR_CTL_LED_BLINK_NONE; // 记录当前要控制灯光闪烁的次数

// void led_config(void)
// { 
//     // P12、P13对应的LED都使用PWM来驱动
//     P1_MD0 |= 0xA0; // P12、P13都配置为多功能IO模式
//     // P1_AF0 &= ~(0x0F); // (可以不写，默认就是0)P12复用为 STMR2_CHB 、P13复用为 STMR2_CHA

//     // 配置STIMER1
//     STMR2_FCONR = 0x00;          // 选择系统时钟，0分频
//     STMR2_PRH = STMR2_PRE / 256; // 周期值
//     STMR2_PRL = STMR2_PRE % 256;
//     // 占空比默认为0，不点亮LED
//     // STMR2_CMPAH = STMR2_PRE / 2 / 256; // 通道A占空比
//     // STMR2_CMPAL = STMR2_PRE / 2 % 256;
//     // STMR2_CMPBH = STMR2_PRE / 4 / 256; // 通道B占空比
//     // STMR2_CMPBL = STMR2_PRE / 4 % 256;

//     STMR2_PCONRA = 0x10; // 使能CHA，计数值大于CHA比较值输出0，小于输出1
//     STMR2_PCONRB = 0x10; // 使能CHB，计数值大于CHA比较值输出0，小于输出1
//     STMR2_CR |= 0x01;    // 使能高级定时器
// }

void led_red_on(void)
{
#ifdef USE_P13_RLED_USE_P12_GLED
    P1_MD0 &= ~(0x03 << 6);
    P1_MD0 |= 0x02 << 6;                 // 多功能IO模式
    P1_AF0 &= ~(0x03 << 6);              // 复用为 STMR2_CHA
    STMR2_CMPAH = (STMR2_PRE + 1) / 256; // 通道A占空比  100%
    STMR2_CMPAL = (STMR2_PRE + 1) % 256; 
#endif

#ifdef USE_P12_RLED_USE_P13_GLED
    P1_MD0 &= ~(0x03 << 4);
    P1_MD0 |= 0x02 << 4;                 // 多功能IO模式
    P1_AF0 &= ~(0x03 << 4);              // 复用为 STMR2_CHB
    STMR2_CMPBH = (STMR2_PRE + 1) / 256; // 通道B占空比  100%
    STMR2_CMPBL = (STMR2_PRE + 1) % 256;
#endif
}

void led_red_off(void)
{
#ifdef USE_P13_RLED_USE_P12_GLED
    P1_MD0 &= ~(0x03 << 6);
    P1_MD0 |= 0x01 << 6; // 输出模式
    P13 = 0;             // 输出低电平
#endif

#ifdef USE_P12_RLED_USE_P13_GLED
    P1_MD0 &= ~(0x03 << 4);
    P1_MD0 |= 0x01 << 4; // 输出模式
    P12 = 0;             // 输出低电平
#endif
}

void led_green_on(void)
{
#ifdef USE_P13_RLED_USE_P12_GLED 
    P1_MD0 &= ~(0x03 << 4);
    P1_MD0 |= 0x02 << 4;                 // 多功能IO模式
    P1_AF0 &= ~(0x03 << 4);              // 复用为 STMR2_CHB
    STMR2_CMPBH = (STMR2_PRE + 1) / 256; // 通道B占空比  100%
    STMR2_CMPBL = (STMR2_PRE + 1) % 256;
#endif

#ifdef USE_P12_RLED_USE_P13_GLED
    P1_MD0 &= ~(0x03 << 6);
    P1_MD0 |= 0x02 << 6;                 // 多功能IO模式
    P1_AF0 &= ~(0x03 << 6);              // 复用为 STMR2_CHA
    STMR2_CMPAH = (STMR2_PRE + 1) / 256; // 通道A占空比  100%
    STMR2_CMPAL = (STMR2_PRE + 1) % 256; 
#endif
}

void led_green_off(void)
{
#ifdef USE_P13_RLED_USE_P12_GLED 
    P1_MD0 &= ~(0x03 << 4);
    P1_MD0 |= 0x01 << 4; // 输出模式
    P12 = 0;             // 输出低电平
#endif

#ifdef USE_P12_RLED_USE_P13_GLED
    P1_MD0 &= ~(0x03 << 6);
    P1_MD0 |= 0x01 << 6; // 输出模式
    P13 = 0;             // 输出低电平
#endif
}