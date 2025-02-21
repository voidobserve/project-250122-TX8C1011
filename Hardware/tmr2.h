#ifndef __TMR2_H
#define __TMR2_H

#include "my_config.h"

#define TMR2_CNT_TIME (SYSCLK / 1 / (164000 - 1500) - 1) // 计数周期 164KHz(实际上会有误差，这里要加上补偿)
// TMR2_CNT_TIME == 195~196

// void tmr2_pwm_config(void);
void tmr2_pwm_enable(void);
void tmr2_pwm_disable(void);

#endif