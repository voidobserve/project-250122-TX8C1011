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

volatile u8 flag_bat_is_empty; // 标志位，用于检测是否拔出了电池

volatile bit flag_is_in_charging = 0;             // 是否处于充电的标志位
volatile bit flag_tim_scan_maybe_not_charge = 0;  // 用于给定时器扫描的标志位，可能检测到了拔出充电器
volatile bit flag_tim_scan_maybe_in_charging = 0; // 用于给定时器扫描的标志位，可能检测到了充电器
volatile bit flag_tim_set_is_in_charging = 0;     // 由定时器置位/复位的，表示是否有插入充电器的标志位

volatile bit flag_is_enable_key_scan = 0; // 标志位，是否使能按键扫描(每10ms被定时器置位，在主函数中检测并清零)
volatile bit flag_is_dev_open = 0;        // 设备是否开机的标志位，0--未开机，1--开机

volatile bit flag_ctl_led_blink = 0; // 控制标志位，是否控制指示灯闪烁
volatile bit flag_ctl_turn_dir = 0;  // 控制标志位，是否更换电机的方向

volatile bit flag_ctl_dev_close = 0; // 控制标志位，是否要关闭设备

// 控制函数，开机
void fun_ctl_power_on(void)
{
    alter_motor_speed(2); // 开机后，电机进入二档

    motor_pwm_b_enable(); // 电机正向转动

    cur_motor_dir = 1;    // 表示当前电机在正转
    cur_motor_status = 2; // 更新电机挡位状态
    flag_is_dev_open = 1; // 表示设备已经开启

    // 让绿灯闪烁
    LED_GREEN_ON();
    cur_sel_led = CUR_SEL_LED_GREEN;
    cur_ctl_led_blink_cnt = cur_motor_status; // led闪烁次数与当前电机的挡位有关
    flag_ctl_led_blink = 1;                   // 打开LED闪烁的功能
}

// 控制函数，关机
void fun_ctl_power_off(void)
{
    // alter_motor_speed(0); 关机（好像可以不写这一条）

    // 关闭灯光闪烁的动画
    flag_ctl_led_blink = 0;
    delay_ms(1); // 等待定时器处理完相应的变量
    LED_GREEN_OFF();
    LED_RED_OFF();

    // 关闭加热
    cur_ctl_heat_status = 0;

    // 关闭PWM输出：
    motor_pwm_disable();

    cur_motor_status = 0; // 表示电机已经关闭
    cur_motor_dir = 0;    // 清零，回到初始状态
    flag_is_dev_open = 0; // 表示设备已经关闭
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
        if (1 == cur_motor_status)
        {
            // 从 1档 -> 2档
            cur_motor_status = 2;
        }
        else if (2 == cur_motor_status)
        {
            // 从 2档 -> 3档
            cur_motor_status = 3;
        }
        else if (3 == cur_motor_status)
        {
            // 从 3档 -> 1档
            cur_motor_status = 1;
        }
    }
    else
    {
        // 如果是根据传参来调节挡位
        cur_motor_status = adjust_motor_status;
    }

    alter_motor_speed(cur_motor_status);

    // 每次切换挡位，都让定时器加载动画
    flag_ctl_led_blink = 0; // 打断当前正在闪烁的功能
    delay_ms(1);
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

/**
 * @brief  Main program.
 * @param  None
 * @retval None
 */
