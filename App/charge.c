#include "charge.h"
#include "tx8c101x.h"

#if USE_MY_DEBUG
#include <stdio.h>
#endif

extern volatile bit flag_is_in_charging;
extern volatile bit flag_tim_scan_maybe_not_charge;
extern volatile bit flag_tim_scan_maybe_in_charging;
extern volatile bit flag_tim_set_is_in_charging;

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
    u8 i = 0;                 // 循环计数值
    u32 adc_bat_val = 0;      // 存放检测到的电池电压的adc值
    u32 adc_charging_val = 0; // 存放检测到的充电电压的adc值
    u16 tmp_bat_val = 0;      // 存放检测到的电池电压+计算的压差对应的adc值

    adc_sel_channel(ADC_CHANNEL_BAT); // 切换到检测电池降压后的电压的检测引脚
    adc_bat_val = adc_get_val_once(); // 更新电池对应的ad值

    adc_sel_channel(ADC_CHANNEL_CHARGE);   // 切换到检测充电的电压检测引脚(检测到的充电电压 == USB-C口电压 / 2)
    adc_charging_val = adc_get_val_once(); // 更新当前检测到的充电电压对应的ad值

    if (flag_is_in_charging)
    {
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

            tmr2_pwm_disable(); // 关闭PWM输出

#if USE_MY_DEBUG
            printf("uncharging\n");
#endif
        }
    } // if (flag_is_in_charging)
    else
    {
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

#if USE_MY_DEBUG
            printf("charging\n");
#endif

            tmr2_pwm_enable(); // 使能PWM输出
        }
    }

    if (flag_is_in_charging)
    {
        u16 max_pwm_val = 0;  // 临时存放最大占空比对应的值
        u16 last_pwm_val = 0; // 记录之前的pwm占空比的值
        u16 tmp_val = 0;      // 临时存放需要调节的占空比对应的值
        static u16 tmp_val_l[8] = {0};
        static u8 tmp_val_cnt = 0;

        last_pwm_val = TMR2_PWML + ((u16)TMR2_PWMH << 7);  // 读出上一次PWM占空比对应的值
        max_pwm_val = TMR2_PRL + ((u16)TMR2_PRH << 7) + 1; // 读出PWM占空比设定的、最大的值
        // printf("max_pwm_val %lu\n", max_pwm_val);

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

        if (adc_bat_val <= 2837) // 如果检测电池电压小于 6.5V
        {
            tmp_bat_val = (adc_bat_val + 37);
        }
        else if (adc_bat_val <= 3056) // 如果检测电池电压小于 7.0V
        {
            tmp_bat_val = (adc_bat_val + 27);
        }
        else if (adc_bat_val <= 3188) // 如果检测电池电压小于 7.3V
        {
            tmp_bat_val = (adc_bat_val + 16);
        }
        else if (adc_bat_val <= 3326) // 如果检测电池电压小于 7.62V
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

        tmp_bat_val += 95; //

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
            last_pwm_val = last_pwm_val + 1;
        }
        else if (tmp_val < last_pwm_val)
        {
            last_pwm_val = last_pwm_val - 1;
        }
 
        TMR2_PWML = last_pwm_val % 256;
        TMR2_PWMH = last_pwm_val / 256; 
    } // if (flag_is_in_charging)
}
