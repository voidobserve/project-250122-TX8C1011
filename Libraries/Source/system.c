/**
  ******************************************************************************
  * @file    system.c
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


/** @addtogroup Template_Project
  * @{
  */
  
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define HRCSC_STEP_VAL            (*(u8 code *)0x9111)  // 获取HRC时钟细调值

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

extern void delay_ms(u16 ms);

/**
  * @brief  System initialization function
  * @param  None
  * @retval None
  */
// void system_init(void)
// {
//     u8 hrcsc_data = HRCSC_STEP_VAL & 0x7f;      // 获取HRC时钟细调值
    
//     WDT_KEY    = 0xDD;                          // Close WDT
    
//     FLASH_TIM0 = 0x55;
//     FLASH_TIM1 = 0x54;                          // FLASH 16M 访问速度
//     FLASH_TRIM = 0x0A;
    
//     AIP_CON0   = 0x80 | hrcsc_data;             // 使能 hirc
//     CLK_CON0   = 0x03;                          // 选择 hirc clk
//     CLK_CON2   = 0x00;                          // 系统时钟不分频
//     CLK_CON6   = 0x1F;                          // FLASH烧写时钟32分频：1M
    
//     delay_ms(1);                                // 该延迟不可删除，保证烧录稳定性

//     __ADC_VREFP_TRIM_5V_CALIB;                  // 填入5V电压供电下内部2.4V参考电压的校准值
    
// //    __SET_ALL_IO_ANALOG_MODE;              // 所有IO的初始状态设为模拟模式，在二次开发时不使用到的IO保持模拟模式可减小功耗
// }

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
