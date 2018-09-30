/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : 71x_map.h
* Author             : MCD Application Team
* Date First Issued  : 05/16/2003
* Description        : Peripherals memory mapping and registers structures
********************************************************************************
* History:
*  24/05/2005 : V3.0
*  30/11/2004 : V2.0
*  14/07/2004 : V1.3
*  01/01/2004 : V1.2
*******************************************************************************
 THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS WITH
 CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
 AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
 OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT
 OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION
 CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

#ifndef __71x_map_H
#define __71x_map_H

#ifndef EXTERN
#define EXTERN extern
#endif

#include "inttypes.h"
#include "71x_conf.h"
#include "71x_type.h"


/* IP registers structures */

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t DATA0;
  volatile uint16_t EMPTY1[3];
  volatile uint16_t DATA1;
  volatile uint16_t EMPTY2[3];
  volatile uint16_t DATA2;
  volatile uint16_t EMPTY3[3];
  volatile uint16_t DATA3;
  volatile uint16_t EMPTY4[3];
  volatile uint16_t CSR;
  volatile uint16_t EMPTY5[7];
  volatile uint16_t CPR;
} ADC12_TypeDef;

typedef volatile struct
{
  volatile uint32_t CKDIS;
  volatile uint32_t SWRES;
} APB_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t RXR;
  volatile uint16_t EMPTY1;
  volatile uint16_t TXR;
  volatile uint16_t EMPTY2;
  volatile uint16_t CSR1;
  volatile uint16_t EMPTY3;
  volatile uint16_t CSR2;
  volatile uint16_t EMPTY4;
  volatile uint16_t CLK;
} BSPI_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t CRR;
  volatile uint16_t EMPTY1;
  volatile uint16_t CMR;
  volatile uint16_t EMPTY2;
  volatile uint16_t M1R;
  volatile uint16_t EMPTY3;
  volatile uint16_t M2R;
  volatile uint16_t EMPTY4;
  volatile uint16_t A1R;
  volatile uint16_t EMPTY5;
  volatile uint16_t A2R;
  volatile uint16_t EMPTY6;
  volatile uint16_t MCR;
  volatile uint16_t EMPTY7;
  volatile uint16_t DA1R;
  volatile uint16_t EMPTY8;
  volatile uint16_t DA2R;
  volatile uint16_t EMPTY9;
  volatile uint16_t DB1R;
  volatile uint16_t EMPTY10;
  volatile uint16_t DB2R;
  volatile uint16_t EMPTY11[27];
} CAN_MsgObj_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t CR;
  volatile uint16_t EMPTY1;
  volatile uint16_t SR;
  volatile uint16_t EMPTY2;
  volatile uint16_t ERR;
  volatile uint16_t EMPTY3;
  volatile uint16_t BTR;
  volatile uint16_t EMPTY4;
  volatile uint16_t IDR;
  volatile uint16_t EMPTY5;
  volatile uint16_t TESTR;
  volatile uint16_t EMPTY6;
  volatile uint16_t BRPR;
  volatile uint16_t EMPTY7[3];
  CAN_MsgObj_TypeDef sMsgObj[2];
  volatile uint16_t EMPTY8[16];
  volatile uint16_t TR1R;
  volatile uint16_t EMPTY9;
  volatile uint16_t TR2R;
  volatile uint16_t EMPTY10[13];
  volatile uint16_t ND1R;
  volatile uint16_t EMPTY11;
  volatile uint16_t ND2R;
  volatile uint16_t EMPTY12[13];
  volatile uint16_t IP1R;
  volatile uint16_t EMPTY13;
  volatile uint16_t IP2R;
  volatile uint16_t EMPTY14[13];
  volatile uint16_t MV1R;
  volatile uint16_t EMPTY15;
  volatile uint16_t MV2R;
  volatile uint16_t EMPTY16;
} CAN_TypeDef;

