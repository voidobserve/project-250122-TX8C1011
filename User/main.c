/**
 ******************************************************************************
 * @file    main.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    01-05-2021
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 * 版权说明后续补上
 *
 ******************************************************************************
 */

#include "include.h"
#include "my_config.h"

volatile bit flag_bat_is_empty = 0;     // 标志位，用于检测是否拔出了电池
volatile bit flag_bat_is_near_full = 0; // 标志位，表示电池是否快充满电
volatile bit flag_bat_is_full = 0;      // 电池是否被充满电的标志位

volatile bit flag_is_in_charging = 0;               // 是否处于充电的标志位
volatile bit flag_tim_scan_maybe_not_charge = 0;    // 用于给定时器扫描的标志位，可能检测到了拔出充电器
volatile bit flag_tim_scan_maybe_in_charging = 0;   // 用于给定时器扫描的标志位，可能检测到了充电器
volatile bit flag_tim_set_is_in_charging = 0;       // 由定时器置位/复位的，表示是否有插入充电器的标志位
volatile bit flag_tim_scan_bat_maybe_full = 0;      // 用于给定时器扫描的标志位，可能检测到电池被充满电
volatile bit flag_tim_set_bat_is_full = 0;          // 由定时器置位/复位的，表示电池是否被充满电的标志位
volatile bit flag_tim_scan_bat_maybe_near_full = 0; // 用于给定时器扫描的标志位，可能检测到电池快被充满电
volatile bit flag_tim_set_bat_is_near_full = 0;     // 由定时器置位/复位的，表示电池是否快被充满电的标志位

volatile bit flag_tim_scan_maybe_low_bat = 0; // 用于给定时器扫描的标志位，可能检测到了低电量
volatile bit flag_tim_set_bat_is_low = 0;     // 由定时器置位/复位的，表示在工作时，电池是否处于低电量的标志位

volatile bit flag_tim_scan_maybe_shut_down = 0; // 用于给定时器扫描的标志位，可能检测到了 电池电压 低于 关机对应的电压
volatile bit flag_tim_set_shut_down = 0;        // 由定时器置位/复位的，表示在工作时，检测到了 电池电压 在一段时间内都低于 关机对应的电压

volatile bit flag_tim_scan_maybe_motor_stalling = 0; // 用于给定时器扫描的标志位，可能检测到了电机堵转
volatile bit flag_tim_set_motor_stalling = 0;        // 由定时器置位/复位的，表示在工作时检测到了电机堵转

volatile bit flag_is_enable_key_scan = 0; // 标志位，是否使能按键扫描(每10ms被定时器置位，在主函数中检测并清零)
// volatile bit flag_is_dev_open = 0;        // （只在长按开机、关机时使用）设备是否开机的标志位，0--未开机，1--开机

volatile bit flag_ctl_led_blink = 0; // 控制标志位，是否控制指示灯闪烁
volatile bit flag_ctl_turn_dir = 0;  // 控制标志位，是否更换电机的方向

volatile bit flag_ctl_dev_close = 0; // 控制标志位，是否要关闭设备

volatile bit flag_ctl_low_bat_alarm = 0; // 控制标志位，是否使能低电量报警

volatile bit flag_is_disable_to_open = 0; // 标志位，是否不使能开机(低电量不允许开机)

volatile bit flag_is_recv_ctl = 0; // 标志位，是否接收了控制命令
/*
    标志位，是否通过语音来切换方向.
    用在定时器中断中，判断自动换方向，如果这个标志位置一，
    会清除当前自动换方向的计时，重新开始自动换方向的计时,
    定时器清除计时后，会给这个标志位清零
*/
volatile bit flag_is_turn_dir_by_speech = 0;

/*
    标志位，是否接收到了新的语音信息/有新的按键操作
    用在定时器中断，在通过语音关闭设备后，一段时间无操作而彻底关机
    如果这个标志位置一，在定时器内会清除自动彻底关机的倒计时，
    定时器在清除计时后，会给这个标志位清零
*/
volatile bit flag_is_new_operation = 0;

volatile bit flag_is_enter_low_power = 0; // 标志位，是否要进入低功耗

#if 1
// 控制函数，开机
void fun_ctl_power_on(void)
{
    alter_motor_speed(2); // 开机后，电机进入二档

    motor_pwm_b_enable(); // 电机正向转动

    cur_motor_dir = 1;    // 表示当前电机在正转
    cur_motor_status = 2; // 更新电机挡位状态

    // 让绿灯闪烁
    LED_GREEN_ON();
    cur_sel_led = CUR_SEL_LED_GREEN;

    cur_ctl_led_blink_cnt = cur_motor_status; // led闪烁次数与当前电机的挡位有关

    flag_ctl_led_blink = 1; // 打开LED闪烁的功能
}

// 控制函数，关机
void fun_ctl_power_off(void)
{
    // alter_motor_speed(0); 关机（好像可以不写这一条）

    // 关闭灯光闪烁的动画
    // flag_ctl_led_blink = 0;
    // delay_ms(1); // 等待定时器中断内部清空闪烁功能对应的标志和变量，否则打断闪灯的效果会变差
    interrupt_led_blink();
    LED_GREEN_OFF();

    if (0 == flag_is_in_charging)
    {
        // 不在充电，才关闭红色LED，否则充电时的红色LED实现不了呼吸灯效果
        LED_RED_OFF();
    }

    // 关闭加热
    cur_ctl_heat_status = 0;

    // 关闭驱动电机的PWM输出：
    motor_pwm_disable();

    cur_motor_status = 0; // 表示电机已经关闭
    cur_motor_dir = 0;    // 清零，回到初始状态

    // 关机可能是 充电时进入了关机、低电量进入了关机、手动关机、自动进入了关机
    // 如果是 充电时进入了关机，不应该清除相应的标志位
    if (0 == flag_is_in_charging)
    {
        // 如果没有在充电，清空跟低电量不开机无关的标志位
        // flag_bat_is_empty = 0;
        flag_bat_is_near_full = 0;
        flag_bat_is_full = 0;
        // flag_tim_scan_maybe_not_charge = 0;
        // flag_tim_scan_maybe_in_charging = 0;
        flag_tim_scan_bat_maybe_full = 0;
        flag_tim_scan_maybe_low_bat = 0;
        flag_tim_scan_maybe_shut_down = 0;
        flag_is_enable_key_scan = 0;
        flag_ctl_low_bat_alarm = 0;
    }
}

