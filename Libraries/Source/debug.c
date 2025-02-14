/**
 ******************************************************************************
 * @file    debug.c
 * @author  HUGE-IC Application Team
 * @version V1.0.0
 * @date    01-05-2021
 * @brief   Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
 *
 *
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "include.h"
#include "my_config.h"

/** @addtogroup Template_Project
 * @{
 */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Delay function
 * @param  cnt : the number of cnt to delay
 * @retval None
 */
void delay(u16 cnt)
{
    while (cnt--)
    {
    }
}

/**
 * @brief  Delay function
 * @param  ms : the number of ms to delay
 * @retval None
 */
void delay_ms(u16 ms)
{
    while (ms--)
    {
        delay(1662);
    }
}

#if USE_MY_DEBUG
/**
 * @brief  Debug puchar function
 * @param  uart_data : Data that needs to be sent
 * @retval None
 */
void debug_putchar(u8 uart_data)
{
    while (!(UART0_STA & (1 << 4)))
        ;
    UART0_DATA = uart_data;
    while (!(UART0_STA & (1 << 4)))
        ;
}

// 重写putchar()函数
char putchar(char c)
{
    debug_putchar(c);
    return c;
}

/**
 * @brief  Debug initialization function
 * @param  None
 * @retval None
 */
void debug_init(void)
{
    u8 i = 0;

    // UART0 IO : TX--P13(AF3)
    P1_MD0 &= ~0xC0;
    P1_MD0 |= 0x80;
    P1_AF0 |= 0xC0;

    while (!(UART0_STA & (1 << 4)))
        ;

    UART0_BAUD1 = (((SYSCLK - DEBUG_BAUD) / DEBUG_BAUD) >> 8) & 0xFF;
    UART0_BAUD0 = ((SYSCLK - DEBUG_BAUD) / DEBUG_BAUD) & 0xFF;
    UART0_CON = 0x10;
}
#endif

// /**
//  * @brief  Debug User printf function
//  * @param  p_data : The string to send
//  * @retval None
//  */
// void user_printf(char *p_data)
// {
//     while (*p_data != '\0')
//     {
//         debug_putchar(*p_data++);
//     }
// }

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