typedef volatile struct
{
  volatile uint32_t ICR;
  volatile uint32_t CICR;
  volatile uint32_t CIPR;
  volatile uint32_t EMPTY1[3];
  volatile uint32_t IVR;
  volatile uint32_t FIR;
  volatile uint32_t IER;
  volatile uint32_t EMPTY2[7];
  volatile uint32_t IPR;
  volatile uint32_t EMPTY3[7];
  volatile uint32_t SIR[32];
} EIC_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t BCON0;
  volatile uint16_t EMPTY1;
  volatile uint16_t BCON1;
  volatile uint16_t EMPTY2;
  volatile uint16_t BCON2;
  volatile uint16_t EMPTY3;
  volatile uint16_t BCON3;
  volatile uint16_t EMPTY4;
} EMI_TypeDef;

typedef volatile struct
{
  volatile uint32_t CR0;
  volatile uint32_t CR1;
  volatile uint32_t DR0;
  volatile uint32_t DR1;
  volatile uint32_t AR;
  volatile uint32_t ER;
} FLASHR_TypeDef;

typedef volatile struct
{
  volatile uint32_t NVWPAR;
  volatile uint32_t EMPTY;
  volatile uint32_t NVAPR0;
  volatile uint32_t NVAPR1;
} FLASHPR_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t PC0;
  volatile uint16_t EMPTY1;
  volatile uint16_t PC1;
  volatile uint16_t EMPTY2;
  volatile uint16_t PC2;
  volatile uint16_t EMPTY3;
  volatile uint16_t PD;
  volatile uint16_t EMPTY4;
} GPIO_TypeDef;

typedef volatile struct __attribute__ ((packed))
{
  volatile uint8_t  CR;
  volatile uint8_t  EMPTY1[3];
  volatile uint8_t  SR1;
  volatile uint8_t  EMPTY2[3];
  volatile uint8_t  SR2;
  volatile uint8_t  EMPTY3[3];
  volatile uint8_t  CCR;
  volatile uint8_t  EMPTY4[3];
  volatile uint8_t  OAR1;
  volatile uint8_t  EMPTY5[3];
  volatile uint8_t  OAR2;
  volatile uint8_t  EMPTY6[3];
  volatile uint8_t  DR;
  volatile uint8_t  EMPTY7[3];
  volatile uint8_t  ECCR;
} I2C_TypeDef;

typedef volatile struct
{
  volatile uint32_t CCR;
  volatile uint32_t EMPTY1;
  volatile uint32_t CFR;
  volatile uint32_t EMPTY2[3];
  volatile uint32_t PLL1CR;
  volatile uint32_t PER;
  volatile uint32_t SMR;
} RCCU_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t MDIVR;
  volatile uint16_t EMPTY1;
  volatile uint16_t PDIVR;
  volatile uint16_t EMPTY2;
  volatile uint16_t RSTR;
  volatile uint16_t EMPTY3;
  volatile uint16_t PLL2CR;
  volatile uint16_t EMPTY4;
  volatile uint16_t BOOTCR;
  volatile uint16_t EMPTY5;
  volatile uint16_t PWRCR;
} PCU_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t CRH;
  volatile uint16_t EMPTY1;
  volatile uint16_t CRL;
  volatile uint16_t EMPTY2;
  volatile uint16_t PRLH;
  volatile uint16_t EMPTY3;
  volatile uint16_t PRLL;
  volatile uint16_t EMPTY4;
  volatile uint16_t DIVH;
  volatile uint16_t EMPTY5;
  volatile uint16_t DIVL;
  volatile uint16_t EMPTY6;
  volatile uint16_t CNTH;
  volatile uint16_t EMPTY7;
  volatile uint16_t CNTL;
  volatile uint16_t EMPTY8;
  volatile uint16_t ALRH;
  volatile uint16_t EMPTY9;
  volatile uint16_t ALRL;
} RTC_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t ICAR;
  volatile uint16_t EMPTY1;
  volatile uint16_t ICBR;
  volatile uint16_t EMPTY2;
  volatile uint16_t OCAR;
  volatile uint16_t EMPTY3;
  volatile uint16_t OCBR;
  volatile uint16_t EMPTY4;
  volatile uint16_t CNTR;
  volatile uint16_t EMPTY5;
  volatile uint16_t CR1;
  volatile uint16_t EMPTY6;
  volatile uint16_t CR2;
  volatile uint16_t EMPTY7;
  volatile uint16_t SR;
} TIM_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t BR;
  volatile uint16_t EMPTY1;
  volatile uint16_t TxBUFR;
  volatile uint16_t EMPTY2;
  volatile uint16_t RxBUFR;
  volatile uint16_t EMPTY3;
  volatile uint16_t CR;
  volatile uint16_t EMPTY4;
  volatile uint16_t IER;
  volatile uint16_t EMPTY5;
  volatile uint16_t SR;
  volatile uint16_t EMPTY6;
  volatile uint16_t GTR;
  volatile uint16_t EMPTY7;
  volatile uint16_t TOR;
  volatile uint16_t EMPTY8;
  volatile uint16_t TxRSTR;
  volatile uint16_t EMPTY9;
  volatile uint16_t RxRSTR;
} UART_TypeDef;

