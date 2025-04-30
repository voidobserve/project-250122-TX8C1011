#ifndef __MY_CONFIG_H
#define __MY_CONFIG_H

#include "include.h"

#define USE_MY_DEBUG 0
#define USE_DEBUG_PIN 0 // 是否使用单线通信(不用串口)的引脚用于测试

#if USE_MY_DEBUG
#include <stdio.h>
#endif

#include "adc.h"
#include "tmr0.h"
#include "tmr2.h"
#include "uart.h"

#include "motor.h"
#include "key.h"
#include "led.h"

#include "heating.h"
#include "charge.h"
#include "speech_process.h"
#include "low_power.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

// ===================================================
// 按键相关配置                                      //
// ===================================================
// 选择使用哪个引脚检测 开关/模式 按键，哪个引脚检测 加热 按键
#if 0
// 使用第 10 脚检测 开关/模式按键
// 使用第 07 脚检测 加热按键
#define USE_P10_DETECT_MODE_USE_P07_DETECT_HEAT
#else
// 使用第 07 脚检测 开关/模式按键
// 使用第 10 脚检测 加热按键
#define USE_P07_DETECT_MODE_USE_P10_DETECT_HEAT
#endif // 选择使用哪个引脚检测 开关/模式 按键，哪个引脚检测 加热 按键

// ===================================================
// LED相关配置                                      //
// ===================================================
// 选择使用哪个引脚驱动红灯，哪个引脚驱动绿灯
#if 1
// 使用P13驱动红灯，P12驱动绿灯
#define USE_P13_RLED_USE_P12_GLED
#else
// 使用P12驱动红灯，P13驱动绿灯
#define USE_P12_RLED_USE_P13_GLED
#endif // 选择使用哪个引脚驱动红灯，哪个引脚驱动绿灯

// ===================================================
// 串口接收相关配置                                   //
// ===================================================
#define FORMAT_HEAD (0xA5) // 串口接收控制命令的格式头

/*
    电池电量检测脚外部是 1M上拉，330K下拉，
    检测电池电压的分压系数 == 330K / (1M + 330K)
    约为 0.24812030075187969924812030075188

    ADC使用内部2.4V参考电压
*/
// #define LOW_BAT_ALARM_AD_VAL (2964 + 65) // 低电量报警对应的ad值 7V（计算得出的有误差，这里要加上补偿）
// #define LOW_BAT_ALARM_AD_VAL (2979) /*(2964 + 15)*/ // 低电量报警对应的ad值 7V（计算得出的有误差，这里要加上补偿，这里约为7.03V）
#define LOW_BAT_ALARM_AD_VAL (2894) /* (2879 + 15) */ // 低电量报警对应的ad值  V（计算得出的有误差，这里要加上补偿，这里约为7.03V）
// #define LOW_BAT_ALARM_AD_VAL (2964) // 低电量报警对应的ad值 7V

// #define SHUT_DOWN_BAT_AD_VAL (2894) /*(2879)*/ // 关机电量对应的ad值 6.8V 计算得出的有误差，这里要加上补偿

// 低电量报警的连续扫描时间，单位：ms（超过该时间，启动低电量报警）
#define LOW_BAT_SCAN_TIMES_MS (1000)
// #define LOW_BAT_SCAN_TIMES_MS (50) // 测试时使用
// 关机电量的连续扫描时间，单位：ms（超过该时间，关机）
// #define SHUT_DOWN_SCAN_TIMES_MS (2000)

// 电机堵转时，对应的ad值
#define MOTOR_STALLING_AD_VAL (188) // 2.4V参考电压、12位精度下，这里对应110mV
// 电机堵转的连续扫描时间，单位：ms
#define MOTOR_STALLING_SCAN_TIMES_MS (10000)

// 关闭按摩指令后，无操作自动关机的时，单位：ms
#define NO_OPERATION_SHUT_DOWN_TIMES_MS (120000)

// 使能LED闪烁功能(有多个地方调用相同的代码块，为了节省程序空间，将其整合到一起)
extern void fun_enable_led_blink(u8 blink_times);

#endif
