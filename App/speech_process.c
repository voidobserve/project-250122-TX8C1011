#include "speech_process.h"

extern volatile bit flag_is_recv_ctl; // 标志位，是否接收了控制命令

extern volatile bit flag_tim_set_bat_is_low; // 由定时器置位/复位的，表示在工作时，电池是否处于低电量的标志位
extern volatile bit flag_ctl_led_blink;      // 控制标志位，是否控制指示灯闪烁
extern volatile bit flag_ctl_turn_dir;       // 控制标志位，是否更换电机的方向

extern volatile bit flag_ctl_dev_close; // 控制标志位，是否要关闭设备

/*
    标志位，是否通过语音来切换方向.
    用在定时器中断中，判断自动换方向，如果这个标志位置一，
    会清除当前自动换方向的计时，重新开始自动换方向的计时
*/
extern volatile bit flag_is_turn_dir_by_speech;

/*
    标志位，是否接收到了新的语音信息/有新的按键操作
    用在定时器中断，在通过语音关闭设备后，一段时间无操作而彻底关机
    如果这个标志位置一，在定时器内会清除自动彻底关机的倒计时，
    定时器在清除计时后，会给这个标志位清零
*/
extern volatile bit flag_is_new_operation;

extern volatile bit flag_ctl_low_bat_alarm; // 控制标志位，是否使能低电量报警

extern void fun_ctl_motor_status(u8 adjust_motor_status);
extern void fun_ctl_heat_status(u8 adjust_heat_status);

// 初始化控制语音IC电源的引脚
// void speech_ctl_pin_config(void)
// {
//     // P11
//     P1_MD0 |= 0x01 << 2; // 输出模式
// }

