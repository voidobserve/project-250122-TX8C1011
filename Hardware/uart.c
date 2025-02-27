#include "uart.h"

#include <stdio.h>



extern volatile bit flag_is_recv_ctl; // 标志位，是否接收了控制命令

// void uart1_config(void)
// {
//     // P00 RX
//     P0_MD0 &= ~0x03;
//     P0_MD0 |= 0x02; // 多功能IO模式
//     P0_AF0 &= ~0x03;
//     P0_AF0 |= 0x01; // P00 复用为 UART1_RX

//     __EnableIRQ(UART1_IRQn); // 打开UART模块中断
//     // IE_EA = 1;               // 打开总中断

//     UART1_BAUD1 = (USER_UART_BAUD >> 8) & 0xFF; // 配置波特率高八位
//     UART1_BAUD0 = USER_UART_BAUD & 0xFF;        // 配置波特率低八位
//     UART1_CON = 0x91;                           // 8bit数据，1bit停止位，使能中断
//     UART1_CON |= 0x02;                          // 接收模式
// }

// 定义串口接收的状态机
// 表示串口已经接收了什么类型的数据
enum
{
    RECV_NONE = 0,
    RECV_HEAD,
    RECV_CTL,
    RECV_TAIL,
};
volatile u8 cur_recv_status = RECV_NONE;

 
static volatile u8 recv_data = 0; // 串口接收数据缓冲区，只在当前文件内使用
volatile u8 recv_ctl = 0;         // 存放接收完成的控制命令
void UART1_IRQHandler(void) interrupt UART1_IRQn
{
// RX接收完成中断
#if 0
    if (UART1_STA & 0x20)
    {
        // uart_data = UART1_DATA;
        // printf("%d\n",uart_data); 
        // _flag_is_recved_data = 1;
    }
    UART1_STA |= 0x08; // 清中断请求标志位
#endif

    if (UART1_STA & 0x20)
    {
        if (RECV_NONE == cur_recv_status)
        {
            // 如果当前什么都没有接收，准备接收到来的格式头
            if (FORMAT_HEAD == UART1_DATA)
            {
                cur_recv_status = RECV_HEAD;
            }
        }
        else if (RECV_HEAD == cur_recv_status)
        {
            // 如果已经接收了格式头
            recv_data = UART1_DATA;
            cur_recv_status = RECV_CTL; // 表示已经接收了控制命令
        }
        else if (RECV_CTL == cur_recv_status)
        {
            // 如果已经接收了格式头和控制命令，开始计算校验
            // 校验 == (格式头 + 控制命令) & 0xFF
            if (((FORMAT_HEAD + recv_data) & 0xFF) == UART1_DATA)
            // if ((u8)(FORMAT_HEAD + recv_data)  == UART1_DATA)
            {
                // 给对应的标志位置一，让主循环来扫描并处理对应的控制命令
                // 将控制命令存放到另外的变量中，不占用串口接收的缓冲
                recv_ctl = recv_data;
                flag_is_recv_ctl = 1;
            }
            else
            {
                // 校验不通过
                // flag_is_recv_ctl = 0; // 好像可以不加这一句
            }

            cur_recv_status = RECV_NONE;
        }
    }
    UART1_STA |= 0x08; // 清中断请求标志位
}