typedef volatile struct
{
  volatile uint32_t EP0R;
  volatile uint32_t EP1R;
  volatile uint32_t EP2R;
  volatile uint32_t EP3R;
  volatile uint32_t EP4R;
  volatile uint32_t EP5R;
  volatile uint32_t EP6R;
  volatile uint32_t EP7R;
  volatile uint32_t EP8R;
  volatile uint32_t EP9R;
  volatile uint32_t EP10R;
  volatile uint32_t EP11R;
  volatile uint32_t EP12R;
  volatile uint32_t EP13R;
  volatile uint32_t EP14R;
  volatile uint32_t EP15R;
  volatile uint32_t CNTR;
  volatile uint32_t ISTR;
  volatile uint32_t FNR;
  volatile uint32_t DADDR;
  volatile uint32_t BTABLE;
} USB_TypeDef;

typedef volatile struct __attribute__ ((packed,aligned (2)))
{
  volatile uint16_t CR;
  volatile uint16_t EMPTY1;
  volatile uint16_t PR;
  volatile uint16_t EMPTY2;
  volatile uint16_t VR;
  volatile uint16_t EMPTY3;
  volatile uint16_t CNT;
  volatile uint16_t EMPTY4;
  volatile uint16_t SR;
  volatile uint16_t EMPTY5;
  volatile uint16_t MR;
  volatile uint16_t EMPTY6;
  volatile uint16_t KR;
} WDG_TypeDef;

typedef volatile struct __attribute__ ((packed))
{
  volatile uint8_t  SR;
  volatile uint8_t  EMPTY1[7];
  volatile uint8_t  CTRL;
  volatile uint8_t  EMPTY2[3];
  volatile uint8_t  MRH;
  volatile uint8_t  EMPTY3[3];
  volatile uint8_t  MRL;
  volatile uint8_t  EMPTY4[3];
  volatile uint8_t  TRH;
  volatile uint8_t  EMPTY5[3];
  volatile uint8_t  TRL;
  volatile uint8_t  EMPTY6[3];
  volatile uint8_t  PRH;
  volatile uint8_t  EMPTY7[3];
  volatile uint8_t  PRL;
} XTI_TypeDef;


/* IRQ vectors */
typedef struct
{
  uint32_t T0TIMI_IRQHandler;
  uint32_t FLASH_IRQHandler;
  uint32_t RCCU_IRQHandler;
  uint32_t RTC_IRQHandler;
  uint32_t WDG_IRQHandler;
  uint32_t XTI_IRQHandler;
  uint32_t USBHP_IRQHandler;
  uint32_t I2C0ITERR_IRQHandler;
  uint32_t I2C1ITERR_IRQHandler;
  uint32_t UART0_IRQHandler;
  uint32_t UART1_IRQHandler;
  uint32_t UART2_IRQHandler;
  uint32_t UART3_IRQHandler;
  uint32_t BSPI0_IRQHandler;
  uint32_t BSPI1_IRQHandler;
  uint32_t I2C0_IRQHandler;
  uint32_t I2C1_IRQHandler;
  uint32_t CAN_IRQHandler;
  uint32_t ADC12_IRQHandler;
  uint32_t T1TIMI_IRQHandler;
  uint32_t T2TIMI_IRQHandler;
  uint32_t T3TIMI_IRQHandler;
  uint32_t EMPTY1[3];
  uint32_t HDLC_IRQHandler;
  uint32_t USBLP_IRQHandler;
  uint32_t EMPTY2[2];
  uint32_t T0TOI_IRQHandler;
  uint32_t T0OC1_IRQHandler;
  uint32_t T0OC2_IRQHandler;
} IRQVectors_TypeDef;

