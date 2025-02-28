#ifndef __KEY_H
#define __KEY_H

#include "my_config.h"

// void key_config(void); 
void clear_key_scan(void);// 清除按键扫描使用的全局变量
void key_scan_10ms_isr(void);
void key_event_handle(void);

#endif
