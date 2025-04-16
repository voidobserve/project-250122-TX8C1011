#include "tmr2.h"

// void tmr2_pwm_config(void)
// {
//     // 配置P01为timer2 pwm输出端口
//     // P0_MD0 &= ~(0x03 << 2);
//     // P0_MD0 |= 0x02 << 2;
//     // P0_AF0 &= ~(0x03 << 2);

//     P0_AF0 &= ~(0x03 << 2); // 复用模式选择为TMR2PWM输出

//     TMR2_PRL = TMR2_CNT_TIME % 256; // 周期值
//     TMR2_PRH = TMR2_CNT_TIME / 256;
//     // 占空比配置：（不能去掉，否则刚上电/充电时可能会是随机值，而不是0）
//     TMR2_PWML = 0; // 占空比 0%
//     TMR2_PWMH = 0;
//     TMR2_CNTL = 0x00; // 清除计数值
//     TMR2_CNTH = 0x00;
//     TMR2_CONL = (((0x0 & 0x7) << 5) | ((0x7 & 0x7) << 2) | ((0x2 & 0x3) << 0)); // 0分频，系统时钟，PWM模式

//     // 关闭定时器2
//     tmr2_pwm_disable();
// }

void tmr2_pwm_enable(void)
{
    // 配置P01为timer2 pwm输出端口
    P0_MD0 &= ~(0x03 << 2);
    P0_MD0 |= 0x02 << 2; // 多功能IO模式
    // P0_AF0 &= ~(0x03 << 2);

    // TMR2_PWML = 30; // 占空比 
    // TMR2_PWML = 0; // 占空比 0%
    // TMR2_PWMH = 0;
    TMR2_CONL |= 0x02; // 定时器2配置为PWM模式
}

void tmr2_pwm_disable(void)
{
    TMR2_CONL &= ~0x03; // 关闭定时器2
    P0_MD0 &= ~(0x03 << 2);
    P0_MD0 |= 0x01 << 2; // 输出
    P01 = 0;
    // TMR2_PWML = 0; // 占空比 0%
    // TMR2_PWMH = 0;
}

