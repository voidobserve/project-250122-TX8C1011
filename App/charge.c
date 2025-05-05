#include "charge.h"
#include "tx8c101x.h"

#if USE_MY_DEBUG
#include <stdio.h>
#endif

extern volatile bit flag_is_in_charging;
extern volatile bit flag_bat_is_full;      // 电池是否被充满电的标志位
extern volatile bit flag_bat_is_near_full; // 标志位，表示电池是否快充满电
extern volatile bit flag_ctl_dev_close;    // 控制标志位，是否要关闭设备
extern volatile bit flag_ctl_led_blink;    // 控制标志位，是否控制指示灯闪烁

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

// extern volatile bit flag_tim_scan_maybe_shut_down; // 用于给定时器扫描的标志位，可能检测到了 电池电压 低于 关机对应的电压
// extern volatile bit flag_tim_set_shut_down;        // 由定时器置位/复位的，表示在工作时，检测到了 电池电压 在一段时间内都低于 关机对应的电压

extern volatile bit flag_is_disable_to_open; // 标志位，是否不使能开机(低电量不允许开机)

extern volatile bit flag_is_enter_low_power; // 标志位，是否要进入低功耗

extern volatile bit flag_tim_scan_maybe_motor_stalling; // 用于给定时器扫描的标志位，可能检测到了电机堵转