/**
 * @brief 控制电机挡位
 *           如果要根据传参来调节挡位，会改变 全局变量 cur_motor_status 的状态
 *
 * @param adjust_motor_status 要调节到的挡位
 *                          0--根据全局变量 cur_motor_status 的状态来自动调节
 *                          1--调节为1档
 *                          2--调节为2档
 *                          3--调节为3档
 *                          其他--函数直接返回 （为了节省程序空间，这里没有加）
 */
void fun_ctl_motor_status(u8 adjust_motor_status)
{
#if 0  // 可以节省5字节空间
    // if (adjust_motor_status > 3)
    // {
    //     return; // 传参有误，直接返回
    // }
#endif // 可以节省5字节空间

    if (0 == adjust_motor_status)
    {
        // 如果不是根据传参来调节挡位，
        // 而是根据全局变量cur_motor_status的状态来调节
        // if (1 == cur_motor_status)
        // {
        //     // 从 1档 -> 2档
        //     cur_motor_status = 2;
        // }
        // else if (2 == cur_motor_status)
        if (2 == cur_motor_status)
        {
            // 从 2档 -> 3档
            cur_motor_status = 3;
        }
        else if (3 == cur_motor_status)
        {
            // 从 3档 -> 1档
            cur_motor_status = 1;
        }
        else
        {
            // 1. 从初始状态 0 == cur_motor_status 变为 2 档
            // 2. 或者是 从 1档 -> 2档
            cur_motor_status = 2;
        }
    }
    else
    {
        // 如果是根据传参来调节挡位
        cur_motor_status = adjust_motor_status;
    }

    alter_motor_speed(cur_motor_status);

    // 每次切换挡位，都让定时器加载动画
    // flag_ctl_led_blink = 0; // 打断当前正在闪烁的功能
    // delay_ms(1);            // 等待定时器中断内部清空闪烁功能对应的标志和变量，否则打断闪灯的效果会变差
    interrupt_led_blink();

    // 如果不处于低电量报警状态，才使能LED闪烁功能： 
    if (0 == flag_ctl_low_bat_alarm)
    {
        if (0 == cur_ctl_heat_status)
        {
            // 如果没有打开加热，让绿灯闪烁
            LED_RED_OFF();
            LED_GREEN_ON();
            cur_sel_led = CUR_SEL_LED_GREEN;
        }
        else
        {
            // 如果打开了加热，让红灯闪烁
            LED_GREEN_OFF();
            LED_RED_ON();
            cur_sel_led = CUR_SEL_LED_RED;
        }

        cur_ctl_led_blink_cnt = cur_motor_status; // led闪烁次数与当前电机的挡位有关
        flag_ctl_led_blink = 1;                   // 打开LED闪烁的功能
    }
}
#endif

