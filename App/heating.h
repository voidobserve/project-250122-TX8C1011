#ifndef __HEATING_H
#define __HEATING_H

#include "my_config.h"

// ===================================================
// 加热相关配置 	                                 //
// ===================================================
#define CONTROL_HEAT_PIN P14 // 驱动控制加热的引脚
/*实际测得给高电平为加热*/
#define HEATING_ON()              \
	{                             \
		do                        \
		{                         \
			CONTROL_HEAT_PIN = 1; \
		} while (0);              \
	}
#define HEATING_OFF()             \
	{                             \
		do                        \
		{                         \
			CONTROL_HEAT_PIN = 0; \
		} while (0);              \
	}

// 记录并控制当前加热的状态
// 0--初始值，无状态（会关闭加热）
// 1--加热一档
// 2--加热二档
extern volatile u8 cur_ctl_heat_status;

void heating_pin_config(void); // 初始化控制加热的引脚
// 关闭加热(调用前需要注意，是不是要先关闭当前正在执行的灯光闪烁功能)
void fun_ctl_close_heat(void);

void fun_ctl_heat_status(u8 adjust_heat_status); // 控制加热状态

#endif
