#include "tmr1.h"

// 配置1ms定时器
#define TMR0_CNT_TIME (SYSCLK / 128 / (1000 - 20) - 1) // 计数周期 1KHz(实际上会有误差，这里要加上补偿)


void tmr1_config(void)
{
    __EnableIRQ(TMR0_IRQn); // 使能timer0中断
    IE_EA = 1;              // 使能总中断

    TMR0_PRL = TMR0_CNT_TIME;                                                   // 周期值
    TMR0_CNTL = 0x00;                                                           // 清除计数值
    TMR0_CONH = 0xA0;                                                           // 使能计数中断
    TMR0_CONL = (((0x7 & 0x7) << 5) | ((0x7 & 0x7) << 2) | ((0x1 & 0x3) << 0)); // 128分频，系统时钟，count模式
}