extern volatile bit flag_is_adjust_current_time_comes;

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
    volatile u16 adc_bat_val = 0;      // 存放检测到的电池电压的adc值
    volatile u16 adc_charging_val = 0; // 存放检测到的充电电压的adc值
    static u8 over_charging_cnt = 0;   // 存放过充计数

    adc_sel_channel(ADC_CHANNEL_BAT); // 切换到检测电池降压后的电压的检测引脚
    adc_bat_val = adc_get_val();      // 更新电池对应的ad值

    adc_sel_channel(ADC_CHANNEL_CHARGE); // 切换到检测充电的电压检测引脚(检测到的充电电压 == USB-C口电压 / 2)
    adc_charging_val = adc_get_val();    // 更新当前检测到的充电电压对应的ad值

    // printf("bat val %u\n", adc_bat_val);
    // printf("charge val %u\n", adc_charging_val);

    // 低电量不允许开机
    if (adc_bat_val < LOW_BAT_ALARM_AD_VAL)
    {
        flag_is_disable_to_open = 1;
    }
    // else if (adc_bat_val > (LOW_BAT_ALARM_AD_VAL + 15))
    else
    {
        flag_is_disable_to_open = 0;
        flag_tim_scan_maybe_low_bat = 0;
    }

    if (flag_is_in_charging)
    {
        // 如果正在充电，检测电池是否充满电

#if 1 // 检测在充电时，电池是否充满电，并做相应的处理

        if (0 == flag_bat_is_full)
        {
            // 如果正在充电，且电池未充满电，检测电视是否快充满电 或是 电池充满电

            // 如果检测到充满电（可能触发了电池保护板的过充保护），直接输出0%的PWM
            if (adc_bat_val >= (ADCDETECT_BAT_FULL + ADCDETECT_BAT_NULL_EX))
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
                delay_ms(1);    // 必须要等待定时器关闭充电时闪烁的呼吸灯
                LED_RED_OFF();  // 关闭充电时闪烁的呼吸灯
                LED_GREEN_ON(); // 快充满电时，让绿灯常亮
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
            // 给对应的标志位置一，如果连续 xx ms都是这个状态，说明拔出了充电器
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

            flag_is_enter_low_power = 1; // 允许进入低功耗

            // 可以交给低功耗函数来关闭PWM：
            // tmr2_pwm_disable(); // 关闭控制充电的PWM输出
            delay_ms(1); // 等待定时器清空相应的变量和标志位
                         // 可以交给低功耗函数来关灯：
                         // LED_GREEN_OFF();
                         // LED_RED_OFF();

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

        // if (0 != cur_motor_status || 0 != cur_ctl_heat_status)
        if (SPEECH_CTL_PIN_OPEN == SPEECH_CTL_PIN) /* 如果语音IC还在工作，说明没有进入低功耗 */
        {
            // 如果设备在工作
            if (adc_bat_val <= LOW_BAT_ALARM_AD_VAL)
            {
                // 如果电池电压 小于等于 低电量报警对应的电压
                flag_tim_scan_maybe_low_bat = 1;
                // flag_tim_scan_maybe_shut_down = 0; // 当前电池电压正处于 关机电压 ~ 低电量之间，还没到要关机的情况
            }
            else
            {
                // 如果电池电压 大于 低电量报警对应的电压
                flag_tim_scan_maybe_low_bat = 0;
                // flag_tim_scan_maybe_shut_down = 0;
            }

            // 如果连续一段时间检测到电池电压低于关机电压
            // if (flag_tim_set_shut_down)
            // {
            //     flag_ctl_dev_close = 1;
            //     // SPEECH_POWER_DISABLE(); // 关闭语音IC的电源(在低功耗函数内关闭)
            //     flag_is_enter_low_power = 1; // 允许进入低功耗
            // }
            // else if (flag_tim_set_bat_is_low && 0 == flag_ctl_low_bat_alarm)
            if (flag_tim_set_bat_is_low &&
                0 == flag_ctl_low_bat_alarm &&
                0 == flag_is_enter_low_power &&
                0 == flag_tim_scan_maybe_motor_stalling) /* 如果电机未堵转 */
            {
                // 如果连续一段时间检测到电池电压处于低电量，并且没有打开低电量报警
                interrupt_led_blink(); // 关闭LED闪烁
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

            // 在测试时关闭
            tmr2_pwm_enable();           // 使能PWM输出
            flag_ctl_dev_close = 1;      // 控制标志位置一，让主函数扫描到，并关机
            flag_is_enter_low_power = 0; // 不进入低功耗

            // flag_ctl_led_blink = 0; // 打断当前的灯光闪烁效果
            // delay_ms(1);
            interrupt_led_blink();
            LED_GREEN_OFF(); // 关闭绿灯(如果等到主循环扫描到再关闭绿灯，第1ms会出现红灯和绿灯一起点亮的情况)
        }
#endif // 检测不在充电时，是否有插入充电线，并做相应的处理
    }

#if 1 // 充电电流控制

    // if (flag_is_in_charging && 0 == flag_bat_is_full)
    if (flag_is_in_charging)
    {
#if 0 // 使用计算的方式来调节充电电流

        u8 i = 0;                             // 循环计数值
        const u8 max_pwm_val = TMR2_PRL + 1;  // 临时存放最大占空比对应的值
        volatile u8 last_pwm_val = TMR2_PWML; // 记录之前的pwm占空比的值
        volatile u16 tmp_val;                 // 临时存放需要调节的占空比对应的值
        volatile u16 tmp_bat_val;             // 存放检测到的电池电压+计算的压差对应的adc值(可以不初始化)
        static volatile u16 tmp_val_l[8] = {0};
        static volatile u8 tmp_val_cnt = 0;

        tmp_bat_val = adc_bat_val;
// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000);
// tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 800 / 1000);
// tmp_bat_val += 2300;
#if 1
        if (adc_bat_val <= 2752) // 如果检测电池电压小于 6.5V（实际测试，在这个阶段充电速度会很快，测不出稳定充电时的电流）
        {
            // tmp_bat_val = (adc_bat_val + (37)); /*  */
            // tmp_bat_val += ((37 + 25)); /*  */
            // tmp_bat_val += ((37 + 35)); /*  */
            tmp_bat_val += ((80)); /*  */
            // tmp_bat_val += ((37 + 50)); /*   */
            // tmp_bat_val += ((37 + 70)); /*  */
            // tmp_bat_val = (adc_bat_val + (37 + 40)); /* 6.40--1.07， */
        }
        else if (adc_bat_val <= 2837) // 如果检测电池电压小于 6.7V
        {
            // tmp_bat_val = (adc_bat_val + (40)); /*  6.6--0.93 */
            // tmp_bat_val += (20);
            // tmp_bat_val += (25);
            tmp_bat_val += (60); /*  */
            // tmp_bat_val += (70); /*  */
            // tmp_bat_val += (80); /*  */
            // tmp_bat_val += (90); /*  */
            // tmp_bat_val += (100); /*  */
            // tmp_bat_val += (30);
            // tmp_bat_val += (40);
        }
        else if (adc_bat_val <= 2964) // 如果检测电池电压小于 7.0V
        {
            // tmp_bat_val -= 150;
            // tmp_bat_val = (adc_bat_val + (40)); /*  6.6--0.93 */
            tmp_bat_val += (20); /*  */
            // tmp_bat_val += (40); /*  */

            // tmp_bat_val += (200); /*  */
            // tmp_bat_val += (400); /*  */
            // tmp_bat_val += (480); /*  */
            // tmp_bat_val += (500); /*  */
            // tmp_bat_val += (600); /*  */
        }
        else if (adc_bat_val <= 3091) // 如果坚持电池电压小于 7.3V
        {
            // tmp_bat_val = (adc_bat_val + 16);
            // tmp_bat_val += (10);
            // tmp_bat_val -= (5);
            // tmp_bat_val -= (10); /* 到了7.2V--1.3A */
            // tmp_bat_val -= (15);
            // tmp_bat_val -= (20);
            tmp_bat_val -= 25; /* 7.23--1.16 */
            // tmp_bat_val -= (30); /* 7.17--0.97 */
        }
#endif
        // else if (adc_bat_val <= 3227) // 如果检测电池电压小于 7.62V
        // else
        // {
        //     // tmp_bat_val = (adc_bat_val + (0));
        //     // tmp_bat_val += (0);
        //     // tmp_bat_val -= 60;

        //     // tmp_bat_val -=25;
        //     // u16 tmp_u16 = (adc_bat_val - 3091) / 2;
        //     // for (i = 0; i < tmp_u16; i++)
        //     // {
        //     //     tmp_bat_val--;
        //     // }
        //     #if 0
        //     // 电池电压越大，需要修改的补偿值越大，通过循环来处理
        //     u16 tmp_u16 = (adc_bat_val - 3091) / 2;
        //     for (i = 0; i < tmp_u16; i++)
        //     {
        //         // tmp_bat_val -= 1;
        //         tmp_bat_val -= 2;
        //         // tmp_bat_val -= 3;// 到后面电池电压越高，电流越小
        //         // tmp_bat_val -= 4; // 到后面电池电压越高，电流越小
        //     }

        //     // tmp_bat_val += 40;
        //     // tmp_bat_val += 80;
        //     // tmp_bat_val += 120;
        //     // tmp_bat_val += 130; // 0.8-0.9A
        //     // tmp_bat_val += 140;
        //     // tmp_bat_val += 145;
        //     tmp_bat_val += 150;
        //     // tmp_bat_val += 160;
        //     #endif

        /*
            实际测试，这里电压升高后，电流也会快速增加:
        */
        //     // tmp_bat_val -= -0.762(adc_bat_val - 3091) + 79;
        //     u16 tmp = (u32)(adc_bat_val - 3091) * 381 / 500;
        //     if (tmp >= 79) // 如果adc_bat_val大于等于3195，电池一侧电压至少大于7.54
        //     {
        //         tmp_bat_val -= (tmp - 79); /*  */
        //         // tmp_bat_val += (tmp - 79); /* 这里的计算还有问题，电流会到2.3A */
        //     }
        //     else
        //     {
        //         tmp_bat_val -= (79 - tmp) ;
        //     }
        // }
        else // 如果在充电时检测到电池电压大于
        {
            // tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 522);
            // tmp_bat_val -= ((u32)adc_bat_val * 157 / 1000 - 522);
            // tmp_bat_val -= ((u32)adc_bat_val * 157 / 1000 - 500);
            // tmp_bat_val -= ((u32)adc_bat_val * 157 / 1000 - 480);
            // tmp_bat_val -= ((u32)adc_bat_val * 157 / 1000 - 480);
            // tmp_bat_val += 100; // 7.45左右，0.51A
            // tmp_bat_val += 160; //  7.45--0.573

            // tmp_bat_val += 200; //
            // tmp_bat_val -= ((u32)adc_bat_val * 360 / 1000);

            // tmp_bat_val -= 45;
            // tmp_bat_val -= 55;

            u16 tmp = adc_bat_val - 3091;
            // tmp_bat_val += 150;
            // tmp_bat_val += 200;
            tmp_bat_val += 225;
            // tmp_bat_val += 250;
            // tmp_bat_val += 300;
            while (1)
            {
                // if (tmp > 3)
                // {
                //     // ad值每变化3就补偿5
                //     tmp -= 3;
                //     tmp_bat_val -= 5;
                // }
                if (tmp > 4)
                {
                    tmp -= 4;
                    tmp_bat_val -= 5;
                }
                else
                {
                    break;
                }
            }

            // tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 522) + (15); /* 实际测试，在最后减去215的补偿下， */
        }

        /* tmp_bat_val越大，最终充电电流也会越大，这里要对它做补偿，tmp_bat_val越大，补偿也要相应增大 */
        // tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 800 / 1000);
        // tmp_bat_val += 15;
        // tmp_bat_val += 25;
        // tmp_bat_val += 50;
        // tmp_bat_val += 100;
        // tmp_bat_val += 115;
        // tmp_bat_val += 200;
        // tmp_bat_val += 350;
        // tmp_bat_val += 400;
        // tmp_bat_val += 500;
        // tmp_bat_val += 600;
        // tmp_bat_val += 800;
        // tmp_bat_val += 1000;
        // tmp_bat_val += 1100;
        // tmp_bat_val += 1200;
        // tmp_bat_val += 2000;
        // tmp_bat_val += 2100;
        // tmp_bat_val += 2200;
        // tmp_bat_val += 2300;
        // tmp_bat_val += 2500;

#if 1  // 补偿开关
        {
            // u16 i;
            // for (i = 0; i < 50; i++) //
            // for (i = 0; i < 80; i++) //
            for (i = 0; i < 120; i++) // ===================
            // for (i = 0; i < 125; i++) //
            // for (i = 0; i < 150; i++) //
            // for (i = 0; i < 300; i++) //
            // for (i = 0; i < 1000; i++) //
            {
                if (tmp_bat_val > 2) // tmp_bat_val不能等于0
                {
                    tmp_bat_val--;
                }
            }
        }
#endif // 补偿开关

        if (adc_bat_val >= 3472) // 大于8.2V,将电流调节至500-600mA
        {
            // tmp_bat_val -= 20;
        }

        tmp_val = max_pwm_val - ((u32)adc_charging_val * max_pwm_val * 94 / 147) / tmp_bat_val;

        // 重新计算了分压系数的公式（实际测试发现电流会比未重新计算的，电流还要大，斜率也更大）：
        // tmp_val = max_pwm_val - ((u32)adc_charging_val * max_pwm_val * 165 / 266) / tmp_bat_val;

        if (tmp_val >= max_pwm_val)
        {
            // 如果PWM占空比对应的值 大于 最大占空比对应的值，说明计算溢出（可能是电池电压过小），按0处理
            tmp_val = 0;
        }

        // 滤波操作，一开始tmp_val会很小，采集多次后趋于一个平均值：
        // 如果不滤波，充电电流会频繁跳动
        tmp_val_cnt++;
        tmp_val_cnt &= 0x07;
        tmp_val_l[tmp_val_cnt] = (tmp_val_l[tmp_val_cnt] + tmp_val) >> 1;
        tmp_val = 0;
        for (i = 0; i < 8; i++)
        {
            tmp_val += tmp_val_l[i];
        }
        tmp_val >>= 3;

        if ((u8)tmp_val > last_pwm_val)
        {
            // last_pwm_val = last_pwm_val + 1;
            last_pwm_val++;
        }
        else if ((u8)tmp_val < last_pwm_val)
        // else // 不能直接用else，如果减到0还在继续减小，会导致数据溢出
        {
            // last_pwm_val = last_pwm_val - 1;
            if (last_pwm_val > 1) // 防止减到0还在继续减，导致溢出
                last_pwm_val--;
        }

        TMR2_PWML = last_pwm_val % 256;
        // TMR2_PWMH = last_pwm_val / 256;  // 最大占空比的值不超过255，为了节省程序空间，可以不用配置高8位的寄存器
        TMR2_PWMH = 0;

        // 充电时，每隔一段时间再调整一次PWM占空比，否则充电电流跳动会很厉害
        // delay_ms(100);

#endif // 使用计算的方式来调节充电电流

#if 1
        u8 i = 0;
        u8 last_pwm_val = TMR2_PWML;     // 读出上一次PWM占空比对应的值
        const u8 max_pwm_val = TMR2_PRL; // 读出PWM占空比设定的、最大的值
        u16 tmp_bat_val = adc_bat_val;
        static u8 tmp_val_cnt = 0;
        u32 tmp_val;
        static u8 tmp_val_l[8] = 0; // 用来存放计算得到的pwm占空比值，最后再一起计算平均值

        // last_pwm_val = TMR2_PWML + ((u16)TMR2_PWMH << 7);  // 读出上一次PWM占空比对应的值
        // max_pwm_val = TMR2_PRL + ((u16)TMR2_PRH << 7) + 1; // 读出PWM占空比设定的、最大的值

        if (adc_bat_val <= 2752) // 如果检测电池电压小于 6.5V
        {
            // tmp_bat_val = (adc_bat_val + 37);
            tmp_bat_val += 37; // 0.8升到0.93
        }
        else if (adc_bat_val <= 2964) // 如果检测电池电压小于 7.0V
        {
            // tmp_bat_val = (adc_bat_val + 27);
            tmp_bat_val += 27; // +=27，最后再+=95，只有0.22A
            // tmp_bat_val += 127;
        }
        else if (adc_bat_val <= 3091) // 如果检测电池电压小于 7.3V
        {
            // tmp_bat_val = (adc_bat_val + 16);
            tmp_bat_val += 16;
        }
        else if (adc_bat_val <= 3227) // 如果检测电池电压小于 7.62V
        {
            // tmp_bat_val = (adc_bat_val + 0);
        }
        else // 如果在充电时检测到电池电压大于
        {
            // tmp_bat_val = (u32)adc_bat_val - ((u32)adc_bat_val * 157 / 1000 - 522);
            tmp_bat_val -= ((u32)adc_bat_val * 157 / 1000 - 506);
        }

        // tmp_bat_val += 55; //
        // tmp_bat_val += 75; //
        // tmp_bat_val += 95; // 移植过来后，原本是加95
        // tmp_bat_val += 150; //
        // tmp_bat_val += 300; //
        // tmp_bat_val += 600; //
        tmp_bat_val += 900; // 1A-1.1A ========================82khz

        // 164KHz:
        // tmp_bat_val += 1050; // 700-800mA
        // tmp_bat_val += 1100; // 1A - 1.1A，但是这个频率是164KHz，MOS会先发热

        if (adc_bat_val >= 3515) // 如果电池电压大于 8.3V ，降低充电电流（实际测试是在8.25V附近）
        {
            // tmp_bat_val -= 40;
            // tmp_bat_val -= 100; // 约1.1A
            // tmp_bat_val -= 300; // 约0.97A
            tmp_bat_val -= 600; // 约0.8A，8.26V--728mA，8.31--791mA
        }

        tmp_val = max_pwm_val - (u8)(((u32)adc_charging_val * max_pwm_val * 94 / 147) / tmp_bat_val);

        if ((u8)tmp_val >= max_pwm_val)
        {
            // 如果PWM占空比对应的值 大于 最大占空比对应的值，说明计算溢出（可能是电池电压过小），按0处理
            tmp_val = 0;
        }

        // 滤波操作，一开始tmp_val会很小，采集多次后趋于一个平均值：
        tmp_val_cnt++;
        tmp_val_cnt &= 0x07;
        tmp_val_l[tmp_val_cnt] = ((u16)(tmp_val_l[tmp_val_cnt]) + tmp_val) >> 1;
        tmp_val = 0;
        for (i = 0; i < 8; i++)
        {
            tmp_val += tmp_val_l[i];
        }
        tmp_val >>= 3;

        // {
        //     /*
        //         如果差值过大，则快速调节，如果差值过小，则慢速调节，
        //         防止电流突变，导致不同的板子最终充电电流不一致
        //     */
        //     static u8 cnt = 0;
        //     cnt++;

        //     if (tmp_val > last_pwm_val)
        //     {
        //         if ((tmp_val - last_pwm_val) > 2 || cnt >= 10)
        //         {
        //             last_pwm_val++;
        //             cnt = 0;
        //         }
        //     }
        //     else if (tmp_val < last_pwm_val)
        //     {
        //         if ((last_pwm_val - tmp_val) > 2 || cnt >= 10)
        //         {
        //             last_pwm_val--;
        //             cnt = 0;
        //         }
        //     }
        // }

        if (flag_is_adjust_current_time_comes)
        {
            flag_is_adjust_current_time_comes = 0;
            if ((u8)tmp_val > last_pwm_val)
            // if (tmp_val > last_pwm_val)
            {
                // last_pwm_val = last_pwm_val + 1;
                last_pwm_val++;
            }
            else if ((u8)tmp_val < last_pwm_val)
            // else if (tmp_val < last_pwm_val)
            {
                // last_pwm_val = last_pwm_val - 1;
                last_pwm_val--;
            }
        }

        // T2DATA = last_pwm_val;

        // printf("last_pwm_val %u\n",last_pwm_val);
        TMR2_PWML = last_pwm_val;
        // TMR2_PWML = last_pwm_val % 256;
        // TMR2_PWMH = last_pwm_val / 256;
        TMR2_PWMH = 0;

#endif

    } // if (flag_is_in_charging)
#endif // 充电电流控制
}
