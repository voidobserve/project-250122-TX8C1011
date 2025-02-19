#include "heating.h"

// 记录并控制当前加热的状态
// 0--初始值，无状态（会关闭加热）
// 1--加热一档
// 2--加热二档
volatile u8 cur_ctl_heat_status = 0;

extern volatile bit flag_ctl_led_blink;
extern volatile bit flag_ctl_low_bat_alarm; // 控制标志位，是否使能低电量报警

// 初始化控制加热的引脚
void heating_pin_config(void)
{
    // 配置为输出模式
    //  用 P14 控制加热
    P1_MD1 |= 0x01;
}


// 关闭加热(调用前需要注意，是不是要先关闭当前正在执行的灯光闪烁功能)
void fun_ctl_close_heat(void)
{
    cur_ctl_heat_status = 0; // 清零，在定时器中会关闭加热

    // 如果不处于低电量报警状态，低电量报警状态只有红灯闪烁
    if (0 == flag_ctl_low_bat_alarm)
    {
        LED_RED_OFF(); // 关闭红色指示灯

        if (0 != cur_motor_status)
        {
            // 如果电机还在运行，才显示绿灯
            // 防止关闭了电机，打开了加热，又关闭加热的情况
            LED_GREEN_ON(); // 显示绿灯
        }
    }
}

/**
 * @brief 控制加热状态
 *          如果要根据传参来调节挡位，会改变 全局变量 cur_ctl_heat_status 的状态
 *
 * @param adjust_heat_status 要调节到的挡位
 *                          0--根据全局变量 cur_ctl_heat_status 的状态来自动调节
 *                          1--调节为1档
 *                          2--调节为2档
 *                          3--关闭加热
 *                          其他--函数直接返回 （为了节省程序空间，这里没有加）
 */
void fun_ctl_heat_status(u8 adjust_heat_status)
{
#if 0  // 可以节省4字节程序空间
    if (adjust_heat_status >= 4)
    {
        return; // 传参有误，直接返回
    }
#endif // 可以节省4字节程序空间

    flag_ctl_led_blink = 0; // 打断当前正在闪烁的功能
    delay_ms(1);            // 等待定时器中断内部清空闪烁功能对应的标志和变量，否则打断闪灯的效果会变差

    if (0 == adjust_heat_status)
    {
        // 如果是要根据 全局变量 cur_ctl_heat_status 来自动调节
        if (0 == cur_ctl_heat_status)
        {
            cur_ctl_heat_status = 1;
        }
        else if (1 == cur_ctl_heat_status)
        {
            cur_ctl_heat_status = 2;
        }
        else if (2 == cur_ctl_heat_status)
        {
            // 关闭加热
            fun_ctl_close_heat();
        }
    }
    else
    {
        // 根据传参来调节当前的加热挡位
        if (adjust_heat_status == 3)
        {
            // 如果是要关闭加热
            fun_ctl_close_heat();
        }
        else
        {
            cur_ctl_heat_status = adjust_heat_status;
        }
    }

    // 如果加热没有关闭，开启对应的灯光闪烁效果:
    // 如果不处于低电量报警状态，低电量报警状态只有红灯闪烁
    if (0 == flag_ctl_low_bat_alarm)
    {
        if (0 != cur_ctl_heat_status)
        {
            LED_GREEN_OFF();
            LED_RED_ON();
            cur_sel_led = CUR_SEL_LED_RED;
            cur_ctl_led_blink_cnt = cur_ctl_heat_status; // 灯光闪烁次数与当前加热的挡位有关
            flag_ctl_led_blink = 1;
        }
    }
} 