#ifndef __SPEECH_PROCESS_H
#define __SPEECH_PROCESS_H

#include "my_config.h"

#define SPEECH_CTL_PIN P11     // 控制语音IC电源的引脚
#define SPEECH_CTL_PIN_OPEN 1  // 开启语音IC电源时，对应的引脚电平
#define SPEECH_CTL_PIN_CLOSE 0 // 关闭语音IC电源时，对应的引脚电平

// 打开语音IC电源
#define SPEECH_POWER_ENABLE()                 \
    do                                        \
    {                                         \
        SPEECH_CTL_PIN = SPEECH_CTL_PIN_OPEN; \
    } while (0);

// 关闭语音IC电源
#define SPEECH_POWER_DISABLE()                 \
    do                                         \
    {                                          \
        SPEECH_CTL_PIN = SPEECH_CTL_PIN_CLOSE; \
    } while (0);

void speech_ctl_pin_config(void); // 初始化控制语音IC电源的引脚 
void speech_scan_process(void);   // 检测从串口接收到的控制命令并进行处理
 

#endif
