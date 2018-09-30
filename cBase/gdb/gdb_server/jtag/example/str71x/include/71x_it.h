/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : 71x_it.h
* Author             : MCD Application Team
* Date First Issued  : 05/16/2003
* Description        : Interrupt handlers
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

#ifndef _71x_IT_H
#define _71x_IT_H

#include "71x_lib.h"

extern void Default_IRQHandler  (void);
extern void T0TIMI_IRQHandler   (void);
extern void FLASH_IRQHandler    (void);
extern void RCCU_IRQHandler     (void);
extern void RTC_IRQHandler      (void);
extern void WDG_IRQHandler      (void);
extern void XTI_IRQHandler      (void);
extern void USBHP_IRQHandler    (void);
extern void I2C0ITERR_IRQHandler(void);
extern void I2C1ITERR_IRQHandler(void);
extern void UART0_IRQHandler    (void);
extern void UART1_IRQHandler    (void);
extern void UART2_IRQHandler    (void);
extern void UART3_IRQHandler    (void);
extern void BSPI0_IRQHandler    (void);
extern void BSPI1_IRQHandler    (void);
extern void I2C0_IRQHandler     (void);
extern void I2C1_IRQHandler     (void);
extern void CAN_IRQHandler      (void);
extern void ADC12_IRQHandler    (void);
extern void T1TIMI_IRQHandler   (void);
extern void T2TIMI_IRQHandler   (void);
extern void T3TIMI_IRQHandler   (void);
extern void HDLC_IRQHandler     (void);
extern void USBLP_IRQHandler    (void);
extern void T0TOI_IRQHandler    (void);
extern void T0OC1_IRQHandler    (void);
extern void T0OC2_IRQHandler    (void);

#endif /* _71x_IT_H */

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
