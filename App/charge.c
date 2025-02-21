#include "charge.h"
#include "tx8c101x.h"

#if USE_MY_DEBUG
#include <stdio.h>
#endif

extern volatile bit flag_is_in_charging;
extern volatile bit flag_bat_is_full;      // 电池是否被充满电的标志位
extern volatile bit flag_bat_is_near_full; // 标志位，表示电池是否快充满电
extern volatile bit flag_ctl_dev_close;    // 控制标志位，是否要关闭设备

extern volatile bit flag_tim_scan_maybe_not_charge;
extern volatile bit flag_tim_scan_maybe_in_charging;
extern volatile bit flag_tim_set_is_in_charging;

extern volatile bit flag_tim_scan_bat_maybe_near_full; // 用于给定时器扫描的标志位，可能检测到电池快被充满电
extern volatile bit flag_tim_set_bat_is_near_full;     // 由定时器置位/复位的，表示电池是否快被充满电的标志位

extern volatile bit flag_tim_scan_bat_maybe_full; // 用于给定时器扫描的标志位，可能检测到电池被充满电
extern volatile bit flag_tim_set_bat_is_full;     // 由定时器置位/复位的，表示电池是否被充满电的标志位

// 用于给定时器扫描的标志位，可能检测到了低电量
// 会在开机工作时 更新该标志位的状态，在充电时清除标志位
extern volatile bit flag_tim_scan_maybe_low_bat;
extern volatile bit flag_tim_set_bat_is_low; // 由定时器置位/复位的，表示在工作时，电池是否处于低电量的标志位
extern volatile bit flag_ctl_low_bat_alarm;  // 控制标志位，是否使能低电量报警

extern volatile bit flag_tim_scan_maybe_shut_down; // 用于给定时器扫描的标志位，可能检测到了 电池电压 低于 关机对应的电压
extern volatile bit flag_tim_set_shut_down;        // 由定时器置位/复位的，表示在工作时，检测到了 电池电压 在一段时间内都低于 关机对应的电压

extern volatile bit flag_is_disable_to_open; // 标志位，是否不使能开机(低电量不允许开机)

extern volatile bit flag_is_enter_low_power; // 标志位，是否要进入低功耗

// 充电扫描与检测
/**
 * @brief    充电、电池的扫描和相关处理
             如果一上电检测到电池为空，不能调用该函数
             除了定时器，可供外部检测的标志位：
                flag_is_in_charging
                flag_bat_is_full
 */
