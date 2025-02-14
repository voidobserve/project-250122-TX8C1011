#include "key.h"

// ===================================================
// 按键相关配置                                      //
// ===================================================
#define KEY_FILTER_TIMES (3) // 按键消抖次数 (消抖时间 == 消抖次数 * 按键扫描时间)
volatile u8 key_event;       // 存放按键事件的变量

/*
    检测ad按键，使用VDD作为参考电压
    10K上拉连接到VDD
    开关和模式按键使用2.2R下拉，加热按键使用10K下拉
    那么开关和模式按键按下时，对应的电压是
    2.2R / (10K + 2.2R) * VDD,约为 0.0002 VDD
    加热按键按下时，对应的电压
    10K / (10K + 10K) * VDD == 0.5 VDD

    VDD对应的ad值为4096，那么
    开关和模式按键对应的ad值  0.8192
    加热按键对应的ad值       2048
    没有按键按下时，对应的ad值 4096
    取 ad值>>4 的中间值作为划分

    0.8192 / 16 == 0.0512
    2048 / 16 == 128
    4096 / 16 == 256
    (128 - 0.0512) / 2 ,约为64
    (256 - 128) / 2，约为64
    那么检测到 ad值>>4 小于 64 + 0 时，认为是 开关/模式按键按下
    检测到 ad值>>4 大于 64 且小于 64 + 128时，认为是 加热按键按下
    检测到 ad值>>4 大于 64 + 128 时，认为 没有按键按下
*/
#define AD_KEY_VAL_NONE 255 // 表示没有按键按下

#define AD_KEY_VAL_MODE (64)       // 开关/模式按键
#define AD_KEY_VAL_HEAT (64 + 128) // 加热按键

// 定义按键的id，与 key_id_table[]中的顺序有关
// 目前除了KEY_ID_NONE, KEY_ID_MODE对应数组的第0个，KEY_ID_HEAT对应数组的第二个
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

// 存放按键
const u8 key_id_table[] = {
    AD_KEY_VAL_MODE,
    AD_KEY_VAL_HEAT,
}; // 各个按键以中间值作为分隔，从小到大排列

extern volatile bit flag_is_dev_open;
extern volatile bit flag_is_in_charging;

extern void fun_ctl_motor_status(u8 adjust_motor_status);
extern void fun_ctl_power_on(void);
extern void fun_ctl_power_off(void);
extern void fun_ctl_heat_status(u8 adjust_heat_status);

// ad按键扫描检测函数，需要放到周期为10ms的循环内执行
void key_scan_10ms_isr(void)
{
    u8 i = 0; // 循环计数值
    static volatile u8 last_key_id = KEY_ID_NONE;
    static volatile u8 press_cnt = 0;               // 按键按下的时间计数
    static volatile u8 filter_cnt = 0;              // 按键消抖，使用的变量
    static volatile u8 filter_key_id = KEY_ID_NONE; // 按键消抖时使用的变量
    volatile u8 cur_key_id = KEY_ID_NONE;

    static volatile bit flag_is_key_mode_hold = 0;

    volatile u8 adc_val = 0;

    adc_sel_channel(ADC_CHANNEL_KEY_SCAN);
    // adc_val = adc_get_val_once();
    // adc_val >>= 4;

    adc_val = adc_get_val_once() >> 4;

    for (i = 0; i < ARRAY_SIZE(key_id_table); i++) // 获取对应的按键id
    {
        if (adc_val < key_id_table[i])
        {
            // 由于第0个表示KEY_ID_NONE,这里要赋值有效的键值，需要加上1
            cur_key_id = i + 1;
            break;
        }
    }

    // 消抖/滤波
    if (cur_key_id != filter_key_id)
    {
        // 如果有按键按下/松开
        filter_cnt = 0;
        filter_key_id = cur_key_id;
        return;
    }

    if (filter_cnt < KEY_FILTER_TIMES)
    {
        // 如果检测到相同的按键按下/松开
        // 防止计数溢出
        filter_cnt++;
        return;
    }

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
            if (flag_is_dev_open)
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
                if (press_cnt >= 100) // 1000ms加上看门狗唤醒的1024ms
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
    if (flag_is_dev_open)
    {
        // 如果设备正在运行
        if (KEY_EVENT_MODE_HOLD == key_event)
        {
            // 关机：
            fun_ctl_power_off();
        }
        else if (KEY_EVENT_MODE_CLICK == key_event)
        {
            // 设备运行时，检测到 开关/模式 按键短按
            // 参数填0，根据全局变量 cur_motor_status 的状态来自动调节
            fun_ctl_motor_status(0);
        }
        else if (KEY_EVENT_HEAT_CLICK == key_event)
        {
            // 设备运行时，检测到 加热 按键短按
            // 参数填0，根据全局变量 cur_ctl_heat_status 的状态来自动调节
            fun_ctl_heat_status(0);
        }
    }
    else
    {
        // 如果设备没有在运行
        if (KEY_EVENT_MODE_HOLD == key_event && /* 长按了 开关/模式按键 */
            0 == flag_is_in_charging)           /* 当前没有在充电 */
        {
            fun_ctl_power_on(); // 函数内部会将 flag_is_dev_open 置一
        }
    }
 
    // 处理完成后，清除按键事件
    key_event = KEY_EVENT_NONE;
}