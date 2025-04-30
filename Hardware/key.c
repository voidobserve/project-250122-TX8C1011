#include "key.h"

// ===================================================
// 按键相关配置                                      //
// ===================================================
#define KEY_FILTER_TIMES (3) // 按键消抖次数 (消抖时间 == 消抖次数 * 按键扫描时间)
volatile u8 key_event;       // 存放按键事件的变量

enum
{
    KEY_ID_NONE = 0,
    KEY_ID_MODE,
    KEY_ID_HEAT,
};

// 定义按键的事件
enum
{
    KEY_EVENT_NONE = 0,
    KEY_EVENT_MODE_CLICK,
    KEY_EVENT_MODE_HOLD,
    KEY_EVENT_HEAT_CLICK,
};

extern volatile bit flag_ctl_dev_close; // 控制标志位，是否要关闭设备

extern volatile bit flag_is_in_charging;

extern volatile bit flag_is_enter_low_power; // 标志位，是否要进入低功耗
extern volatile bit flag_is_disable_to_open; // 标志位，是否不使能开机(低电量不允许开机)

extern void fun_ctl_motor_status(u8 adjust_motor_status);
extern void fun_ctl_power_on(void);
extern void fun_ctl_power_off(void);
extern void fun_ctl_heat_status(u8 adjust_heat_status);

// void key_config(void)
// {
//     // 上拉：
//     P0_PU |= 0x01 << 7;
//     P1_PU |= 0x01;
//     P0_MD1 &= ~(0x03 << 6); // P07 输入模式
//     P1_MD0 &= ~0x03;        // P10 输入模式
// }

// static volatile bit flag_is_key_mode_hold = 0; // 标志位，按键是否按住，没有松手
// static volatile u8 last_key_id = KEY_ID_NONE;
// static volatile u8 press_cnt = 0; // 按键按下的时间计数 
static volatile bit flag_is_key_mode_hold; // 标志位，按键是否按住，没有松手
static volatile u8 last_key_id;
static volatile u8 press_cnt; // 按键按下的时间计数

// 清除按键扫描使用的全局变量
void clear_key_scan(void)
{
    flag_is_key_mode_hold = 0; // 标志位，按键是否按住，没有松手
    last_key_id = KEY_ID_NONE;
    press_cnt = 0; // 按键按下的时间计数
}

// ad按键扫描检测函数，需要放到周期为10ms的循环内执行
void key_scan_10ms_isr(void)
{
    // static volatile u8 filter_cnt = 0;              // 按键消抖，使用的变量
    // static volatile u8 filter_key_id = KEY_ID_NONE; // 按键消抖时使用的变量
    volatile u8 cur_key_id = KEY_ID_NONE;

#ifdef USE_P10_DETECT_MODE_USE_P07_DETECT_HEAT
    if (0 == P10) // 开关/模式 按键 优先级要高于 加热 按键
    {
        cur_key_id = KEY_ID_MODE;
    }
    else if (0 == P07)
    {
        cur_key_id = KEY_ID_HEAT;
    }
    else
    {
        cur_key_id = KEY_ID_NONE;
    }
#endif

#ifdef USE_P07_DETECT_MODE_USE_P10_DETECT_HEAT
    if (0 == P07) // 开关/模式 按键 优先级要高于 加热 按键
    {
        cur_key_id = KEY_ID_MODE;
    }
    else if (0 == P10)
    {
        cur_key_id = KEY_ID_HEAT;
    }
    else
    {
        cur_key_id = KEY_ID_NONE;
    }
#endif

    // 消抖/滤波
    // if (cur_key_id != filter_key_id)
    // {
    //     // 如果有按键按下/松开
    //     filter_cnt = 0;
    //     filter_key_id = cur_key_id;
    //     return;
    // }

    // if (filter_cnt < KEY_FILTER_TIMES)
    // {
    //     // 如果检测到相同的按键按下/松开
    //     // 防止计数溢出
    //     filter_cnt++;
    //     return;
    // }

    // 滤波/消抖完成后，执行到这里
    if (last_key_id != cur_key_id)
    {
        if (last_key_id == KEY_ID_NONE)
        {
            // 如果有按键按下，清除按键按下的时间计数
            press_cnt = 0;
        }
        else if (cur_key_id == KEY_ID_NONE)
        {
            // 如果按键松开
            if (press_cnt < 75)
            {
                // 按下时间小于 750ms ，是短按
                if (KEY_ID_MODE == last_key_id)
                {
                    // 开关/模式按键短按
                    key_event = KEY_EVENT_MODE_CLICK;
                }
                else if (KEY_ID_HEAT == last_key_id)
                {
                    // 加热按键短按
                    key_event = KEY_EVENT_HEAT_CLICK;
                }
            }
            else
            {
                // 长按、长按持续之后松手
                if (KEY_ID_MODE == last_key_id)
                {
                    flag_is_key_mode_hold = 0;
                }
            }
        }
    }
    else if (cur_key_id != KEY_ID_NONE)
    {
        // 如果按键按住不放
        if (press_cnt < 255)
            press_cnt++;

        if (KEY_ID_MODE == cur_key_id)
        {
            // if (flag_is_dev_open)
            // 当前记录电机和加热状态的变量，只要有一个不为0，说明设备在工作
            if (cur_motor_status || cur_ctl_heat_status)
            {
                if (press_cnt >= 200) // 2000ms
                {
                    if (flag_is_key_mode_hold)
                    {
                        // 如果该按键还未松手，不执行操作
                    }
                    else
                    {
                        key_event = KEY_EVENT_MODE_HOLD;
                        flag_is_key_mode_hold = 1;
                    }
                }
            }
            else
            {
                // 如果当前设备是关闭的
                if (press_cnt >= 200) // 2000ms
                {
                    if (flag_is_key_mode_hold)
                    {
                        // 如果该按键还未松手，不执行操作
                    }
                    else
                    {
                        key_event = KEY_EVENT_MODE_HOLD;
                        flag_is_key_mode_hold = 1;
                    }
                }
            }
        }
    }

    last_key_id = cur_key_id;
}