void user_config(void)
{
#if 0 // 未优化程序空间的代码：
    IE_EA = 1; // 使能总中断

    // 检测开机/模式按键 和 检测加热按键的引脚：
    // key_config();
    // 上拉：
    P0_PU |= 0x01 << 7;
    P1_PU |= 0x01;
    P0_MD1 &= ~(0x03 << 6); // P07 输入模式
    P1_MD0 &= ~0x03;        // P10 输入模式

    // heating_pin_config();    // 加热控制引脚
    P1_MD1 |= 0x01; // 用 P14 控制加热

                    // speech_ctl_pin_config(); // 控制语音IC电源的引脚
    // P11
    P1_MD0 |= 0x01 << 2; // 输出模式

    // led_config();
    // P12、P13对应的LED都使用PWM来驱动
    P1_MD0 |= 0xA0; // P12、P13都配置为多功能IO模式
    // P1_AF0 &= ~(0x0F); // (可以不写，默认就是0)P12复用为 STMR2_CHB 、P13复用为 STMR2_CHA
    // 配置STIMER1
    STMR2_FCONR = 0x00;          // 选择系统时钟，0分频
    STMR2_PRH = STMR2_PRE / 256; // 周期值
    STMR2_PRL = STMR2_PRE % 256;
    // 占空比默认为0，不点亮LED
    // STMR2_CMPAH = STMR2_PRE / 2 / 256; // 通道A占空比
    // STMR2_CMPAL = STMR2_PRE / 2 % 256;
    // STMR2_CMPBH = STMR2_PRE / 4 / 256; // 通道B占空比
    // STMR2_CMPBL = STMR2_PRE / 4 % 256;
    STMR2_PCONRA = 0x10; // 使能CHA，计数值大于CHA比较值输出0，小于输出1
    STMR2_PCONRB = 0x10; // 使能CHB，计数值大于CHA比较值输出0，小于输出1
    STMR2_CR |= 0x01;    // 使能高级定时器



    // tmr0_config();     // 1ms定时器
    __EnableIRQ(TMR0_IRQn);                                                     // 使能timer0中断
    TMR0_PRL = TMR0_CNT_TIME;                                                   // 周期值
    TMR0_CNTL = 0x00;                                                           // 清除计数值
    TMR0_CONH = 0xA0;                                                           // 使能计数中断
    TMR0_CONL = (((0x7 & 0x7) << 5) | ((0x7 & 0x7) << 2) | ((0x1 & 0x3) << 0)); // 128分频，系统时钟，count模式

    // tmr2_pwm_config(); // 控制升压的PWM
    P0_AF0 &= ~(0x03 << 2);         // 复用模式选择为TMR2PWM输出
    TMR2_PRL = TMR2_CNT_TIME % 256; // 周期值
    TMR2_PRH = TMR2_CNT_TIME / 256;
    // 占空比配置：（不能去掉，否则刚上电/充电时可能会是随机值，而不是0）
    TMR2_PWML = 0; // 占空比 0%
    TMR2_PWMH = 0;
    TMR2_CNTL = 0x00; // 清除计数值
    TMR2_CNTH = 0x00;
    TMR2_CONL = (((0x0 & 0x7) << 5) | ((0x7 & 0x7) << 2) | ((0x2 & 0x3) << 0)); // 0分频，系统时钟，PWM模式
    // 关闭定时器2
    tmr2_pwm_disable();

    // adc_config();
    // P04 AIN4 检测充电口传过来的ad值
    P0_MD1 |= 0x03;        // 模拟模式
    P0_AIOEN |= 0x01 << 4; // 使能模拟功能
    // P05 AIN5 检测电池分压后的ad值
    P0_MD1 |= 0x03 << 2;   // 模拟模式
    P0_AIOEN |= 0x01 << 5; // 使能模拟功能
    // 使用 P06 AIN06 检测电机是否堵转
    P1_MD1 |= 0x3 << 4;    // 模拟模式
    P1_AIOEN |= 0x01 << 6; // 使能模拟功能
    AIP_CON2 |= 0xC0;      // 使能ADC中CMP使能信号和CMP校准功能
    AIP_CON4 |= 0x01;      // 使能ADC偏置电流，参考电压选择内部2.4V(Note: 使用内部参考时，芯片需在5V电压供电下)
    ADC_CFG1 = 0x3C;       // ADC时钟分频，16分频
    ADC_CFG2 = 0xFF;       // ADC采样时钟，256个ADC时钟

    // motor_config();  // 控制电机的两路PWM
    P0_AF0 &= ~0xF0; // P02 复用为 STMR1_PWMB，P03 复用为 STMR1_PWMA
    // 配置STIMER1
    STMR1_FCONR = 0x00;          // 选择系统时钟，0分频
    STMR1_PRH = STMR1_PRE / 256; // 周期值
    STMR1_PRL = STMR1_PRE % 256;
    // 占空比，默认为0
    // STMR1_CMPAH = STMR1_PRE / 2 / 256;
    // STMR1_CMPAL = STMR1_PRE / 2 % 256;
    // STMR1_CMPBH = STMR1_PRE / 4 / 256;
    // STMR1_CMPBL = STMR1_PRE / 4 % 256;
    STMR1_PCONRA = 0x10; // 使能CHA，计数值大于CHA比较值输出0，小于输出1
    STMR1_PCONRB = 0x10; // 使能CHB，计数值大于CHA比较值输出0，小于输出1
    STMR1_CR |= 0x01;    // 使能高级定时器
    // 关闭定时器，IO配置为输出模式，输出低电平
    motor_pwm_disable();

    // uart1_config();
    // P00 RX
    P0_MD0 &= ~0x03;
    P0_MD0 |= 0x02; // 多功能IO模式
    P0_AF0 &= ~0x03;
    P0_AF0 |= 0x01;                             // P00 复用为 UART1_RX
    __EnableIRQ(UART1_IRQn);                    // 打开UART模块中断
    UART1_BAUD1 = (USER_UART_BAUD >> 8) & 0xFF; // 配置波特率高八位
    UART1_BAUD0 = USER_UART_BAUD & 0xFF;        // 配置波特率低八位
    UART1_CON = 0x91;                           // 8bit数据，1bit停止位，使能中断
    UART1_CON |= 0x02;                          // 接收模式

#if 0
    // 不使用的引脚配置为输出，输出低电平
    P1_MD1 |= 0x01 << 2;
    P15 = 0;
#endif

#endif // 未优化程序空间的代码

    IE_EA = 1; // 使能总中断

    // 上拉：
    // P0_PU |= 0x01 << 7;
    // P1_PU |= 0x01;
    P0_PU = 0x80; // P07 上拉
    P1_PU = 0x01; // P10 上拉

    /*
        P03 多功能IO模式
        P02 多功能IO模式
        P01 多功能IO模式
        P00 多功能IO模式
    */
    P0_MD0 = 0xAA;
    /*
        P07 输入模式 检测开机/模式切换的引脚
        P06 模拟模式 检测电机是否堵转
        P05 模拟模式 检测电池分压后的电压(外部1M上拉，330K下拉)
        P04 模拟模式 检测是否有充电(外部3.3K上拉，2.2K下拉)
    */
    P0_MD1 = 0x3F;
    /*
        P13 多功能IO模式 驱动红色LED
        P12 多功能IO模式 驱动绿色LED
        P11 输出模式 控制语音IC供电
        P10 输入模式 检测加热按键的引脚
    */
    // P1_MD0 = 0xA4;
    P1_MD0 = 0x54; // P12、P13是驱动LED的引脚，灯光初始状态应该是关闭的
    /*
        P15 输入模式 外部2K上拉，连接到充电C口的5V，10K下拉接地
        P14 输出模式 控制加热
    */
    P1_MD1 = 0x01;
    /*
        P03 复用为 STMR1_PWMA
        P02 复用为 STMR1_PWMB
        P01 复用为 TMR2_PWM
        P00 复用为 UART1_RX
    */
    P0_AF0 = 0x01;
    /*
        P07 不复用
        P06 不复用
        P05 不复用
        P04 不复用
    */
    // P0_AF1 = 0x00; // 默认就是0,可以不写
    /*
        P13 复用为 STMR2_CHA
        P12 复用为 STMR2_CHB
        P11 不复用
        P10 不复用
    */
    // P1_AF0 = 0x00; // 默认就是0,可以不写
    /*
        P15 不复用
        P14 不复用
    */
    // P1_AF1 = 0x00; // 默认就是0,可以不写

    /*
        只使能 P04、P05和P06 的模拟功能
    */
    P0_AIOEN = (0x01 << 4) | (0x01 << 5) | (0x01 << 6); //

    // P12、P13对应的LED都使用PWM来驱动
    LED_GREEN_OFF();
    LED_RED_OFF();
    // 配置STIMER1
    STMR2_FCONR = 0x00;          // 选择系统时钟，0分频
    STMR2_PRH = STMR2_PRE / 256; // 周期值
    STMR2_PRL = STMR2_PRE % 256;
    // 占空比默认为0，不点亮LED
    // STMR2_CMPAH = STMR2_PRE / 2 / 256; // 通道A占空比
    // STMR2_CMPAL = STMR2_PRE / 2 % 256;
    // STMR2_CMPBH = STMR2_PRE / 4 / 256; // 通道B占空比
    // STMR2_CMPBL = STMR2_PRE / 4 % 256;
    STMR2_PCONRA = 0x10; // 使能CHA，计数值大于CHA比较值输出0，小于输出1
    STMR2_PCONRB = 0x10; // 使能CHB，计数值大于CHA比较值输出0，小于输出1
    STMR2_CR |= 0x01;    // 使能高级定时器

    __EnableIRQ(TMR0_IRQn);                                                     // 使能timer0中断
    TMR0_PRL = TMR0_CNT_TIME;                                                   // 周期值
    TMR0_CNTL = 0x00;                                                           // 清除计数值
    TMR0_CONH = 0xA0;                                                           // 使能计数中断
    TMR0_CONL = (((0x7 & 0x7) << 5) | ((0x7 & 0x7) << 2) | ((0x1 & 0x3) << 0)); // 128分频，系统时钟，count模式

    // 控制升压的PWM
    TMR2_PRL = TMR2_CNT_TIME % 256; // 周期值
    TMR2_PRH = TMR2_CNT_TIME / 256;
    // 占空比配置：（不能去掉，否则刚上电/充电时可能会是随机值，而不是0）
    TMR2_PWML = 0; // 占空比 0%
    TMR2_PWMH = 0;
    // TMR2_CNTL = 0x00; // 清除计数值
    // TMR2_CNTH = 0x00;
    TMR2_CONL = (((0x0 & 0x7) << 5) | ((0x7 & 0x7) << 2) | ((0x2 & 0x3) << 0)); // 0分频，系统时钟，PWM模式
    // 关闭定时器2
    tmr2_pwm_disable();

    AIP_CON2 |= 0xC0; // 使能ADC中CMP使能信号和CMP校准功能
    AIP_CON4 |= 0x01; // 使能ADC偏置电流，参考电压选择内部2.4V(Note: 使用内部参考时，芯片需在5V电压供电下)
    ADC_CFG1 = 0x3C;  // ADC时钟分频，16分频
    ADC_CFG2 = 0xFF;  // ADC采样时钟，256个ADC时钟

    // 配置STIMER1
    STMR1_FCONR = 0x00;          // 选择系统时钟，0分频
    STMR1_PRH = STMR1_PRE / 256; // 周期值
    STMR1_PRL = STMR1_PRE % 256;
    // 占空比，默认为0
    // STMR1_CMPAH = STMR1_PRE / 2 / 256;
    // STMR1_CMPAL = STMR1_PRE / 2 % 256;
    // STMR1_CMPBH = STMR1_PRE / 4 / 256;
    // STMR1_CMPBL = STMR1_PRE / 4 % 256;
    STMR1_PCONRA = 0x10; // 使能CHA，计数值大于CHA比较值输出0，小于输出1
    STMR1_PCONRB = 0x10; // 使能CHB，计数值大于CHA比较值输出0，小于输出1
    STMR1_CR |= 0x01;    // 使能高级定时器
    // 关闭定时器，IO配置为输出模式，输出低电平
    motor_pwm_disable();

    // P00 RX
    __EnableIRQ(UART1_IRQn);                    // 打开UART模块中断
    UART1_BAUD1 = (USER_UART_BAUD >> 8) & 0xFF; // 配置波特率高八位
    UART1_BAUD0 = USER_UART_BAUD & 0xFF;        // 配置波特率低八位
    // UART1_CON = 0x91;                           // 8bit数据，1bit停止位，使能中断
    // UART1_CON |= 0x02;                          // 接收模式
    UART1_CON = 0x93; // 8bit数据，1bit停止位，使能中断,接收模式
}

