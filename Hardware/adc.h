#ifndef __ADC_H
#define __ADC_H

#include "my_config.h"

// 定义adc检测的引脚，用在adc切换检测引脚
enum
{
    ADC_CHANNEL_NONE = 0,
    ADC_CHANNEL_BAT,      // 检测电池分压后的电压
    ADC_CHANNEL_CHARGE,   // 检测是否有充电
    ADC_CHANNEL_MOTOR,    // 检测电机是否堵转 
    // ADC_CHANNEL_KEY_SCAN, // 测试用， 检测ad按键
};

void adc_config(void);
void adc_sel_channel(u8 adc_channel);
u16 adc_get_val_once(void);
u16 adc_get_val(void);

#endif 