void speech_scan_process(void)
{
    if (flag_is_recv_ctl)
    {
        // 如果接收到了正确的控制命令
        volatile u8 cmd = 0;
        flag_is_recv_ctl = 0;
        cmd = recv_ctl; // 从缓冲区取出控制命名

        // 表示有新的控制命令，清除在关闭按摩指令后，无操作自动关机的倒计时
        flag_is_new_operation = 1;
        delay_ms(1); // 等待定时器清空对应的（2min没有语音操作而关机）计时变量和标志位

        // (用 if-else if 语句会比switch语句更省空间)
        // 根据当前设备的情况，对指令进行处理
        if (CMD_OPEN_DEV == cmd) // 打开按摩/按摩枕/按摩器
        {
            // 如果当前电机是关闭的，才对 开机控制命令进行处理
            if (0 == cur_motor_status)
            {
                alter_motor_speed(2); // 开机后，电机进入二档
                motor_pwm_b_enable(); // 电机正向转动

                cur_motor_dir = 1;    // 表示当前电机在正转
                cur_motor_status = 2; // 更新电机挡位状态

                // 如果不处于低电量报警状态，才使能LED闪烁功能：
                if (0 == flag_tim_set_bat_is_low)
                {
                    if (0 == cur_ctl_heat_status)
                    {
                        // 如果没有打开加热，让绿灯闪烁
                        LED_RED_OFF();
                        LED_GREEN_ON();
                        cur_sel_led = CUR_SEL_LED_GREEN;
                    }
                    else
                    {
                        // 如果打开了加热，让红灯闪烁
                        LED_GREEN_OFF();
                        LED_RED_ON();
                        cur_sel_led = CUR_SEL_LED_RED;
                    }

                    cur_ctl_led_blink_cnt = cur_motor_status; // led闪烁次数与当前电机的挡位有关
                    flag_ctl_led_blink = 1;                   // 打开LED闪烁的功能
                }
            }
            else
            {
                // 电机正在运行，不做处理
            }
        }
        else if (CMD_CLOSE_DEV == cmd) // 关闭按摩/按摩枕/按摩器
        {
            // flag_ctl_dev_close = 1;
            cur_ctl_heat_status = 0; // 关闭加热
            motor_pwm_disable();     // 关闭驱动电机的PWM输出：
            LED_RED_OFF();
            interrupt_led_blink(); // 关闭灯光闪烁的动画
            LED_GREEN_ON();        // 点亮绿灯
            delay_ms(100);
            cur_sel_led = CUR_SEL_LED_GREEN;             // 选中绿灯，准备让它闪烁
            cur_ctl_led_blink_cnt = CUR_CTL_LED_BLINK_1; // 闪烁一次
            flag_ctl_led_blink = 1;                      // 使能灯光闪烁
        }
        else if (CMD_OPEN_HEAT == cmd) // 打开加热
        {
            if (0 == cur_ctl_heat_status)
            {
                // 如果加热没有开启
                // 参数填0，根据全局变量 cur_ctl_heat_status 的状态来自动调节
                fun_ctl_heat_status(0);
            }
        }
        else if (CMD_CLOSE_HEAT == cmd) // 关闭加热
        {
            if (0 != cur_ctl_heat_status)
            {
                // 如果加热已经开启
                // flag_ctl_led_blink = 0; // 打断当前正在闪烁的功能
                // delay_ms(1);            // 等待定时器中断内部清空闪烁功能对应的标志和变量，否则打断闪灯的效果会变差
                interrupt_led_blink();
                fun_ctl_close_heat();
                delay_ms(100);
                cur_sel_led = CUR_SEL_LED_GREEN;
                cur_ctl_led_blink_cnt = 1;
                flag_ctl_led_blink = 1;
            }
        }
        else if (CMD_HEAT_FIRST == cmd) // 加热一档
        {
            // if (0 != cur_ctl_heat_status)
            {
                fun_ctl_heat_status(1);
            }
        }
        else if (CMD_HEAT_SECOND == cmd) // 加热二档
        {
            // if (0 != cur_ctl_heat_status)
            {
                fun_ctl_heat_status(2);
            }
        }
        // else if (cur_motor_status) /* 接下来都是与电机有关的操作，需要电机在运作的情况下，再做处理，否则不会进入 */
        // {
        //     if (CMD_INTENSITY_FIRST == cmd) // 力度一档
        //     {
        //         fun_ctl_motor_status(1);
        //     }
        //     else if (CMD_INTENSITY_SECOND == cmd) // 力度二档
        //     {
        //         fun_ctl_motor_status(2);
        //     }
        //     else if (CMD_INTENSITY_THIRD == cmd) // 力度三档
        //     {
        //         fun_ctl_motor_status(3);
        //     }
        //     else if (CMD_CHANGE_DIR == cmd) // 换个方向
        //     {
        //         // 标志位置一，让换方向的操作在主函数执行，因为换方向要间隔500ms
        //         flag_ctl_turn_dir = 1;
        //         // 给另外一个标志位置一，让定时器清除自动换方向的计时
        //         flag_is_turn_dir_by_speech = 1;
        //     }
        // }

#if 1
        else if (CMD_INTENSITY_FIRST == cmd) // 力度一档
        {
            if (0 != cur_motor_status)
            {
                // if (0 == cur_motor_status)
                // {
                //     // motor_forward();
                //     motor_pwm_b_enable(); // 电机正向转动
                // }
                fun_ctl_motor_status(1);
            }
        }
        else if (CMD_INTENSITY_SECOND == cmd) // 力度二档
        {
            if (0 != cur_motor_status)
            {
                // if (0 == cur_motor_status)
                // {
                //     // motor_forward();
                //     motor_pwm_b_enable(); // 电机正向转动
                // }
                fun_ctl_motor_status(2);
            }
        }
        else if (CMD_INTENSITY_THIRD == cmd) // 力度三档
        {
            if (0 != cur_motor_status)
            {
                // if (0 == cur_motor_status)
                // {
                //     // motor_forward();
                //     motor_pwm_b_enable(); // 电机正向转动
                // }
                fun_ctl_motor_status(3);
            }
        }
        else if (CMD_CHANGE_DIR == cmd) // 换个方向
        {
            // if (0 != cur_motor_status)
            if (cur_motor_status) // 电机还在运行，才执行下面语句块
            {
                // 标志位置一，让换方向的操作在主函数执行，因为换方向要间隔500ms
                flag_ctl_turn_dir = 1;

                // 给另外一个标志位置一，让定时器清除自动换方向的计时
                flag_is_turn_dir_by_speech = 1;

                // 让灯光闪烁一次
                interrupt_led_blink(); // 打断当前正在执行的LED闪烁功能

                // 如果不处于低电量报警状态，才使能LED闪烁功能：
                if (0 == flag_ctl_low_bat_alarm)
                {
                    if (0 == cur_ctl_heat_status)
                    {
                        // 如果没有打开加热，让绿灯闪烁
                        LED_RED_OFF();
                        LED_GREEN_ON();
                        cur_sel_led = CUR_SEL_LED_GREEN;
                    }
                    else
                    {
                        // 如果打开了加热，让红灯闪烁
                        LED_GREEN_OFF();
                        LED_RED_ON();
                        cur_sel_led = CUR_SEL_LED_RED;
                    }

                    cur_ctl_led_blink_cnt = 1; // 指定led闪烁次数
                    flag_ctl_led_blink = 1;    // 打开LED闪烁的功能
                }
            }
        }
#endif
    }
}