void main(void)
{
    system_init();

    // 关闭HCK和HDA的调试功能
    WDT_KEY = 0x55; // 解除写保护
    IO_MAP |= 0x03; // 关闭HCK和HDA引脚的调试功能（解除映射）
    WDT_KEY = 0xBB;

    // 初始化打印
#if USE_MY_DEBUG
    // debug_init();
    // printf("TXM8C101x_SDK main start\n");
#endif //  #if USE_MY_DEBUG

    // 初始化 检测按键 的引脚：
    // 假设用 P11 AIN9 来检测ad按键 （更换板之后是直接检测电平）
    P1_MD0 |= 0x03 << 2;   // 模拟模式
    P1_AIOEN |= 0x01 << 1; // 使能模拟功能

    heating_pin_config(); // 加热控制引脚
    led_config();
    tmr1_config();     // 1ms定时器
    tmr2_pwm_config(); // 控制升压的PWM

    adc_config();
    motor_config(); // 控制电机的两路PWM

    // 由于芯片下载之后会没有反应，这里用绿色灯作为指示：
    LED_GREEN_ON();
    delay_ms(1000);
    LED_GREEN_OFF();

#if 0  // 上电时检测电池是否正确安装:
    /*
        如果打开PWM后，检测电池的电压比满电还要高，说明没有接入电池，
        检测到的是充电5V升压后的电压
    */
    PWM2EC = 1; // 打开控制升压电路的pwm
    T2DATA = 100;
    adc_sel_channel(ADC_CHANNEL_BAT); // 切换到检测电池降压后的电压的检测引脚
    for (i = 0; i < 10; i++)          // 每55ms进入一次，循环内每次间隔约4.8ms
    {
        adc_val = adc_get_val();
        if (adc_val >= ADCDETECT_BAT_FULL + ADCDETECT_BAT_NULL_EX)
        {
            flag_bat_is_empty = 1;
            flag_bat_is_full = 1;
        }
    }

    PWM2EC = 0; // 关闭控制升压电路的pwm
    T2DATA = 0;
#endif // 上电时检测电池是否正确安装:

    while (1)
    {
#if 0  // 上电时，如果检测到电池没有安装，让LED闪烁，直到重新上电
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

        charge_scan_handle();

        if (0 == flag_is_in_charging && /* 不充电时，才对按键做检测和处理 */
            1)                          /* 不处于低电量不能开机的状态时 */
        {
            if (flag_is_enable_key_scan) // 每10ms，该标志位会被定时器置位一次
            {
                flag_is_enable_key_scan = 0;
                key_scan_10ms_isr();
            }

            key_event_handle();
        }

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

#if 1 // 根据控制标志位来控制关机
        if (flag_ctl_dev_close)
        {
            // 如果要关闭设备
            flag_ctl_dev_close = 0;
            fun_ctl_power_off();
        }
#endif // 根据控制标志位来控制关机

#if 0 // 检测设备状态，确认设备是否已经开机，并更新相应标志位
    // （好像可以不用加，自动关机检测那里已经包含了 检测电机和加热的状态）

        // if ((0 != cur_motor_status) || /* 如果电机未关闭 */
        //     (0 != cur_ctl_heat_status) /* 如果加热未关闭 */
        // )
        // {
        //     flag_is_dev_open = 1; // 表示设备开机
        // }

#endif

    } // while (1)
}

void TMR0_IRQHandler(void) interrupt TMR0_IRQn
{
    if (TMR0_CONH & 0x80)
    {
        TMR0_CONH |= 0x80;
        // 1ms产生一次中断，进入到这里

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
                    if (not_charge_ms_cnt >= 50)
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
                    if (charging_ms_cnt >= 50)
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
            static u8 __blink_cnt = 0; // 一开始存放要闪烁的次数，后面控制当前剩余闪烁次数

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
                1                             /* 当前没有在充电 */
            )
            {
                shut_down_ms_cnt++;
                if (shut_down_ms_cnt >= 600000) // xx ms 后，关机
                {
                    shut_down_ms_cnt = 0;
                    flag_ctl_dev_close = 1;
                }
            }
            else
            {
                shut_down_ms_cnt = 0;
            }
        } // 自动关机
#endif // 控制自动关机

#if 1     // 控制电机自动换方向
        { // 自动换方向

            static volatile u16 turn_dir_ms_cnt = 0; // 控制在运行时每 xx min切换一次转向的计时变量

            if ((0 != cur_motor_status) && /* 如果电机不是关闭的 */
                1                          /* 如果没有通过语音调节过方向 */
            )
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
            }

        } // 自动换方向
#endif // 控制电机自动换方向
    }
}

/**
 * @}
 */

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
