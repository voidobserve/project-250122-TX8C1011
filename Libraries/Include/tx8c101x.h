/**
  ******************************************************************************
  * @file    tx8c101x.h
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
#ifndef __TX8C101x_H__
#define __TX8C101x_H__

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "typedef.h"

// SP/DP
sfr   SP         = 0x81;
sfr   DPL0       = 0x82;
sfr   DPH0       = 0x83;
sfr16 DP0        = 0x82;
sfr   DPL1       = 0x84;
sfr   DPH1       = 0x85;
sfr16 DP1        = 0x84;
sfr   DPS        = 0x86;
sfr   PCON0      = 0x87;
sfr   SPH        = 0x9B;
sfr   PCON1      = 0x9C;

// IE
sfr  IE          = 0xA8;
sfr  IE1         = 0xA9;
sbit IE_EA       = 0xAF;

// IP
sfr IP           = 0xB8;
sfr IP1          = 0xB9;

// PSW
sfr  PSW         = 0xD0;
sbit PSW_CY      = 0xD7;
sbit PSW_AC      = 0xD6;
sbit PSW_F0      = 0xD5;
sbit PSW_RS1     = 0xD4;
sbit PSW_RS0     = 0xD3;
sbit PSW_OV      = 0xD2;
sbit PSW_F1      = 0xD1;
sbit PSW_Parity  = 0xD0;

// ACC
sfr  ACC         = 0xE0;
sbit ACC_7       = 0xE7;
sbit ACC_6       = 0xE6;
sbit ACC_5       = 0xE5;
sbit ACC_4       = 0xE4;
sbit ACC_3       = 0xE3;
sbit ACC_2       = 0xE2;
sbit ACC_1       = 0xE1;
sbit ACC_0       = 0xE0;

// B
sfr B            = 0xF0;

// P0
sfr  P0          = 0x80;
sbit P00         = 0x80;
sbit P01         = 0x81;
sbit P02         = 0x82;
sbit P03         = 0x83;
sbit P04         = 0x84;
sbit P05         = 0x85;
sbit P06         = 0x86;
sbit P07         = 0x87;

// P1
sfr  P1          = 0x90;
sbit P10         = 0x90;
sbit P11         = 0x91;
sbit P12         = 0x92;
sbit P13         = 0x93;
sbit P14         = 0x94;
sbit P15         = 0x95;

// TMR ALLCON 
sfr TMR_ALLCON   = 0xF5;

// TMR0
sfr TMR0_CONL    = 0x88;
sfr TMR0_CONH    = 0x89;
sfr TMR0_CNTL    = 0x8A;
sfr TMR0_PRL     = 0x8C;
sfr TMR0_PWML    = 0x8E;

// TMR1
sfr TMR1_CONL    = 0xB0;
sfr TMR1_CONH    = 0xB1;
sfr TMR1_CNTL    = 0xB2;
sfr TMR1_PRL     = 0xB4;
sfr TMR1_PWML    = 0xB6;

// TMR2
sfr TMR2_CONL    = 0xC0;
sfr TMR2_CONH    = 0xC1;
sfr TMR2_CNTL    = 0xC2;
sfr TMR2_CNTH    = 0xC3;
sfr TMR2_PRL     = 0xC4;
sfr TMR2_PRH     = 0xC5;
sfr TMR2_PWML    = 0xC6;
sfr TMR2_PWMH    = 0xC7;

// SUPER TIMER1
sfr STMR1_CNTL   = 0xC8;
sfr STMR1_CNTH   = 0xC9;
sfr STMR1_PRL    = 0xCA;
sfr STMR1_PRH    = 0xCB;
sfr STMR1_CMPAL  = 0xCC;
sfr STMR1_CMPAH  = 0xCD;
sfr STMR1_CMPBL  = 0xCE;
sfr STMR1_CMPBH  = 0xCF;
sfr STMR1_CR     = 0xD1;
sfr STMR1_FCONR  = 0xD2;
sfr STMR1_VPERR  = 0xD3;
sfr STMR1_DTUA   = 0xD4;
sfr STMR1_BRAKE  = 0xD5;
sfr STMR1_DTR    = 0xD6;
sfr STMR1_PCONRA = 0xD7;
sfr STMR1_PCONRB = 0xD8;
sfr STMR1_IE     = 0xD9;
sfr STMR1_SR     = 0xDA;

// SUPER TIMER2
sfr STMR2_CNTL   = 0xDB;
sfr STMR2_CNTH   = 0xDC;
sfr STMR2_PRL    = 0xDD;
sfr STMR2_PRH    = 0xDE;
sfr STMR2_CMPAL  = 0xDF;
sfr STMR2_CMPAH  = 0xE1;
sfr STMR2_CMPBL  = 0xE2;
sfr STMR2_CMPBH  = 0xE3;
sfr STMR2_CR     = 0xE4;
sfr STMR2_FCONR  = 0xE5;
sfr STMR2_VPERR  = 0xE6;
sfr STMR2_DTUA   = 0xE7;
sfr STMR2_BRAKE  = 0xE8;
sfr STMR2_DTR    = 0xE9;
sfr STMR2_PCONRA = 0xEA;
sfr STMR2_PCONRB = 0xEB;
sfr STMR2_IE     = 0xEC;
sfr STMR2_SR     = 0xED;

sfr STMR_ALLCO   = 0xF5;

// LVD CTL
sfr LVD_CON0     = 0xBA;
sfr LVD_CON1     = 0xBB;
sfr LVD_CON2     = 0xBC;
sfr LVD_CON3     = 0xBD;

// LP & WK
sfr LP_CON       = 0xBE;
sfr SYS_PND      = 0xBF;

sfr WKUP_CON0    = 0xAB;
sfr WKPND        = 0xAC;

// WDT
sfr WDT_CON      = 0xAD;
sfr WDT_KEY      = 0xAE;

// ADC
sfr ADC_CFG0     = 0x91;
sfr ADC_CFG1     = 0x92;
sfr ADC_STA      = 0x93;
sfr ADC_DATAH0   = 0x94;
sfr ADC_DATAL0   = 0x95;
sfr ADC_DATAH1   = 0x96;
sfr ADC_DATAL1   = 0x97;
sfr ADC_CHS0     = 0x98;
sfr ADC_CHS1     = 0x99;
sfr ADC_CFG2     = 0x9A;
sfr ADC_CFG3     = 0xEE;

// SPI0
sfr SPI0_CON     = 0xF1;
sfr SPI0_BAUD    = 0xF2;
sfr SPI0_DATA    = 0xF3;
sfr SPI0_STA     = 0xF4;

// CRC16
sfr CRC_REG      = 0x9E;
sfr CRC_FIFO     = 0x9F;

// Uart0
sfr UART0_CON    = 0xF6;
sfr UART0_STA    = 0xF7;
sfr UART0_BAUD0  = 0xF8;
sfr UART0_BAUD1  = 0xF9;
sfr UART0_DATA   = 0xFA;

// Uart1
sfr UART1_CON    = 0xFB;
sfr UART1_STA    = 0xFC;
sfr UART1_BAUD0  = 0xFD;
sfr UART1_BAUD1  = 0xFE;
sfr UART1_DATA   = 0xFF;

// FLASH controller
sfr FLASH_CON    = 0xA0;
sfr FLASH_STA    = 0xA1;
sfr FLASH_DATA   = 0xA2;
sfr FLASH_TIM0   = 0xA3;
sfr FLASH_TIM1   = 0xA4;
sfr FLASH_CRCLEN = 0xA5;
sfr FLASH_PSWD   = 0xA6;
sfr FLASH_ADDR   = 0xA7;
sfr FLASH_TRIM   = 0xAA;

//-------------------------------------------------
// XDATA SFRs
//-------------------------------------------------
// GPIO 0
#define P0_BASE        0x50
#define P0_PU          *(unsigned char volatile pdata *)(P0_BASE+0x0) 
#define P0_PD          *(unsigned char volatile pdata *)(P0_BASE+0x1) 
#define P0_MD0         *(unsigned char volatile pdata *)(P0_BASE+0x2) 
#define P0_MD1         *(unsigned char volatile pdata *)(P0_BASE+0x3) 
#define P0_AF0         *(unsigned char volatile pdata *)(P0_BASE+0x4) 
#define P0_AF1         *(unsigned char volatile pdata *)(P0_BASE+0x5)
#define P0_TRG0        *(unsigned char volatile pdata *)(P0_BASE+0x6) 
#define P0_TRG1        *(unsigned char volatile pdata *)(P0_BASE+0x7) 
#define P0_PND         *(unsigned char volatile pdata *)(P0_BASE+0x8)  
#define P0_IMK         *(unsigned char volatile pdata *)(P0_BASE+0x9)  
#define P0_AIOEN       *(unsigned char volatile pdata *)(P0_BASE+0xA)
#define P0_DRV         *(unsigned char volatile pdata *)(P0_BASE+0xB) 
#define P0_OD          *(unsigned char volatile pdata *)(P0_BASE+0xC) 
    
// GPIO 1
#define P1_BASE        0x60
#define P1_PU          *(unsigned char volatile pdata *)(P1_BASE+0x0) 
#define P1_PD          *(unsigned char volatile pdata *)(P1_BASE+0x1) 
#define P1_MD0         *(unsigned char volatile pdata *)(P1_BASE+0x2) 
#define P1_MD1         *(unsigned char volatile pdata *)(P1_BASE+0x3) 
#define P1_AF0         *(unsigned char volatile pdata *)(P1_BASE+0x4) 
#define P1_AF1         *(unsigned char volatile pdata *)(P1_BASE+0x5)
#define P1_TRG0        *(unsigned char volatile pdata *)(P1_BASE+0x6) 
#define P1_TRG1        *(unsigned char volatile pdata *)(P1_BASE+0x7)
#define P1_PND         *(unsigned char volatile pdata *)(P1_BASE+0x8) 
#define P1_IMK         *(unsigned char volatile pdata *)(P1_BASE+0x9)  
#define P1_AIOEN       *(unsigned char volatile pdata *)(P1_BASE+0xA)
#define P1_DRV         *(unsigned char volatile pdata *)(P1_BASE+0xB) 
#define P1_OD          *(unsigned char volatile pdata *)(P1_BASE+0xC) 
    
// SYS CTRL
#define SYS_BASE       0x08
#define SYS_CON0       *(unsigned char volatile pdata *)(SYS_BASE+0x0)
#define SYS_CON1       *(unsigned char volatile pdata *)(SYS_BASE+0x1)
#define SYS_CON2       *(unsigned char volatile pdata *)(SYS_BASE+0x2)
#define SYS_CON3       *(unsigned char volatile pdata *)(SYS_BASE+0x3)
#define SYS_CON4       *(unsigned char volatile pdata *)(SYS_BASE+0x4)
#define SYS_CON5       *(unsigned char volatile pdata *)(SYS_BASE+0x5)
    
// CLK CTL
#define CLK_BASE       0x10
#define CLK_CON0       *(unsigned char volatile pdata *)(CLK_BASE+0x0)
#define CLK_CON1       *(unsigned char volatile pdata *)(CLK_BASE+0x1)
#define CLK_CON2       *(unsigned char volatile pdata *)(CLK_BASE+0x2)
#define CLK_CON3       *(unsigned char volatile pdata *)(CLK_BASE+0x3)
#define CLK_CON4       *(unsigned char volatile pdata *)(CLK_BASE+0x4)
#define CLK_CON5       *(unsigned char volatile pdata *)(CLK_BASE+0x5)
#define CLK_CON6       *(unsigned char volatile pdata *)(CLK_BASE+0x6)
    
// PMU CTL
#define PMU_BASE       0x20
#define PMU_CON0       *(unsigned char volatile pdata *)(PMU_BASE+0x0)
#define PMU_CON1       *(unsigned char volatile pdata *)(PMU_BASE+0x1)
#define PMU_CON2       *(unsigned char volatile pdata *)(PMU_BASE+0x2)
#define PMU_CON3       *(unsigned char volatile pdata *)(PMU_BASE+0x3)
#define PMU_CON4       *(unsigned char volatile pdata *)(PMU_BASE+0x4)
#define PMU_CON5       *(unsigned char volatile pdata *)(PMU_BASE+0x5)
#define PMU_CON6       *(unsigned char volatile pdata *)(PMU_BASE+0x6)
#define IO_MAP         *(unsigned char volatile pdata *)(PMU_BASE+0x8)
#define IO_MAP1        *(unsigned char volatile pdata *)(PMU_BASE+0xA)

// ANALOG CTL
#define AIP_BASE       0x30
#define AIP_CON0       *(unsigned char volatile pdata *)(AIP_BASE+0x0)
#define AIP_CON1       *(unsigned char volatile pdata *)(AIP_BASE+0x1)
#define AIP_CON2       *(unsigned char volatile pdata *)(AIP_BASE+0x2)
#define AIP_CON3       *(unsigned char volatile pdata *)(AIP_BASE+0x3)
#define AIP_CON4       *(unsigned char volatile pdata *)(AIP_BASE+0x4)
    
// I2C
#define I2C_BASE       0x70
#define I2C0_CON       *(unsigned char volatile pdata *)(I2C_BASE+0x0)
#define I2C0_DATA      *(unsigned char volatile pdata *)(I2C_BASE+0x1)
#define I2C0_ADR       *(unsigned char volatile pdata *)(I2C_BASE+0x2)
#define I2C0_STA       *(unsigned char volatile pdata *)(I2C_BASE+0x3)

// for SIMULATION
#define SIM_END_ADR    *(unsigned char volatile pdata *)(I2C_BASE+0xD)
#define PRINT_HEAD     *(unsigned char volatile pdata *)(I2C_BASE+0xE)
#define PRINT_BODY     *(unsigned char volatile pdata *)(I2C_BASE+0xF)

// COMP
#define CMP_BASE0      0x36
#define CMP_BASE1      0x74
#define CMP0_CON0      *(unsigned char volatile pdata *)(CMP_BASE0+0x0)
#define CMP0_CON1      *(unsigned char volatile pdata *)(CMP_BASE0+0x1)
#define CMP1_CON0      *(unsigned char volatile pdata *)(CMP_BASE0+0x2)
#define CMP1_CON1      *(unsigned char volatile pdata *)(CMP_BASE0+0x3)
#define CMP_CON        *(unsigned char volatile pdata *)(CMP_BASE0+0x4)
#define CMP_STA        *(unsigned char volatile pdata *)(CMP_BASE0+0x5)

#define CMP0_CON2      *(unsigned char volatile pdata *)(CMP_BASE1+0x0)
#define CMP0_CON3      *(unsigned char volatile pdata *)(CMP_BASE1+0x1)
#define CMP0_CON4      *(unsigned char volatile pdata *)(CMP_BASE1+0x2)
#define CMP0_DAC0      *(unsigned char volatile pdata *)(CMP_BASE1+0x3)
#define CMP0_DAC1      *(unsigned char volatile pdata *)(CMP_BASE1+0x4)
#define CMP1_CON2      *(unsigned char volatile pdata *)(CMP_BASE1+0x5)
#define CMP1_CON3      *(unsigned char volatile pdata *)(CMP_BASE1+0x6)
#define CMP1_CON4      *(unsigned char volatile pdata *)(CMP_BASE1+0x7)
#define CMP1_DAC0      *(unsigned char volatile pdata *)(CMP_BASE1+0x8)
#define CMP1_DAC1      *(unsigned char volatile pdata *)(CMP_BASE1+0x9)

#ifdef __cplusplus
}
#endif

#endif //__TX8C101x_H__

/**
  * @}
  */

/**
  * @}
  */

/*************************** (C) COPYRIGHT 2021 HUGE-IC ***** END OF FILE *****/