void charge_scan_handle(void)
{
    u16 adc_bat_val = 0;             // 存放检测到的电池电压的adc值
    u16 adc_charging_val = 0;        // 存放检测到的充电电压的adc值
    u16 tmp_bat_val = 0;             // 存放检测到的电池电压+计算的压差对应的adc值
    static u8 over_charging_cnt = 0; // 存放过充计数

    adc_sel_channel(ADC_CHANNEL_BAT); // 切换到检测电池降压后的电压的检测引脚
    adc_bat_val = adc_get_val(); // 更新电池对应的ad值

    adc_sel_channel(ADC_CHANNEL_CHARGE);   // 切换到检测充电的电压检测引脚(检测到的充电电压 == USB-C口电压 / 2)
    adc_charging_val = adc_get_val(); // 更新当前检测到的充电电压对应的ad值

    // printf("bat val %u\n", adc_bat_val);
    // printf("charge val %u\n", adc_charging_val);

    // 低电量不允许开机
    if (adc_bat_val < LOW_BAT_ALARM_AD_VAL)
    {
        flag_is_disable_to_open = 1;
    }
    else
    {
        flag_is_disable_to_open = 0;
    }

    if (flag_is_in_charging)
    {
        // 如果正在充电，检测电池是否充满电

#if 1 // 检测在充电时，电池是否充满电，并做相应的处理

        if (0 == flag_bat_is_full)
        {
            // 如果正在充电，且电池未充满电，检测电视是否快充满电 或是 电池充满电

            // 如果检测到充满电（可能触发了电池保护板的过充保护），直接输出0%的PWM
            if (adc_bat_val >= ADCDETECT_BAT_FULL + ADCDETECT_BAT_NULL_EX)
            {
                // 如果检测到的ad值比 满电的ad值 还要多
                // PWM占空比设置为0，让占空比重新开始递增，电流从零开始逐渐增大
                TMR2_PWML = 0; // 占空比 0%
                TMR2_PWMH = 0;
                over_charging_cnt++; // 过充检测计数加一
                // flag_tim_scan_bat_maybe_full = 1; // (可以不用给这个标志位置一，这里只是测试时使用)
            }
            else if (adc_bat_val >= ADCDETECT_BAT_FULL) // 检测电池是否满电
            {
                // 给对应的标志位置一，让定时器来检测是否持续一段时间都是满电
                flag_tim_scan_bat_maybe_full = 1;
            }
            else if (adc_bat_val >= ADCVAL_NEAR_FULL_CHAGE) // 检测到有一次电池快充满电
            {
                // 如果电池快充满电
                flag_tim_scan_bat_maybe_near_full = 1; // 表示电池快充满电
            }
            else
            {
                // 如果检测到的ad值小于满电阈值
                // 清空对应的标志位，让定时器不检测是否满电
                flag_tim_scan_bat_maybe_full = 0;
                flag_tim_scan_bat_maybe_near_full = 0;
            }

            if (flag_tim_set_bat_is_near_full && 0 == flag_bat_is_near_full)
            {
                // 如果电池接近满电，关闭充电时的呼吸灯，点亮绿灯，但是不关闭控制充电的PWM
                flag_bat_is_near_full = 1;
                // delay_ms(1);    // 可能要等待定时器关闭充电时闪烁的呼吸灯
                LED_RED_OFF();  // 关闭充电时闪烁的呼吸灯
                LED_GREEN_ON(); // 充满电时，让绿灯常亮
            }

            if (flag_tim_set_bat_is_full || (over_charging_cnt >= 8))
            // if (flag_tim_set_bat_is_full) // 测试时使用到的条件
            {
                // 如果定时器检测了一段时间(5s)，都是充满电的状态，或着是累计有过充，说明电池充满电
                over_charging_cnt = 0; // 清除过充计数
                flag_bat_is_full = 1;  // 表示电池被充满电
                tmr2_pwm_disable();    // 关闭控制升压电路的pwm
                TMR2_PWML = 0;         // 占空比 0%
                TMR2_PWMH = 0;         //
                // flag_is_in_charging = 0; // 不能给这个标志位清零（交给充电扫描来清零）
                // delay_ms(1);    // 可能要等待定时器关闭充电时闪烁的呼吸灯
                LED_RED_OFF();  // 关闭充电时闪烁的呼吸灯
                LED_GREEN_ON(); // 充满电时，让绿灯常亮
            }
        } // if (0 == flag_bat_is_full)

#endif // 检测在充电时，电池是否充满电，并做相应的处理

#if 1 // (这个功能要放在该语句块的最后)检测在充电时，是否拔出了充电线，并做相应的处理
      // 如果正在充电，检测是否拔出了充电线
        if (adc_charging_val < ADCDETECT_CHARING_THRESHOLD)
        {
            // 给对应的标志位置一，如果连续 50 ms都是这个状态，说明拔出了充电器
            flag_tim_scan_maybe_not_charge = 1;
        }
        else
        {
            flag_tim_scan_maybe_not_charge = 0;
        }

        if (0 == flag_tim_set_is_in_charging) // 如果定时器连续 50 ms 都是检测到拔出了充电器
        {
            // 如果在充电时，检测到拔出了充电线
            flag_is_in_charging = 0;

            // 清空充电时使用的标志位和变量：
            flag_tim_scan_bat_maybe_full = 0;
            flag_tim_scan_bat_maybe_near_full = 0;
            // flag_tim_set_bat_is_full = 0; // 可以不用清零这个变量，定时器后续会自动清零
            flag_bat_is_near_full = 0;
            flag_bat_is_full = 0;
            over_charging_cnt = 0; // 清除过充计数

            tmr2_pwm_disable(); // 关闭控制充电的PWM输出
            delay_ms(1); // 等待定时器清空相应的变量和标志位
            LED_GREEN_OFF();
            LED_RED_OFF();

#if USE_MY_DEBUG
            printf("uncharging\n");
#endif // #if USE_MY_DEBUG

            // return; // 退出函数，防止再执行下面的功能
        }
#endif // 检测在充电时，是否拔出了充电线，并做相应的处理

    } // if (flag_is_in_charging)
    else // 如果不在充电
    {

#if 1 // 在设备工作时，检测是否处于低电量，并进行相应处理

        if (0 != cur_motor_status || 0 != cur_ctl_heat_status)
        {
            // 如果设备在工作

            if (adc_bat_val <= SHUT_DOWN_BAT_AD_VAL)
            {
                // 如果电池电压 小于等于 关机对应的电压
                flag_tim_scan_maybe_shut_down = 1;
                flag_tim_scan_maybe_low_bat = 0; // 表示不处于低电压，而是处于关机电压，让定时器只执行关机电压的连续检测部分
            }
            else if (adc_bat_val <= LOW_BAT_ALARM_AD_VAL)
            // if (adc_bat_val <= LOW_BAT_ALARM_AD_VAL) // 还没有添加低电量关机功能时，用于测试低电量报警的功能
            {
                // 如果电池电压 小于等于 低电量报警对应的电压
                flag_tim_scan_maybe_low_bat = 1;
                flag_tim_scan_maybe_shut_down = 0; // 当前电池电压正处于 关机电压 ~ 低电量之间，还没到要关机的情况
            }
            else
            {
                // 如果电池电压 大于 低电量报警对应的电压
                flag_tim_scan_maybe_low_bat = 0;
                flag_tim_scan_maybe_shut_down = 0;
            }

            // 如果连续一段时间检测到电池电压低于关机电压
            if (flag_tim_set_shut_down)
            {
                flag_ctl_dev_close = 1;
                // SPEECH_POWER_DISABLE(); // 关闭语音IC的电源
                flag_is_enter_low_power = 1; // 允许进入低功耗
            }
            else if (flag_tim_set_bat_is_low && 0 == flag_ctl_low_bat_alarm)
            // if (flag_tim_set_bat_is_low && 0 == flag_ctl_low_bat_alarm) // 还没有添加低电量关机功能时，用于测试低电量报警的功能
            {
                // 如果连续一段时间检测到电池电压处于低电量，并且没有打开低电量报警
                LED_GREEN_OFF();
                LED_RED_OFF();
                flag_ctl_low_bat_alarm = 1; // 使能低电量报警
            }
        }
#endif // 在设备工作时，检测是否处于低电量，并进行相应处理

#if 1 // 检测不在充电时，是否有插入充电线，并做相应的处理
      // 如果不在充电，检测是否插入了充电线
        if (adc_charging_val >= ADCDETECT_CHARING_THRESHOLD)
        {
            // 给对应的标志位置一，如果累计 50 ms 都是这个状态，说明插入了充电器
            flag_tim_scan_maybe_in_charging = 1;
        }
        else
        {
            flag_tim_scan_maybe_in_charging = 0;
        }

        if (flag_tim_set_is_in_charging) // 如果定时器累计 50 ms都是检测到插入了充电器
        {
            // 确认是插入充电线后，无论处于什么状态，都变为关机状态
            flag_is_in_charging = 1;

            // 清空不充电时使用的变量和标志位
            flag_tim_scan_maybe_low_bat = 0; // 表示不处于低电量
            flag_ctl_low_bat_alarm = 0;      // 关闭低电量报警

#if USE_MY_DEBUG
            printf("charging\n");
#endif // #if USE_MY_DEBUG

            tmr2_pwm_enable(); // 使能PWM输出
            flag_ctl_dev_close = 1;      // 控制标志位置一，让主函数扫描到，并关机
            flag_is_enter_low_power = 0; // 不进入低功耗

            LED_GREEN_OFF(); // 关闭绿灯(如果等到主循环扫描到再关闭绿灯，第1ms会出现红灯和绿灯一起点亮的情况)
        }
#endif // 检测不在充电时，是否有插入充电线，并做相应的处理
    }

#if 1  // 充电电流控制
 
    // if (flag_is_in_charging && 0 == flag_bat_is_full)
    if (flag_is_in_charging)
    {
        u8 i = 0;             // 循环计数值
        u8 max_pwm_val = 0;  // 临时存放最大占空比对应的值 
        u8 last_pwm_val = 0; // 记录之前的pwm占空比的值
        u16 tmp_val = 0;      // 临时存放需要调节的占空比对应的值 
        static u16 tmp_val_l[8] = {0};
        static u8 tmp_val_cnt = 0;

        // last_pwm_val = (u16)TMR2_PWML + ((u16)TMR2_PWMH << 7); // 读出上一次PWM占空比对应的值
        // max_pwm_val = TMR2_PRL + ((u16)TMR2_PRH << 7) + 1;     // 读出PWM占空比设定的、最大的值
        last_pwm_val = TMR2_PWML;
        max_pwm_val = TMR2_PRL + 1;
 

        /*
            修改电压差值，电压差值 = 203 - (adc_bat_val * 122 / 1000)

            推导过程：
            在充电时测得，充电电流1.1A左右，压差为-30(ad值)时，电池一侧电压为7.8V(ad值：1917)
                         充电电流1.1A左右，压差为0(ad值)时，电池一侧电压为6.8V(ad值：1671)
            假设x轴为电压对应的ad值，y轴为压差对应的ad值，建立参考坐标系
            根据这两个测试点，发现x轴正向增长，y轴负向增长，画出的斜线向下，斜率为负，求出斜率
                k = Δy/Δx = (0 - 30) / (1917 - 1671)，约为 -0.122
            建立公式：y = kx + b，代入，解得 b 约为 203 （四舍五入是204）
            y = kx + b ==> 压差 = -0.122 * 充电时的电池电压 + 203
            转换成单片机可以计算的形式：压差 = 203 - (充电时的电池电压 * 122 / 1000)
        */

        if (adc_bat_val <= 2752) // 如果检测电池电压小于 6.5V
        {
            tmp_bat_val = (adc_bat_val + 37);
        }
        else if (adc_bat_val <= 2964) // 如果检测电池电压小于 7.0V
        {
            tmp_bat_val = (adc_bat_val + 27);
        }
        else if (adc_bat_val <= 3091) // 如果检测电池电压小于 7.3V
        {
            tmp_bat_val = (adc_bat_val + 16);
        }
        else if (adc_bat_val <= 3227) // 如果检测电池电压小于 7.62V
        {
            tmp_bat_val = (adc_bat_val + 0);
        }
        else // 如果在充电时检测到电池电压大于
        {
            // 如果检测电池的分压电阻是 22K / 100K，1.2-1.3A,最常见是在1.22A、1.26A
            // 如果检测电池的分压电阻是 220K / 1M，充电电流在0.9A-1A
            // tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 294);
            tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 522);
        }

        // tmp_bat_val += 95; // 1.8A
        tmp_bat_val += 30;

        // for (i = 0; i < ARRAY_SIZE(bat_val_fix_table); i++)
        // {
        //     if (adc_bat_val <= bat_val_fix_table[i].adc_bat_val)
        //     {
        //         tmp_bat_val = (adc_bat_val + bat_val_fix_table[i].tmp_bat_val_fix);
        //         break;
        //     }

        //     if (i == (ARRAY_SIZE(bat_val_fix_table) - 1))
        //     {
        //         tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 522) + TMP_BAT_VAL_FIX;
        //     }
        // }

        /*
            升压公式：Vo = Vi / (1 - D)

            通过PWM来控制升压，这里假设当前PWM占空比寄存器的值 为 D，PWM占空比寄存器可以到的最大的值 为 1
            Vo = Vi / (PWM占空比寄存器可以到的最大的值 - 当前PWM占空比寄存器的值)
            当前PWM占空比越大，Vo也越大，充电的电流也会越大

            (PWM占空比寄存器可以到的最大的值 - 当前PWM占空比寄存器的值) = Vi / Vo
            当前PWM占空比寄存器的值 = PWM占空比寄存器可以到的最大的值 - Vi / Vo

            这里检测到的充电电压的ad值 == USB-C口电压 / 2[上下拉电阻分压] / 参考电压[3V，那么这里就是除以3] * 4096[ad转换精度，12位-->0~4096]
            即，这里检测到的充电电压的ad值 == USB-C口电压 / 2 / 3 * 4096
            检测到的电池电压的ad值 == 电池电压 * 0.18 / 3V参考电压 * 4096 == 电池电压 * 220 / 1220 / 3V参考电压 * 4096
            (电池的分压电阻： 上拉220K，下拉1M，分压系数： 220 / 1220)

            检测充电电压和检测电池电压使用的不是同一个分压系数，要一起运算时，这里将充电电压的ad再 * 2 * 220 / 1220
            即 (adc_charging_val * 22 / 61)

            再代回公式：当前PWM占空比寄存器的值 = PWM占空比寄存器可以到的最大的值 - Vi / Vo
            当前PWM占空比寄存器的值 = PWM占空比寄存器可以到的最大的值 - 充电电压 / 充电时电池两侧的电压
            tmp_val = max_pwm_val - 充电电压 / 充电时电池两侧的电压
            转换成单片机可以计算的形式：
            tmp_val = max_pwm_val - (adc_charging_val * 22 / 61) / tmp_bat_val，但是 max_pwm_val 的值不是1，不符合 Vo = Vi / (1 - D)
            这里要改成 tmp_val = max_pwm_val - max_pwm_val * (adc_charging_val * 22 / 61) / tmp_bat_val
            tmp_val = max_pwm_val - (adc_charging_val * max_pwm_val * 22 / 61) / tmp_bat_val
        */
        // D = 1 - (Vi / Vo)
        // tmp_val = max_pwm_val - (adc_charging_val * max_pwm_val * 22 / 61) / tmp_bat_val;
        tmp_val = max_pwm_val - ((u32)adc_charging_val * max_pwm_val * 94 / 147) / tmp_bat_val;

        if (tmp_val >= max_pwm_val)
        {
            // 如果PWM占空比对应的值 大于 最大占空比对应的值，说明计算溢出（可能是电池电压过小），按0处理
            tmp_val = 0;
        }

        // 滤波操作，一开始tmp_val会很小，采集多次后趋于一个平均值：
        tmp_val_cnt++;
        tmp_val_cnt &= 0x07;
        tmp_val_l[tmp_val_cnt] = (tmp_val_l[tmp_val_cnt] + tmp_val) >> 1;
        tmp_val = 0;
        for (i = 0; i < 8; i++)
        {
            tmp_val += tmp_val_l[i];
        }
        tmp_val >>= 3;

        if (tmp_val > last_pwm_val)
        {
            // last_pwm_val = last_pwm_val + 1;
            last_pwm_val++;
        }
        else if (tmp_val < last_pwm_val)
        {
            // last_pwm_val = last_pwm_val - 1;
            last_pwm_val--;
        }

        TMR2_PWML = last_pwm_val % 256;
        // TMR2_PWMH = last_pwm_val / 256;  // 最大占空比的值不超过255，为了节省程序空间，可以不用配置高8位的寄存器
        TMR2_PWMH = 0;

        // 充电时，每隔一段时间再调整一次PWM占空比，否则充电电流跳动会很厉害
        delay_ms(100);
    } // if (flag_is_in_charging)
#endif // 充电电流控制
}
