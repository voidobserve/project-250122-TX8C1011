#include "motor.h"

// 20KHz
#define STMR1_PRE (SYSCLK / 1 / (20000 - 100) - 1) // 值 == 1608

// ===================================================
// 电机相关配置                                    //
// ===================================================

// 当前电机的挡位,0--初始状态(0%占空比)，1--一档，2--二档，3--三档
volatile u8 cur_motor_status = 0;
// 记录当前电机的转向，0--初始值，无状态，1--正转，2--反转
volatile u8 cur_motor_dir = 0;

void motor_config(void)
{
    // 正转PWM P02 STMR1_PWMB
    // 反转PWM P03 STMR1_PWMA
    // P0_MD0 &= ~0xF0;
    // P0_MD0 |= 0xA0;
    // P0_AF0 &= ~0xF0;

    P0_AF0 &= ~0xF0; // P02 复用为 STMR1_PWMB，P03 复用为 STMR1_PWMA

    // 配置STIMER1
    STMR1_FCONR = 0x00;          // 选择系统时钟，0分频
    STMR1_PRH = STMR1_PRE / 256; // 周期值
    STMR1_PRL = STMR1_PRE % 256;
    STMR1_CMPAH = STMR1_PRE / 2 / 256;
    STMR1_CMPAL = STMR1_PRE / 2 % 256;
    STMR1_CMPBH = STMR1_PRE / 4 / 256;
    STMR1_CMPBL = STMR1_PRE / 4 % 256;

    STMR1_PCONRA = 0x10; // 使能CHA，计数值大于CHA比较值输出0，小于输出1
    STMR1_PCONRB = 0x10; // 使能CHB，计数值大于CHA比较值输出0，小于输出1
    STMR1_CR |= 0x01;    // 使能高级定时器

    // 关闭定时器，IO配置为输出模式，输出低电平
    motor_pwm_disable();
}

// void motor_pwm_enable(void)
// {
//     // IO配置：
//     P0_MD0 &= ~0xF0;
//     P0_MD0 |= 0xA0;
//     // P0_AF0 &= ~0xF0;
//     STMR1_CR |= 0x01; // 使能高级定时器
// }

void motor_pwm_a_enable(void)
{
    // 反转PWM P03 STMR1_PWMA
    P0_MD0 &= ~(0x03 << 6);
    P0_MD0 |= 0x02 << 6;
    P0_AF0 &= ~(0x03 << 6); // 复用为 STMR1_PWMA
    STMR1_CR |= 0x01;       // 使能高级定时器
}

void motor_pwm_b_enable(void)
{
    // 正转PWM P02 STMR1_PWMB
    P0_MD0 &= ~(0x03 << 4);
    P0_MD0 |= 0x02 << 4;
    P0_AF0 &= ~(0x03 << 4); // 复用为 STMR1_PWMB
    STMR1_CR |= 0x01;       // 使能高级定时器
}

// 关闭PWM通道A的输出，对应IO输出0，但是不会关闭定时器
void motor_pwm_a_disable(void)
{
    // 反转PWM P03 STMR1_PWMA
    P0_MD0 &= ~(0x03 << 6);
    P0_MD0 |= 0x01 << 6; // 输出模式
    P03 = 0;
}

// 关闭PWM通道B的输出，对应IO输出0，但是不会关闭定时器
void motor_pwm_b_disable(void)
{
    // 正转PWM P02 STMR1_PWMB
    P0_MD0 &= ~(0x03 << 4);
    P0_MD0 |= 0x01 << 4; // 输出模式
    P02 = 0;
}

void motor_pwm_disable(void)
{
    STMR1_CR &= ~0x01; // 关闭高级定时器
    // IO配置为输出模式，输出0
    P0_MD0 &= ~0xF0;
    P0_MD0 |= 0x50;
    P02 = 0;
    P03 = 0;
}

/**
 * @brief 修改电机转速（函数内部只修改占空比，不开/关pwm输出）
 *
 * @param adjust_motor_status 要调节的电机挡位
 *          0 -- 关闭电机 0%占空比
 *          1 -- 一档 86.8% 占空比
 *          2 -- 二档 92.8% 占空比
 *          3 -- 三档 100% 占空比
 *          其余参数值不做处理
 */
// 根据 STMR1_PRE 的值来计算占空比：
#define __MOTOR_LEVEL_1 (1395)          // 一档 86.8% 占空比
#define __MOTOR_LEVEL_2 (1492)          // 二档 92.8% 占空比
#define __MOTOR_LEVEL_3 (STMR1_PRE + 1) // 三档 100% 占空比
void alter_motor_speed(u8 adjust_motor_status)
{
    if (0 == adjust_motor_status)
    {
        // T0DATA = 0;
        // T1DATA = 0;
    }
    else if (1 == adjust_motor_status)
    {
        STMR1_CMPAH = __MOTOR_LEVEL_1 / 256;
        STMR1_CMPAL = __MOTOR_LEVEL_1 % 256;
        STMR1_CMPBH = __MOTOR_LEVEL_1 / 256;
        STMR1_CMPBL = __MOTOR_LEVEL_1 % 256;
    }
    else if (2 == adjust_motor_status)
    {
        STMR1_CMPAH = __MOTOR_LEVEL_2 / 256;
        STMR1_CMPAL = __MOTOR_LEVEL_2 % 256;
        STMR1_CMPBH = __MOTOR_LEVEL_2 / 256;
        STMR1_CMPBL = __MOTOR_LEVEL_2 % 256;
    }
    else if (3 == adjust_motor_status)
    {
        STMR1_CMPAH = __MOTOR_LEVEL_3 / 256;
        STMR1_CMPAL = __MOTOR_LEVEL_3 % 256;
        STMR1_CMPBH = __MOTOR_LEVEL_3 / 256;
        STMR1_CMPBL = __MOTOR_LEVEL_3 % 256;
    }
}