void main(void)
{
// system_init();
#define HRCSC_STEP_VAL (*(u8 code *)0x9111) // 获取HRC时钟细调值
    u8 hrcsc_data = HRCSC_STEP_VAL & 0x7f;  // 获取HRC时钟细调值
    WDT_KEY = 0xDD;                         // Close WDT
    FLASH_TIM0 = 0x55;
    FLASH_TIM1 = 0x54; // FLASH 16M 访问速度
    FLASH_TRIM = 0x0A;
    AIP_CON0 = 0x80 | hrcsc_data; // 使能 hirc
    CLK_CON0 = 0x03;              // 选择 hirc clk
    CLK_CON2 = 0x00;              // 系统时钟不分频
    CLK_CON6 = 0x1F;              // FLASH烧写时钟32分频：1M
    delay_ms(1);                  // 该延迟不可删除，保证烧录稳定性
    __ADC_VREFP_TRIM_5V_CALIB;    // 填入5V电压供电下内部2.4V参考电压的校准值

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55;  // 解除写保护
    IO_MAP &= ~0x03; // 关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;

    // 初始化打印
#if USE_MY_DEBUG
    debug_init();
    printf("TXM8C101x_SDK main start\n");
#endif //  #if USE_MY_DEBUG

    user_config();

#if 1
    // 由于芯片下载之后会没有反应，这里用绿色灯作为指示：
    LED_GREEN_ON();
    // LED_RED_ON();
    delay_ms(1000);
    LED_GREEN_OFF();
    // LED_RED_OFF();
#endif

#if 0 // 上电时检测电池是否正确安装(测试通过):

    /*
        如果打开PWM后，检测电池的电压比满电还要高，说明没有接入电池，
        检测到的是充电5V升压后的电压
    */
    TMR2_PWML = 93 % 256; // 调节为约47.6%的占空比
    // 最大占空比的值不超过255，为了节省程序空间，可以不用配置高8位的寄存器
    // 但是这里不能去掉下面这一条，会导致电流异常跳动，设备会反复重启：
    // TMR2_PWMH = 93 / 256;
    TMR2_PWMH = 0;
    tmr2_pwm_enable();                // 打开控制升压电路的pwm
    adc_sel_channel(ADC_CHANNEL_BAT); // 切换到检测电池降压后的电压的检测引脚
    {
        u8 i = 0;
        u16 adc_val = 0;
        for (i = 0; i < 10; i++) // 每55ms进入一次，循环内每次间隔约4.8ms
        {
            adc_val = adc_get_val();
            if (adc_val >= ADCDETECT_BAT_FULL + ADCDETECT_BAT_NULL_EX)
            {
                flag_bat_is_empty = 1; // 表示电池是空的(电池没有安装)
            }
        }
    }
    tmr2_pwm_disable(); // 关闭控制升压电路的PWM
    TMR2_PWML = 0;      // 0%占空比
    // TMR2_PWMH = 0;  // 最大占空比的值不超过255，为了节省程序空间，可以不用配置高8位的寄存器

#endif // 上电时检测电池是否正确安装

    flag_is_enter_low_power = 1; // 上电之后就进入低功耗

    while (1)
    {

#if 0  // (测试通过)上电时，如果检测到电池没有安装，让LED闪烁，直到重新上电
        if (flag_bat_is_empty)
        {
            // 没有放入电池，控制LED闪烁，直到重新上电
            LED_RED_ON();
            delay_ms(200);
            LED_RED_OFF();
            delay_ms(200);
            continue;
        }
#endif // 上电时，如果检测到电池没有安装，让LED闪烁，直到重新上电

#if 1
        // charge_scan_handle();

        // if (0 == flag_is_in_charging &&   /* 不充电时，才对按键做检测和处理 */
        //     0 == flag_is_disable_to_open) /* 不处于低电量不能开机的状态时 */
        // {
        //     if (flag_is_enable_key_scan) // 每10ms，该标志位会被定时器置位一次
        //     {
        //         flag_is_enable_key_scan = 0;
        //         key_scan_10ms_isr();
        //     }

        //     key_event_handle();
        // }
        if (flag_is_enable_key_scan) // 每10ms，该标志位会被定时器置位一次
        {
            flag_is_enable_key_scan = 0;
            key_scan_10ms_isr();
        }

        key_event_handle();

        speech_scan_process(); // 检测是否有从语音IC传来的命令，如果有则做相应的处理
#endif

#if 1 // 电机自动转向
        if (flag_ctl_turn_dir)
        {
            // 如果要切换电机转向
            flag_ctl_turn_dir = 0;
            if (0 == cur_motor_dir)
            {
                // 如果当前电机为初始状态，不调节
            }
            else if (1 == cur_motor_dir)
            {
                // 如果当前电机为正转，调节为反转
                // PWM0EC = 0; // 关闭正转的PWM输出
                motor_pwm_b_disable(); // 关闭正转的PWM输出
                delay_ms(500);
                // PWM1EC = 1;        // 打开反转的PWM输出
                motor_pwm_a_enable(); // 打开反转的PWM输出
                cur_motor_dir = 2;    // 表示电机当前转向为 反转
            }
            else if (2 == cur_motor_dir)
            {
                // 如果当前电机为反转，调节为正转
                // PWM1EC = 0; // 关闭反转的PWM输出
                motor_pwm_a_disable(); // 关闭反转的PWM输出
                delay_ms(500);
                // PWM0EC = 1;        // 打开正转的PWM输出
                motor_pwm_b_enable();
                cur_motor_dir = 1; // 表示电机当前转向为 正转
            }
            else
            {
                // 其他情况，不考虑，不做处理
            }
        }
#endif // 电机自动转向

#if 1 // 电机过流检测和处理

        motor_over_current_detect_handle(); // 函数内部会检测电机有没有运行

#endif // 电机过流检测和处理

#if 1 // 根据控制标志位来控制关机
        if (flag_ctl_dev_close)
        {
            // 如果要关闭设备
            flag_ctl_dev_close = 0;

            if (flag_is_in_charging)
            {
                // 如果正在充电，直接关闭语音IC的电源
                SPEECH_POWER_DISABLE();
            }

            // 函数内部会判断是不是正在充电而关机
            fun_ctl_power_off();
        }
#endif // 根据控制标志位来控制关机

#if 1 // 低功耗

        if (flag_is_enter_low_power)
        {
            flag_is_enter_low_power = 0;

            // SPEECH_POWER_DISABLE(); // 关闭语音IC的电源(现在低功耗函数内部会关闭语音IC的电源)
            low_power();
        }

#endif // 低功耗

    } // while (1)
}