// 按键事件处理
void key_event_handle(void)
{
    if (flag_is_in_charging)
    {
        key_event = KEY_EVENT_NONE;
        return; // 充电时不处理按键事件
    }

    if (KEY_EVENT_MODE_HOLD == key_event) /* 开机/模式按键长按 */
    {
        if (SPEECH_CTL_PIN_OPEN == SPEECH_CTL_PIN) /* 如果语音IC还在工作 */
        {
            // 关机：
            flag_ctl_dev_close = 1;      // 控制标志位置一，让主函数扫描到，并关机
            flag_is_enter_low_power = 1; // 允许进入低功耗
        }
        else /* 如果语音IC不在工作，可能是从低功耗下唤醒 */
        {
            if (flag_is_disable_to_open) // 如果是低电量不允许开机
            {
                // 等待松手后，回到低功耗
                while (0 == P07)
                    ;
                flag_ctl_dev_close = 1;      // 控制标志位置一，让主函数扫描到，并关机
                flag_is_enter_low_power = 1; // 允许进入低功耗
            }
            else
            {
                SPEECH_POWER_ENABLE();
                fun_ctl_power_on();
            }
        }
    }
    else if (KEY_EVENT_MODE_CLICK == key_event) /* 开机/模式按键短按 */
    {
        if (SPEECH_CTL_PIN_OPEN == SPEECH_CTL_PIN) /* 如果语音IC还在工作 */
        {
            if (0 == cur_motor_status)
            {
                // motor_forward();
                motor_pwm_b_enable(); // 电机正向转动
            }
            // 参数填0，根据全局变量 cur_motor_status 的状态来自动调节
            fun_ctl_motor_status(0);
        }
        else /* 如果语音IC不在工作 */
        {
            // 这里可以回到低功耗，关机
            flag_ctl_dev_close = 1;      // 控制标志位置一，让主函数扫描到，并关机
            flag_is_enter_low_power = 1; // 允许进入低功耗
        }
    }
    else if (KEY_EVENT_HEAT_CLICK == key_event) /* 加热按键短按 */
    {
        // if (SPEECH_CTL_PIN_OPEN == SPEECH_CTL_PIN) /* 如果语音IC还在工作 */
        {
            // 参数填0，根据全局变量 cur_ctl_heat_status 的状态来自动调节
            fun_ctl_heat_status(0);
        }
    }

    // 处理完成后，清除按键事件
    key_event = KEY_EVENT_NONE;
}