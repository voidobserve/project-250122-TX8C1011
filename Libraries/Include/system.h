/**
  ******************************************************************************
  * @file    system.h
  * @author  HUGE-IC Application Team
  * @version V1.0.0
  * @date    01-05-2021
  * @brief   Main program body header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2021 HUGE-IC</center></h2>
  *
  *
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NVR2_START_RADDR                        (0x9100)                         // NVR2�Ķ���ʼ��ַ
     
#define VREFP_TRIM_5V_ADDR_OFFSET               (35)
#define UPDATA_FLAG_ADDR_OFFSET                 (54)
    
#define __ADC_VREFP_CALIB(addr_offset)                                                  \
do {                                                                                    \
    if((*((u8 code *)NVR2_START_RADDR + UPDATA_FLAG_ADDR_OFFSET)) == 0x79) {            \
        PMU_CON5 &= ~0x1F;                                                              \
        PMU_CON5 |= ((*((u8 code *)NVR2_START_RADDR + addr_offset)) & 0x1F);            \
    }                                                                                   \
} while(0)

#define __ADC_VREFP_TRIM_5V_CALIB __ADC_VREFP_CALIB(VREFP_TRIM_5V_ADDR_OFFSET)    // У׼��VCC��ѹ5V����

#define __SET_ALL_IO_ANALOG_MODE    \
do {                                \
    P0_MD0 = 0xFF;                  \
    P0_MD1 = 0x3F;                  \
    P1_MD0 = 0xFC;                  \
    P1_MD1 = 0xFF;                  \
} while(0)          // ����IO�ĳ�ʼ״̬��Ϊģ��ģʽ(Ϊ��֤������¼����¼�˿ڲ�����)���ڶ��ο���ʱ��ʹ�õ���IO����ģ��ģʽ�ɼ�С����

// ����SLEEPģʽ
#define __ENTER_SLEEP_MODE                                                                                                         \
do {                                                                                                                               \
    IO_MAP1   |= 0x0C;                                                                                                             \
    SYS_CON3   = 0xff;                                                                                                             \
    PMU_CON5  &= ~0xE0;  /* LPLDO�Ĳο���ѹѡ�����                                                                             */ \
    LVD_CON0  &= ~0x03;  /* �ر�LVD��PMU�ڲ�ģ��                                                                                */ \
    PMU_CON2  &= ~0x38;  /* �ر�TEMPSENSOR��PMU�ڲ�ģ��                                                                         */ \
    CLK_CON0  &= ~0x03;  /* ʱ���л���64K                                                                                       */ \
    FLASH_TIM1 =  0x00;  /* ʱ���е�64k����Ҫ�޸�FLASH�ķ����ٶ�                                                                */ \
    AIP_CON0  &= ~0x80;  /* �ر� HRCOSC                                                                                         */ \
    PMU_CON0  &= ~0x60;  /* �ر� HRCOSC VR06 REF���ر�֮��HRCOSC�Ĳο���ѹ�Ͳ��������ˣ��Ͳ�������HRCOSCʱ���ˣ��ر�PMU PORģ�� */ \
/*    LP_CON    |=  0x10;  /* ʹ��Ӳ��gate lirc���ڵ͹���ģʽ��gate lirc�ﵽʡ���ĵ�Ŀ�ģ���RC64kʱ�ӻ�ֹͣ����                 */ \
    LP_CON    |=  0x01;  /* ʹ��SLEEPģʽ                                                                                       */ \
} while(0)

// SLEEP����֮��Ĵ���
#define __SLEEP_AFTER_WAKE_UP                                           \
do {                                                                    \
    PMU_CON5  |= 0x60;   /* LPLDO�Ĳο���ѹѡ��Ĭ��ֵ                */ \
    WKPND      = 0xFF;   /* ���Ѻ����pending                        */ \
    PMU_CON0  |= 0x60;   /* ʹ�� HRCOSC VR06 REF����PMU PORģ��    */ \
    AIP_CON0  |= 0x80;   /* ʹ�� HRCOSC                              */ \
    FLASH_TIM1 = 0x54;   /* ʱ���е�HRCOSC����Ҫ�޸�FLASH�ķ����ٶ�  */ \
    CLK_CON0  |= 0x03;   /* ʱ���л���HROSC                          */ \
    LVD_CON0  |= 0x03;   /* ��LVD ��PMU�ڲ�ģ��                    */ \
} while(0)

// ����STOPģʽ
#define __ENTER_STOP_MODE                                               \
do {                                                                    \
    CLK_CON0 &= ~0x03;   /* ʱ���л���64K                            */ \
    AIP_CON0 &= ~0x80;   /* �ر� HRCOSC                              */ \
    LP_CON   |=  0x02;   /* ����stop mode                            */ \
} while(0)

// STOP����֮��Ĵ���
#define __STOP_AFTER_WAKE_UP                                            \
do {                                                                    \
    WKPND     =  0xFF;   /* ���Ѻ����pending                        */ \
    AIP_CON0 |=  0x80;   /* ʹ�� HRCOSC                              */ \
    CLK_CON0 |=  0x03;   /* ʱ���л���HRCOSC                         */ \
} while(0)

// ����IDLEģʽ
#define __ENTER_IDLE_MODE                                               \
do {                                                                    \
    LP_CON &= ~0x80;                                                    \
} while(0)
     
/****************** Peripheral Interrupt Numbers ****************************/
//#define  WKPND_IRQn           
#define  LVD_IRQn               0
#define  P0_IRQn                1
#define  P1_IRQn                2
#define  TMR0_IRQn              3
#define  TMR1_IRQn              4
#define  TMR2_IRQn              5
#define  STMR1_IRQn             6
#define  STMR2_IRQn             7
#define  ADC_IRQn               8
#define  COMP_IRQn              9
#define  IIC_IRQn              10
#define  UART0_IRQn            11
#define  UART1_IRQn            12
#define  SPI0_IRQn             13
#define  WDT_IRQn              14
#define __EnableIRQ(IRQn)     ((IRQn < 7) ? (IE |=  (1<<IRQn)) : (IE1 |=  (1<<(IRQn-7))))
#define __DisableIRQ(IRQn)    ((IRQn < 7) ? (IE &= ~(1<<IRQn)) : (IE1 &= ~(1<<(IRQn-7))))

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @addtogroup Template_Project
  * @{
  */

/** @addtogroup SYSTEM
  * @{
  */ 

     
/** @defgroup SYSTEM_Exported_Constants
  * @{
  */
/**
  * @}
  */


/** @defgroup SYSTEM_Exported_Functions
  * @{
  */
  
/**
  * @brief  System initialization function
  * @param  None
  * @retval None
  */
void system_init(void);

  
/**
  * @}
  */ 
     

#ifdef __cplusplus
}
#endif

#endif //__SYSTEM_H__

/**
  * @}
  */

/**
  * @}
  */

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
