#include "low_power.h"

extern void user_config(void);

/*
    低功耗，测试时，没有焊上语音IC，低功耗期间，整个板的功耗为14uA
*/
void low_power(void)
{
    // 如果是长按关机，下面的判断会导致无法进入低功耗
    // 如果一直按着开机/模式按键，返回
    // if (P15 || 0 == P07) // 如果检测到在充电或是按着开机按键
    // if (0 == P07)
    // {
    //     return;
    // }

    // while (0 == P07)
    //     ;             // 等待开关按键松开

    clear_key_scan(); // 清除按键扫描使用的全局变量，防止由按键关机进入低功耗之后，需要第二次长按才能开机的情况

label_low_power_begin: // 标签，刚开始进入低功耗

    // 除了P10、P15为输入模式，其余全部配置为输出模式
    P0_MD0 = 0x55;
    P0_MD1 = 0x15;
    P1_MD0 = 0x55;

    // 关闭片上外设:
    __DisableIRQ(TMR0_IRQn);  // 屏蔽TMR0中断
    TMR0_CONL &= ~0x03;       // 关闭tmr0
    tmr2_pwm_disable();       // 关闭tmr2 控制升压的PWM，相应的引脚输出低电平
    ADC_CFG0 &= ~(0x14);      // 关闭ADC和使用到的ADC通道0
    motor_pwm_disable();      // 关闭驱动电机的定时器(pwm)，相应的两个引脚输出低电平
    __DisableIRQ(UART1_IRQn); // 屏蔽接收控制命令的串口中断
    UART1_CON &= ~(0x11);     // 关闭接收控制命令的串口，关闭RX中断

    // 接收控制命令的引脚配置为输出模式，输出低电平:
    // P0_MD0 &= ~0x03;
    // P0_MD0 |= 0x01; // 输出模式
    // P00 = 0;

    STMR2_CR &= ~0x01; // 关闭驱动LED的定时器(pwm)
    // 关闭LED：
    LED_RED_OFF();
    LED_GREEN_OFF();
    HEATING_OFF();          // 控制加热的引脚输出低电平
    SPEECH_POWER_DISABLE(); // 控制语音IC电源的引脚
    // 检测电机是否堵转的引脚配置为输出模式，输出低电平
    // 检测电池分压后的电压的引脚配置为输出模式，输出低电平
    // 检测是否有充电的引脚配置为输出模式，输出低电平
    // 也就是 P04、P05、P06都配置为输出模式，输出低电平
    // P0_MD1 &= ~0x3F;
    // P0_MD1 |= 0x15; // P04 P05 P06 输出模式
    // P0 &= ~0x70;    // P04 P05 P06 输出低电平

    P0 &= ~0x71; // P04 P05 P06 输出低电平

// 如果是 使用第 07 脚检测 开关/模式按键 使用第 10 脚检测 加热按键
#ifdef USE_P07_DETECT_MODE_USE_P10_DETECT_HEAT
    // 关闭检测按键的上拉
    // P0_PU &= ~(0x01 << 7); // 不能关闭这个上拉电阻，会无法唤醒
    P1_PU &= ~0x01; // 关闭P10的上拉

    // 将检测加热按键的引脚配置为输出模式，输出低电平：
    // P1_MD0 &= ~0x03; //
    // P1_MD0 |= 0x01;  // P10 输出模式
    P10 = 0;

    // 检测 开关/模式按键 的引脚配置为输入上拉，检测到低电平触发中断，唤醒CPU
    WDT_KEY = 0x55; // 解除写保护
    IO_MAP |= 0x01 << 4;
    WDT_KEY = 0xBB;
    // P1_TRG0 |= ; // 不用配置触发边沿，默认就是双边沿触发
    P0_IMK |= 0x01 << 7;  // 使能 P07 中断
    __EnableIRQ(P0_IRQn); // 在总中断使能P0中断
    // 使能对应的唤醒通道：
    // WKUP_CON0 |= 0x11; // P07 连接到的唤醒单元使能，低电平触发唤醒
#endif

// 如果是 使用第 10 脚检测 开关/模式按键 使用第 07 脚检测 加热按键
#ifdef USE_P10_DETECT_MODE_USE_P07_DETECT_HEAT
    P0_MD1 &= ~(0x03 << 6);
    P0_MD1 |= 0x01 << 6; // P07 输出模式
    P07 = 0;

    // 检测 开关/模式按键 的引脚配置为输入上拉，检测到低电平触发中断，唤醒CPU
    P1_IMK |= 0x01;       // 使能P10中断
    __EnableIRQ(P1_IRQn); // 在总中断使能P1中断
    // 使能对应的唤醒通道：
    WKUP_CON0 |= 0x22; // P10 连接到的唤醒单元使能，低电平触发唤醒
#endif

    P1_MD1 &= ~(0x03 << 2);
    P1_MD1 |= (0x01 << 3); // P15配置为多功能IO模式
    P1_AF1 |= (0x03 << 2); // P15复用为 PORT_WKUP_IN1 function
    P1_IMK |= 0x01 << 5;  // 使能P15中断
    __EnableIRQ(P1_IRQn); // 在总中断使能P1中断
    // 使能对应的唤醒通道：
    // P07 连接到的唤醒单元0使能，低电平触发唤醒
    // P15 连接到的唤醒单元1使能，高电平触发唤醒
    // WKUP_CON0 = 0x11 | (0x01 << 1);
    WKUP_CON0 = (0x01 << 4) | (0x01 << 1) | (0x01 << 0);

    // ======================================================================
    WKPND = 0xFF;          // 唤醒前清除pending
    LP_CON |= 0x01 << 6;   // 唤醒后程序会继续运行
    __ENTER_SLEEP_MODE;    // 进入SLEEP
    __SLEEP_AFTER_WAKE_UP; // SLEEP唤醒后
    // ======================================================================

    P1_IMK &= ~(0x01 << 5); // 关闭P15中断
    __DisableIRQ(P1_IRQn);  // 在总中断关闭P1中断
    // WKUP_CON0 &= ~(0x01 << 1); // 关闭P15连接到的唤醒单元

    // 如果是 使用第 07 脚检测 开关/模式按键 使用第 10 脚检测 加热按键
#ifdef USE_P07_DETECT_MODE_USE_P10_DETECT_HEAT
    // 关闭检测 开关/模式 按键引脚的中断
    P0_IMK &= ~(0x01 << 7); // 不使能 P07 中断
    __DisableIRQ(P0_IRQn);  // 在总中断不使能P0中断
    // WKUP_CON0 &= ~(0x11);   // 关闭P07连接到的唤醒单元

    // 关闭P15连接到的唤醒单元
    // 关闭P07连接到的唤醒单元
    WKUP_CON0 &= ~((0x01 << 1) | 0x11);
    // P1_AF1 & = ~(0x03 << 2);
    // P1_MD1 &= ~(0x03 << 2); // P15配置为输入模式
    P1_MD1 = 0x01; // P15配置为输入模式，P14配置为输出模式

    WDT_KEY = 0x55; // 解除写保护
    IO_MAP &= ~(0x01 << 4);
    WDT_KEY = 0xBB;
#endif

// 如果是 使用第 10 脚检测 开关/模式按键 使用第 07 脚检测 加热按键
#ifdef USE_P10_DETECT_MODE_USE_P07_DETECT_HEAT
    // 关闭检测 开关/模式 按键引脚的中断
    P1_IMK &= ~0x01;       // 关闭P10中断
    __DisableIRQ(P1_IRQn); // 在总中断关闭P1中断
#endif

    // 唤醒后，如果没有检测到 充电或是按着开机/模式按键，重新回到低功耗
    if (0 == P07 || P15)
    {
        u8 i = 0;
        u8 cnt = 0;

        for (i = 0; i < 200; i++)
        {
            if (0 == P07 || P15)
            {
                cnt++;
            }

            delay_ms(1);
        }

        if (cnt >= 180)
        {
        }
        else
        {
            goto label_low_power_begin;
        }
    }

    // 重新初始化
    user_config();
    key_scan_10ms_isr(); // 获取一次键值
    // LED_GREEN_ON(); // 测试用，看看能不能唤醒
}

// void WDT_IRQHandler(void) interrupt WDT_IRQn
// {
//     if (WDT_CON & 0x40)
//     {
//         WDT_KEY = 0xAA; // feed wdt
//     }
// }

void P0_IRQHandler(void) interrupt P0_IRQn
{
    // if (P0_PND)
    // {
    P0_PND = 0; // 清 中断标志位
    // }
}

void P1_IRQHandler(void) interrupt P1_IRQn
{
    // if (P1_PND)
    // {
    P1_PND = 0; // 清 中断标志位
    // }
}