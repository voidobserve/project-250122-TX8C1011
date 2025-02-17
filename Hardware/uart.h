#ifndef __UART_H
#define __UART_H

#include "my_config.h"

// 定义串口接收到的控制命令：
#define CMD_OPEN_DEV (0x01)         // 打开按摩/按摩枕/按摩器
#define CMD_OPEN_HEAT (0x02)        // 打开加热
#define CMD_CLOSE_HEAT (0x03)       // 关闭加热
#define CMD_CHANGE_DIR (0x04)       // 换个方向
#define CMD_INTENSITY_FIRST (0x05)  // 力度一档
#define CMD_INTENSITY_SECOND (0x06) // 力度二档
#define CMD_INTENSITY_THIRD (0x07)  // 力度三档
#define CMD_HEAT_FIRST (0x08)       // 加热一档
#define CMD_HEAT_SECOND (0x09)      // 加热二档
#define CMD_CLOSE_DEV (0x0A)        // 关闭按摩/按摩枕/按摩器

extern volatile u8 recv_ctl; // 存放接收完成的控制命令

void uart1_config(void); 

#endif