void TMR0_IRQHandler(void) interrupt TMR0_IRQn
{
    if (TMR0_CONH & 0x80)
    {
        TMR0_CONH |= 0x80;
        // 1ms产生一次中断，进入到这里

        if (flag_bat_is_empty)
        {
            return; // 如果电池为空，提前退出中断服务函数
        }

        { // 按键扫描
            static volatile u8 key_scan_cnt = 0;
            key_scan_cnt++;
            if (key_scan_cnt >= 10)
            {
                key_scan_cnt = 0;
                flag_is_enable_key_scan = 1;
            }
        } // 按键扫描

#if 1 // 检测是否拔出/插入充电器

        { // 检测是否拔出/插入充电器
            static u8 not_charge_ms_cnt = 0;
            static u8 charging_ms_cnt = 0;
            if (flag_is_in_charging)
            {
                if (flag_tim_scan_maybe_not_charge)
                {
                    // 正在充电，并且检测到可能有充电器断开，进行计时
                    not_charge_ms_cnt++;
                    if (not_charge_ms_cnt >= 250)
                    {
                        not_charge_ms_cnt = 0;
                        flag_tim_set_is_in_charging = 0;
                    }
                }
                else
                {
                    // 正在充电，并且没有检测到充电器断开：
                    not_charge_ms_cnt = 0;
                    flag_tim_set_is_in_charging = 1;
                }
            }
            else
            {
                if (flag_tim_scan_maybe_in_charging)
                {
                    // 没有在充电，但是检测到有充电器插入，进行计时，确认充电器是否真的插入：
                    charging_ms_cnt++;
                    if (charging_ms_cnt >= 250)
                    {
                        charging_ms_cnt = 0;
                        flag_tim_set_is_in_charging = 1;
                    }
                }
                else
                {
                    // 没有在充电，并且也没有检测到充电器插入
                    charging_ms_cnt = 0;
                    flag_tim_set_is_in_charging = 0;
                }
            }
        } // 检测是否拔出/插入充电器
#endif // 检测是否拔出/插入充电器

#if 1 // 控制灯光闪烁的效果

        { // 控制灯光闪烁的效果

            static volatile u16 blink_ms_cnt = 0; // 计数值，控制灯光闪烁的时间间隔

            // 状态机，
            // 0--初始值、无状态，
            // 1--准备进入闪烁，
            // 2--正在闪烁
            static volatile u8 cur_blink_status = 0;
            static volatile u8 __blink_cnt = 0; // 一开始存放要闪烁的次数，后面控制当前剩余闪烁次数

            if (flag_ctl_led_blink)
            {
                // 如果使能了灯光闪烁
                if (0 == cur_blink_status)
                {
                    // 如果当前灯光状态处于初始值，没有在闪烁
                    cur_blink_status = 1;
                }
            }
            else
            {
                // 如果不使能灯光闪烁，立即停止当前灯光闪烁的效果
                cur_blink_status = 0;
                blink_ms_cnt = 0;
                __blink_cnt = 0;
            }

            if (1 == cur_blink_status)
            {
                // 准备进入闪烁，判断要闪烁的是红灯还是绿灯，是要表示当前电机的挡位还是加热的挡位

                // 可以在闪烁前先关闭灯光，再点亮要闪烁的LED，
                // 最后再使能闪烁的效果，这样就不用加入下面的判断：
                // if (CUR_SEL_LED_RED == cur_sel_led)
                // {
                //     // 如果要让红灯闪烁
                //     // LED_GREEN_OFF();
                //     // LED_RED_ON();
                // }
                // else if (CUR_SEL_LED_GREEN == cur_sel_led)
                // {
                //     // 如果要让绿灯闪烁
                //     // LED_RED_OFF();
                //     // LED_GREEN_ON();
                // }

                // 让灯光保持点亮500ms
                blink_ms_cnt++;
                if (blink_ms_cnt >= 500)
                {
                    blink_ms_cnt = 0;
                    cur_blink_status = 2; // 表示正式进入LED闪烁状态
                }
            }
            else if (2 == cur_blink_status)
            {
                // 如果正在进行LED闪烁
                if (0 == __blink_cnt)
                {
                    __blink_cnt = cur_ctl_led_blink_cnt; // 存放当前要闪烁的次数
                }

                if (__blink_cnt) // 如果闪烁次数不为0，继续闪烁
                {
                    blink_ms_cnt++;
                    if (blink_ms_cnt <= 500)
                    {
                        // 熄灭LED 500ms
                        if (CUR_SEL_LED_RED == cur_sel_led)
                        {
                            // 如果要让红灯闪烁
                            LED_RED_OFF();
                        }
                        else if (CUR_SEL_LED_GREEN == cur_sel_led)
                        {
                            // 如果要让绿灯闪烁
                            LED_GREEN_OFF();
                        }
                    }
                    else if (blink_ms_cnt < 1000)
                    {
                        // 点亮LED 500ms
                        if (CUR_SEL_LED_RED == cur_sel_led)
                        {
                            // 如果要让红灯闪烁
                            LED_RED_ON();
                        }
                        else if (CUR_SEL_LED_GREEN == cur_sel_led)
                        {
                            // 如果要让绿灯闪烁
                            LED_GREEN_ON();
                        }
                    }
                    else
                    {
                        blink_ms_cnt = 0;
                        __blink_cnt--; // 完成了一次闪烁，计数值减一
                        if (0 == __blink_cnt)
                        {
                            // 如果剩余闪烁次数为0，结束闪烁
                            flag_ctl_led_blink = 0; // 不使能闪烁
                            cur_blink_status = 0;   // 恢复到初始值，表示没有在闪烁
                        }
                    }
                }
            }
        } // 控制灯光闪烁的效果
#endif // 控制灯光闪烁的效果

#if 1 // 控制加热

        { // 控制加热
            static volatile u8 __ctl_heat_ms_cnt = 0;

            if (1 == cur_ctl_heat_status)
            {
                // 一档加热
                HEATING_ON();
            }
            else if (2 == cur_ctl_heat_status)
            {
                // 二档加热
                __ctl_heat_ms_cnt++;
                if (__ctl_heat_ms_cnt <= 5)
                {
                    HEATING_ON();
                }
                else if (__ctl_heat_ms_cnt < 10)
                {
                    HEATING_OFF();
                }
                else
                {
                    __ctl_heat_ms_cnt = 0;
                }
            }
            else
            {
                // 无状态，关闭加热
                HEATING_OFF();
                __ctl_heat_ms_cnt = 0;
            }

        } // 控制加热
#endif // 控制加热

#if 1 // 控制自动关机

        { // 自动关机
            static volatile u32 shut_down_ms_cnt = 0;

            if ((0 != cur_motor_status ||     /* 如果电机不是关闭的 */
                 0 != cur_ctl_heat_status) && /* 如果加热不是关闭的 */
                0 == flag_is_in_charging      /* 当前没有在充电 */
            )
            {
                shut_down_ms_cnt++;
                if (shut_down_ms_cnt >= 600000) // xx ms 后，关机
                {
                    shut_down_ms_cnt = 0;
                    flag_ctl_dev_close = 1;
                    flag_is_enter_low_power = 1; // 允许进入低功耗
                }
            }
            else
            {
                shut_down_ms_cnt = 0;
            }
        } // 自动关机
#endif // 控制自动关机

#if 1 // 控制电机自动换方向

        { // 自动换方向

            static volatile u16 turn_dir_ms_cnt = 0; // 控制在运行时每 xx min切换一次转向的计时变量

            if ((0 != cur_motor_status) &&       /* 如果电机不是关闭的 */
                0 == flag_is_turn_dir_by_speech) /* 如果没有通过语音调节过方向 */

            {
                turn_dir_ms_cnt++;
                if (turn_dir_ms_cnt >= 60000) // xx ms 后，换方向
                // if (turn_dir_ms_cnt >= 60) // xx ms 后，换方向（测试用）
                {
                    turn_dir_ms_cnt = 0;
                    // 标志位置一，让换方向的操作在主函数执行，因为换方向要间隔500ms
                    flag_ctl_turn_dir = 1;
                }
            }
            else
            {
                turn_dir_ms_cnt = 0;

                if (flag_is_turn_dir_by_speech)
                {
                    flag_is_turn_dir_by_speech = 0; // 已经清除自动换方向的计时，便清除该标志位
                }
            }

        } // 自动换方向
#endif // 控制电机自动换方向

#if 1 // 检测是否快充满电

        {
            static volatile u16 bat_is_near_full_ms_cnt = 0;
            // 如果不在充电就清空了标志位 flag_tim_scan_bat_maybe_near_full ，
            //  |-- 可以不加这个判断条件：
            // if (flag_is_in_charging)
            {
                if (flag_tim_scan_bat_maybe_near_full)
                {
                    // 正在充电，且检测到快充满电，进行计时
                    bat_is_near_full_ms_cnt++;
                    if (bat_is_near_full_ms_cnt >= 5000) // xx ms
                    {
                        bat_is_near_full_ms_cnt = 0;
                        flag_tim_set_bat_is_near_full = 1;
                    }
                }
                else
                {
                    // 正在充电，但是有一次没有检测到快充满电，清除计时和标志位
                    bat_is_near_full_ms_cnt = 0;
                    flag_tim_set_bat_is_near_full = 0;
                }
            }
        }

#endif // 检测是否快充满电

#if 1 // 检测是否充满电

        { // 检测是否充满电
            static volatile u16 bat_is_full_ms_cnt = 0;
            // 如果不在充电就清空了标志位 flag_tim_scan_bat_maybe_full ，
            //  |-- 可以不加这个判断条件：
            // if (flag_is_in_charging)
            {
                if (flag_tim_scan_bat_maybe_full)
                {
                    // 正在充电，并且有检测到电池满电，进行累计：
                    bat_is_full_ms_cnt++;
                    if (bat_is_full_ms_cnt >= 5000) // xx ms
                    {
                        bat_is_full_ms_cnt = 0;
                        flag_tim_set_bat_is_full = 1;
                    }
                }
                else
                {
                    // 正在充电，但是没有检测到电池满电：
                    bat_is_full_ms_cnt = 0;
                    flag_tim_set_bat_is_full = 0;
                }
            }
        } // 检测是否充满电
#endif // 检测是否充满电

#if 1 // 检测是否要低电量报警

        {
            static volatile u16 low_bat_ms_cnt = 0;
            if (flag_tim_scan_maybe_low_bat)
            {
                // 如果可能检测到了低电量，进行连续计时
                low_bat_ms_cnt++;
                if (low_bat_ms_cnt >= LOW_BAT_SCAN_TIMES_MS)
                {
                    low_bat_ms_cnt = 0;
                    flag_tim_set_bat_is_low = 1;
                }
            }
            else
            {
                // 如果没有检测到低电量
                low_bat_ms_cnt = 0;
                flag_tim_set_bat_is_low = 0;
            }
        }

        { // 根据标志位来执行低电量报警的功能，执行前(给控制标志位置一前)要先关闭所有LED

            static volatile bit __flag_is_in_low_bat_alarm = 0;   // 标志位，是否正在执行低电量报警的功能
            static volatile u16 __blink_cnt_in_low_bat_alarm = 0; // 低电量报警时，LED闪烁时间计数

            /* 如果使能了低电量报警，并且设备正在运行 */
            if (flag_ctl_low_bat_alarm &&
                (0 != cur_motor_status || 0 != cur_ctl_heat_status))
            {
                if (0 == __flag_is_in_low_bat_alarm)
                {
                    __flag_is_in_low_bat_alarm = 1; // 在该语句块内部，使能低电量报警的功能
                }
            }
            else
            {
                __blink_cnt_in_low_bat_alarm = 0;
                __flag_is_in_low_bat_alarm = 0;
            }

            if (__flag_is_in_low_bat_alarm)
            {
                __blink_cnt_in_low_bat_alarm++;
                if (__blink_cnt_in_low_bat_alarm <= 300)
                {
                    LED_RED_ON();
                }
                else if (__blink_cnt_in_low_bat_alarm < 600)
                {
                    LED_RED_OFF();
                }
                else
                {
                    __blink_cnt_in_low_bat_alarm = 0;
                }
            }
        } // 根据标志位来执行低电量报警的功能，执行前(给控制标志位置一前)要先关闭所有LED
#endif // 检测是否要低电量报警

#if 1 // 工作时，检测电池电量是否一直低于关机电压

        {
            static u16 __shut_down_cnt = 0; // 对电池电压低于关机电压的连续计时

            // 如果检测到电池低电压，并且电机没有堵转(电机堵转时会把电池电压拉低，会影响判断)
            if (flag_tim_scan_maybe_shut_down && 0 == flag_tim_scan_maybe_motor_stalling)
            {
                // 如果检测到在工作时，电池电压低于关机电压，进行连续计时
                __shut_down_cnt++;
                if (__shut_down_cnt >= SHUT_DOWN_SCAN_TIMES_MS)
                {
                    __shut_down_cnt = 0;
                    flag_tim_set_shut_down = 1;
                }
            }
            else
            {
                // 如果在工作时，没有检测到 电池电压低于关机电压
                __shut_down_cnt = 0;
                flag_tim_set_shut_down = 0;
            }
        }
#endif // 工作时，检测电池电量是否一直低于关机电压

#if 1 // 电机过流检测，超过10s便认为电机堵转

        {
            // 检测到电机堵转后，进行连续计时
            static volatile u16 __detect_motor_stalling_cnt = 0;

            // if (flag_tim_scan_maybe_motor_stalling && 0 != cur_motor_status)
            if (flag_tim_scan_maybe_motor_stalling && cur_motor_status)
            {
                // 如果检测到有电机堵转的情况
                __detect_motor_stalling_cnt++;
                if (__detect_motor_stalling_cnt >= MOTOR_STALLING_SCAN_TIMES_MS)
                {
                    __detect_motor_stalling_cnt = 0;
                    flag_tim_set_motor_stalling = 1;
                }
            }
            else
            {
                // 如果没有检测到有电机堵转的情况
                // P00 = 0; // 方便测试过流检测时间
                __detect_motor_stalling_cnt = 0;
                flag_tim_set_motor_stalling = 0;
            }
        }
#endif // 电机过流检测，超过10s便认为电机堵转

#if 1 // 控制语音IC关机后，无操作而关机

        {
            static u32 no_operation_shut_down_cnt = 0; // 无操作自动关机的计时

            if (0 == flag_is_new_operation && /* 如果没有新的操作 */
                0 == cur_motor_status &&      /* 如果电机关闭 */
                // 0 == cur_ctl_heat_status &&   /* 如果加热关闭 */
                0 == flag_is_enter_low_power) /* 如果没有使能进入低功耗 */
            {
                no_operation_shut_down_cnt++;
                if (no_operation_shut_down_cnt >= NO_OPERATION_SHUT_DOWN_TIMES_MS)
                {
                    no_operation_shut_down_cnt = 0;
                    flag_is_enter_low_power = 1;
                }
            }
            else
            {
                no_operation_shut_down_cnt = 0;
                flag_is_new_operation = 0;
            }
        }
#endif // 控制语音IC关机后，无操作而关机

#if 1 // 控制呼吸灯效果

        {
            // 0 -- 初始状态
            // 1 -- 刚进入呼吸灯状态
            // 2 -- 从亮到灭
            // 3 -- 从灭到亮
            static u8 cur_breathing_status = 0;
            static u16 pwm_duty = 0;

            if (flag_is_in_charging &&
                (0 == flag_bat_is_near_full) &&
                (0 == flag_bat_is_full))
            {
                if (0 == cur_breathing_status)
                {
                    cur_breathing_status = 1;
                }
            }
            else
            {
                cur_breathing_status = 0;
                pwm_duty = 0;
            }

            if (1 == cur_breathing_status)
            {
                // 刚进入呼吸灯闪烁效果
                pwm_duty = (STMR2_PRE + 1);
                cur_breathing_status = 2;
                LED_RED_ON();
            }
            else if (2 == cur_breathing_status)
            {
                if (pwm_duty > 0)
                {
                    pwm_duty--;
                }
                else
                {
                    cur_breathing_status = 3;
                    pwm_duty = 0;
                }

#ifdef USE_P13_RLED_USE_P12_GLED
                STMR2_CMPAH = (pwm_duty) / 256; // 通道A占空比
                STMR2_CMPAL = (pwm_duty) % 256;
#endif

#ifdef USE_P12_RLED_USE_P13_GLED
                STMR2_CMPBH = (pwm_duty) / 256; // 通道B占空比  100%
                STMR2_CMPBL = (pwm_duty) % 256;
#endif
            }
            else if (3 == cur_breathing_status)
            {
                if (pwm_duty < (STMR2_PRE + 1))
                {
                    pwm_duty++;
                }
                else
                {
                    cur_breathing_status = 2;
                    pwm_duty = (STMR2_PRE + 1);
                }

#ifdef USE_P13_RLED_USE_P12_GLED
                STMR2_CMPAH = (pwm_duty) / 256; // 通道A占空比
                STMR2_CMPAL = (pwm_duty) % 256;
#endif

#ifdef USE_P12_RLED_USE_P13_GLED
                STMR2_CMPBH = (pwm_duty) / 256; // 通道B占空比
                STMR2_CMPBL = (pwm_duty) % 256;
#endif
            }
        }

#endif // 控制呼吸灯效果
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
