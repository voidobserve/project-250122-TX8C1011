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
#define NVR2_START_RADDR                        (0x9100)                         // NVR2的读起始地址
     
#define VREFP_TRIM_5V_ADDR_OFFSET               (35)
#define UPDATA_FLAG_ADDR_OFFSET                 (54)
    
#define __ADC_VREFP_CALIB(addr_offset)                                                  \
do {                                                                                    \
    if((*((u8 code *)NVR2_START_RADDR + UPDATA_FLAG_ADDR_OFFSET)) == 0x79) {            \
        PMU_CON5 &= ~0x1F;                                                              \
        PMU_CON5 |= ((*((u8 code *)NVR2_START_RADDR + addr_offset)) & 0x1F);            \
    }                                                                                   \
} while(0)

#define __ADC_VREFP_TRIM_5V_CALIB __ADC_VREFP_CALIB(VREFP_TRIM_5V_ADDR_OFFSET)    // 校准：VCC电压5V左右

#define __SET_ALL_IO_ANALOG_MODE    \
do {                                \
    P0_MD0 = 0xFF;                  \
    P0_MD1 = 0x3F;                  \
    P1_MD0 = 0xFC;                  \
    P1_MD1 = 0xFF;                  \
} while(0)          // 所有IO的初始状态设为模拟模式(为保证正常烧录，烧录端口不设置)，在二次开发时不使用到的IO保持模拟模式可减小功耗

// 进入SLEEP模式
#define __ENTER_SLEEP_MODE                                                                                                         \
do {                                                                                                                               \
    IO_MAP1   |= 0x0C;                                                                                                             \
    SYS_CON3   = 0xff;                                                                                                             \
    PMU_CON5  &= ~0xE0;  /* LPLDO的参考电压选择最低                                                                             */ \
    LVD_CON0  &= ~0x03;  /* 关闭LVD等PMU内部模块                                                                                */ \
    PMU_CON2  &= ~0x38;  /* 关闭TEMPSENSOR等PMU内部模块                                                                         */ \
    CLK_CON0  &= ~0x03;  /* 时钟切换到64K                                                                                       */ \
    FLASH_TIM1 =  0x00;  /* 时钟切到64k后需要修改FLASH的访问速度                                                                */ \
    AIP_CON0  &= ~0x80;  /* 关闭 HRCOSC                                                                                         */ \
    PMU_CON0  &= ~0x60;  /* 关闭 HRCOSC VR06 REF，关闭之后HRCOSC的参考电压就不能正常了，就不能再用HRCOSC时钟了，关闭PMU POR模块 */ \
/*    LP_CON    |=  0x10;  /* 使能硬件gate lirc，在低功耗模式下gate lirc达到省功耗的目的，但RC64k时钟会停止工作                 */ \
    LP_CON    |=  0x01;  /* 使能SLEEP模式                                                                                       */ \
} while(0)

// SLEEP唤醒之后的处理
#define __SLEEP_AFTER_WAKE_UP                                           \
do {                                                                    \
    PMU_CON5  |= 0x60;   /* LPLDO的参考电压选择默认值                */ \
    WKPND      = 0xFF;   /* 唤醒后清除pending                        */ \
    PMU_CON0  |= 0x60;   /* 使能 HRCOSC VR06 REF，打开PMU POR模块    */ \
    AIP_CON0  |= 0x80;   /* 使能 HRCOSC                              */ \
    FLASH_TIM1 = 0x54;   /* 时钟切到HRCOSC后需要修改FLASH的访问速度  */ \
    CLK_CON0  |= 0x03;   /* 时钟切换到HROSC                          */ \
    LVD_CON0  |= 0x03;   /* 打开LVD 等PMU内部模块                    */ \
} while(0)

// 进入STOP模式
#define __ENTER_STOP_MODE                                               \
do {                                                                    \
    CLK_CON0 &= ~0x03;   /* 时钟切换到64K                            */ \
    AIP_CON0 &= ~0x80;   /* 关闭 HRCOSC                              */ \
    LP_CON   |=  0x02;   /* 进入stop mode                            */ \
} while(0)

// STOP唤醒之后的处理
#define __STOP_AFTER_WAKE_UP                                            \
do {                                                                    \
    WKPND     =  0xFF;   /* 唤醒后清除pending                        */ \
    AIP_CON0 |=  0x80;   /* 使能 HRCOSC                              */ \
    CLK_CON0 |=  0x03;   /* 时钟切换到HRCOSC                         */ \
} while(0)

// 进入IDLE模式
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
