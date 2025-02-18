#ifndef __MOTOR_H
#define __MOTOR_H

#include "my_config.h"

#if USE_MOTOR

// 当前电机的挡位,0--初始状态(0%占空比)，1--一档，2--二档，3--三档
extern volatile u8 cur_motor_status;
// 记录当前电机的转向，0--初始值，无状态，1--正转，2--反转
extern volatile u8 cur_motor_dir;

void motor_config(void);
void motor_pwm_enable(void);
void motor_pwm_a_enable(void);
void motor_pwm_b_enable(void);

void motor_pwm_a_disable(void); // 关闭PWM通道A的输出，对应IO输出0，但是不会关闭定时器
void motor_pwm_b_disable(void); // 关闭PWM通道B的输出，对应IO输出0，但是不会关闭定时器

void motor_pwm_disable(void);

void alter_motor_speed(u8 adjust_motor_status);
void motor_over_current_detect_handle(void);
#endif


#endif
