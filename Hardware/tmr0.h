#ifndef __TMR0_H
#define __TMR0_H

#include "my_config.h"

// 配置1ms定时器
#define TMR0_CNT_TIME (SYSCLK / 128 / (1000 - 20) - 1) // 计数周期 1KHz(实际上会有误差，这里要加上补偿)

// void tmr0_config(void);

#endif