/*===================================================================*/

/* Memory mapping */

#define RAM_BASE        0x20000000

#define FLASHR_BASE     0x40100000
#define FLASHPR_BASE    0x4010DFB0

#define EXTMEM_BASE     0x60000000
#define RCCU_BASE       0xA0000000
#define PCU_BASE        0xA0000040
#define APB1_BASE       0xC0000000
#define APB2_BASE       0xE0000000
#define EIC_BASE        0xFFFFF800

#define I2C0_BASE       (APB1_BASE + 0x1000)
#define I2C1_BASE       (APB1_BASE + 0x2000)
#define UART0_BASE      (APB1_BASE + 0x4000)
#define UART1_BASE      (APB1_BASE + 0x5000)
#define UART2_BASE      (APB1_BASE + 0x6000)
#define UART3_BASE      (APB1_BASE + 0x7000)
#define CAN_BASE        (APB1_BASE + 0x9000)
#define BSPI0_BASE      (APB1_BASE + 0xA000)
#define BSPI1_BASE      (APB1_BASE + 0xB000)
#define USB_BASE        (APB1_BASE + 0x8800)

#define XTI_BASE        (APB2_BASE + 0x101C)
#define GPIO0_BASE      (APB2_BASE + 0x3000)
#define GPIO1_BASE      (APB2_BASE + 0x4000)
#define GPIO2_BASE      (APB2_BASE + 0x5000)
#define ADC12_BASE      (APB2_BASE + 0x7000)
#define TIM0_BASE       (APB2_BASE + 0x9000)
#define TIM1_BASE       (APB2_BASE + 0xA000)
#define TIM2_BASE       (APB2_BASE + 0xB000)
#define TIM3_BASE       (APB2_BASE + 0xC000)
#define RTC_BASE        (APB2_BASE + 0xD000)
#define WDG_BASE        (APB2_BASE + 0xE000)

#define EMI_BASE        (EXTMEM_BASE + 0x0C000000)

/*===================================================================*/

/* IP data access */

#ifndef DEBUG
  #define ADC12 ((ADC12_TypeDef *)ADC12_BASE)

  #define APB1  ((APB_TypeDef *)(APB1_BASE+0x10))
  #define APB2  ((APB_TypeDef *)(APB2_BASE+0x10))

  #define BSPI0 ((BSPI_TypeDef *)BSPI0_BASE)
  #define BSPI1 ((BSPI_TypeDef *)BSPI1_BASE)

  #define CAN   ((CAN_TypeDef *)CAN_BASE)

  #define EIC   ((EIC_TypeDef *)EIC_BASE)

  #define EMI   ((EMI_TypeDef *)EMI_BASE)

  #define FLASHR  ((FLASHR_TypeDef *)FLASHR_BASE)
  #define FLASHPR ((FLASHPR_TypeDef *)FLASHPR_BASE)

  #define GPIO0 ((GPIO_TypeDef *)GPIO0_BASE)
  #define GPIO1 ((GPIO_TypeDef *)GPIO1_BASE)
  #define GPIO2 ((GPIO_TypeDef *)GPIO2_BASE)

  #define I2C0  ((I2C_TypeDef *)I2C0_BASE)
  #define I2C1  ((I2C_TypeDef *)I2C1_BASE)

  #define PCU   ((PCU_TypeDef *)PCU_BASE)

  #define RCCU  ((RCCU_TypeDef *)RCCU_BASE)

  #define RTC   ((RTC_TypeDef *)RTC_BASE)

  #define TIM0  ((TIM_TypeDef *)TIM0_BASE)
  #define TIM1  ((TIM_TypeDef *)TIM1_BASE)
  #define TIM2  ((TIM_TypeDef *)TIM2_BASE)
  #define TIM3  ((TIM_TypeDef *)TIM3_BASE)

  #define UART0 ((UART_TypeDef *)UART0_BASE)
  #define UART1 ((UART_TypeDef *)UART1_BASE)
  #define UART2 ((UART_TypeDef *)UART2_BASE)
  #define UART3 ((UART_TypeDef *)UART3_BASE)

  #define USB   ((USB_TypeDef *)USB_BASE)

  #define WDG   ((WDG_TypeDef *)WDG_BASE)

  #define XTI   ((XTI_TypeDef *)XTI_BASE)

  extern uint32_t IRQ_Vector_table[];
  #define IRQVectors ((IRQVectors_TypeDef *)IRQ_Vector_table)

