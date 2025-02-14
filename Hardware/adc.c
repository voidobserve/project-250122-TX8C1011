#include "adc.h"

void adc_config(void)
{
    // P04 AIN4 检测充电口传过来的ad值
    P0_MD1 |= 0x03;        // 模拟模式
    P0_AIOEN |= 0x01 << 4; // 使能模拟功能

    // P06 AIN6 检测电池分压后的ad值
    P0_MD1 |= 0x03 << 4;   // 模拟模式
    P0_AIOEN |= 0x01 << 6; // 使能模拟功能

    AIP_CON2 |= 0xC0; // 使能ADC中CMP使能信号和CMP校准功能
    AIP_CON4 |= 0x01; // 使能ADC偏置电流，参考电压选择内部2.4V(Note: 使用内部参考时，芯片需在5V电压供电下)
}

// 切换adc检测的引脚
void adc_sel_channel(u8 adc_channel)
{
#if 1 // 可以节省7字节程序空间
    // 避免前后切换相同的通道，节省时间：
    static u8 last_adc_channel = ADC_CHANNEL_NONE;
    if (last_adc_channel == adc_channel)
    {
        return;
    }
    last_adc_channel = adc_channel;
#endif // 可以节省7字节程序空间

    // 根据传参，切换成对应的通道
    switch (adc_channel)
    {
    case ADC_CHANNEL_CHARGE:
        // P04 AIN4 检测充电口传过来的ad值
        ADC_CHS0 = 0x04; // 软件触发，P04通路

        AIP_CON3 &= ~(0x03 << 2); // 关闭外部参考电压
        AIP_CON4 |= 0x01 << 4;    // 使能内部参考电压
        break;

    case ADC_CHANNEL_BAT:
        // P06 AIN6 检测电池分压后的ad值
        ADC_CHS0 = 0x06; // 软件触发，P06通路

        AIP_CON3 &= ~(0x03 << 2); // 关闭外部参考电压
        AIP_CON4 |= 0x01 << 4;    // 使能内部参考电压
        break;

    case ADC_CHANNEL_KEY_SCAN:
        // 假设用 P11 AIN9 来检测ad按键 （更换板之后是直接检测电平）
        ADC_CHS0 = 0x09; // 软件触发，P11通路

        // 参考电压改为VDD
        AIP_CON4 &= ~(0x01 << 4); // 关闭内部参考
        AIP_CON3 |= 0x02 << 2;    // 使用VCCA作为参考电压
        break;

    case ADC_CHANNEL_MOTOR: // 过流检测通道

        break;

    default:
        break;
    }

    ADC_CFG0 = 0x14; // 使能A/D转换，使能通道0转换
    delay_ms(1);     // 切换adc检测的引脚后，要延时一段时间，等待adc稳定，防止意料之外的检测结果
}

// 获取adc单次转换后的值
u16 adc_get_val_once(void)
{
    u16 g_temp_value = 0;

    ADC_CFG0 |= 0x01; // 触发ADC0转换
    while (!(ADC_STA & 0x02))
        ;                                          // 等待转换完成
    ADC_STA = 0x02;                                // 清除ADC0转换完成标志位
    g_temp_value = (ADC_DATAH0 << 4) | ADC_DATAL0; // 读取ADC0的值

    return g_temp_value;
}

// 获取adc采集+滤波后的值
u16 adc_get_val(void)
{
    u8 i = 0; // adc采集次数的计数
    u16 g_temp_value = 0;
    u32 g_tmpbuff = 0;
    u16 g_adcmax = 0;
    u16 g_adcmin = 0xFFFF;

    // 采集20次，去掉前两次采样，再去掉一个最大值和一个最小值，再取平均值
    for (i = 0; i < 20; i++)
    {
        // ADC_CFG0 |= 0x01; // 触发ADC0转换
        // while (!(ADC_STA & 0x02))
        //     ;                                          // 等待转换完成
        // ADC_STA = 0x02;                                // 清除ADC0转换完成标志位
        // g_temp_value = (ADC_DATAH0 << 4) | ADC_DATAL0; // 读取ADC0的值

        g_tmpbuff = adc_get_val_once();

        if (i < 2)
            continue; // 丢弃前两次采样的
        if (g_temp_value > g_adcmax)
            g_adcmax = g_temp_value; // 最大
        if (g_temp_value < g_adcmin)
            g_adcmin = g_temp_value; // 最小
        g_tmpbuff += g_temp_value;
    }
    g_tmpbuff -= g_adcmax;           // 去掉一个最大
    g_tmpbuff -= g_adcmin;           // 去掉一个最小
    g_temp_value = (g_tmpbuff >> 4); // 除以16，取平均值

    return g_temp_value;
}