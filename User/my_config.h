#ifndef __MY_CONFIG_H
#define __MY_CONFIG_H

#include "include.h"

#define USE_MY_DEBUG 0

#if USE_MY_DEBUG
#include <stdio.h>
#endif

#include "adc.h"
#include "tmr1.h"
#include "tmr2.h"

#include "motor.h"
#include "key.h"
#include "led.h"

#include "heating.h"
#include "charge.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

/*
    电池电量检测脚外部是 1M上拉，330K下拉，
    检测电池电压的分压系数 == 330K / (1M + 330K)
    约为 0.24812030075187969924812030075188

    ADC使用内部2.4V参考电压
*/
#define LOW_BAT_ALARM_AD_VAL (2964) // 低电量报警对应的ad值 7V
#define SHUT_DOWN_BAT_AD_VAL (2879) // 关机电量对应的ad值 6.8V

// 低电量报警的连续扫描时间，单位：ms（超过该时间，启动低电量报警）
#define LOW_BAT_SCAN_TIMES_MS (5000)
// 关机电量的连续扫描时间，单位：ms（超过该时间，关机）
#define SHUT_DOWN_SCAN_TIMES_MS (1000)

// 电机堵转时，对应的ad值
#define MOTOR_STALLING_AD_VAL (2048) // 
// 电机堵转的连续扫描时间，单位：ms
#define MOTOR_STALLING_SCAN_TIMES_MS (10000) 

#endif