#else   /* DEBUG */

  #ifdef _ADC12
  EXTERN ADC12_TypeDef *ADC12;
  #endif

  #ifdef _APB
  #ifdef _APB1
  EXTERN APB_TypeDef *APB1;
  #endif
  #ifdef _APB2
  EXTERN APB_TypeDef *APB2;
  #endif
  #endif

  #ifdef _BSPI
  #ifdef _BSPI0
  EXTERN BSPI_TypeDef *BSPI0;
  #endif
  #ifdef _BSPI1
  EXTERN BSPI_TypeDef *BSPI1;
  #endif
  #endif

  #ifdef _CAN
  EXTERN CAN_TypeDef *CAN;
  #endif

  #ifdef _EIC
  EXTERN EIC_TypeDef *EIC;
  #endif

  #ifdef _EMI
  EXTERN EMI_TypeDef *EMI;
  #endif

  #ifdef _FLASH
  EXTERN FLASHR_TypeDef *FLASHR;
  EXTERN FLASHPR_TypeDef *FLASHPR;
  #endif

  #ifdef _GPIO
  #ifdef _GPIO0
  EXTERN GPIO_TypeDef *GPIO0;
  #endif
  #ifdef _GPIO1
  EXTERN GPIO_TypeDef *GPIO1;
  #endif
  #ifdef _GPIO2
  EXTERN GPIO_TypeDef *GPIO2;
  #endif
  #endif

  #ifdef _I2C
  #ifdef _I2C0
  EXTERN I2C_TypeDef *I2C0;
  #endif
  #ifdef _I2C1
  EXTERN I2C_TypeDef *I2C1;
  #endif
  #endif

  #ifdef _PCU
  EXTERN PCU_TypeDef *PCU;
  #endif

  #ifdef _RCCU
  EXTERN RCCU_TypeDef *RCCU;
  #endif

  #ifdef _RTC
  EXTERN RTC_TypeDef *RTC;
  #endif

  #ifdef _TIM
  #ifdef _TIM0
  EXTERN TIM_TypeDef *TIM0;
  #endif
  #ifdef _TIM1
  EXTERN TIM_TypeDef *TIM1;
  #endif
  #ifdef _TIM2
  EXTERN TIM_TypeDef *TIM2;
  #endif
  #ifdef _TIM3
  EXTERN TIM_TypeDef *TIM3;
  #endif
  #endif

  #ifdef _UART
  #ifdef _UART0
  EXTERN UART_TypeDef *UART0;
  #endif
  #ifdef _UART1
  EXTERN UART_TypeDef *UART1;
  #endif
  #ifdef _UART2
  EXTERN UART_TypeDef *UART2;
  #endif
  #ifdef _UART3
  EXTERN UART_TypeDef *UART3;
  #endif
  #endif

  #ifdef _USB
  EXTERN USB_TypeDef *USB;
  #endif

  #ifdef _WDG
  EXTERN WDG_TypeDef *WDG;
  #endif

  #ifdef _XTI
  EXTERN XTI_TypeDef *XTI;
  #endif

  #ifdef _IRQVectors
  EXTERN IRQVectors_TypeDef *IRQVectors;
  #define IRQ_Vector_table ((uint32_t *) IRQVectors)
  #endif

#endif  /* DEBUG */

#endif  /* __71x_map_H */

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
