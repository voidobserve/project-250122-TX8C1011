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
#define HRCSC_STEP_VAL            (*(u8 code *)0x9111)  // ��ȡHRCʱ��ϸ��ֵ

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
//     u8 hrcsc_data = HRCSC_STEP_VAL & 0x7f;      // ��ȡHRCʱ��ϸ��ֵ
    
//     WDT_KEY    = 0xDD;                          // Close WDT
    
//     FLASH_TIM0 = 0x55;
//     FLASH_TIM1 = 0x54;                          // FLASH 16M �����ٶ�
//     FLASH_TRIM = 0x0A;
    
//     AIP_CON0   = 0x80 | hrcsc_data;             // ʹ�� hirc
//     CLK_CON0   = 0x03;                          // ѡ�� hirc clk
//     CLK_CON2   = 0x00;                          // ϵͳʱ�Ӳ���Ƶ
//     CLK_CON6   = 0x1F;                          // FLASH��дʱ��32��Ƶ��1M
    
//     delay_ms(1);                                // ���ӳٲ���ɾ������֤��¼�ȶ���

//     __ADC_VREFP_TRIM_5V_CALIB;                  // ����5V��ѹ�������ڲ�2.4V�ο���ѹ��У׼ֵ
    
// //    __SET_ALL_IO_ANALOG_MODE;              // ����IO�ĳ�ʼ״̬��Ϊģ��ģʽ���ڶ��ο���ʱ��ʹ�õ���IO����ģ��ģʽ�ɼ�С����
// }

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
