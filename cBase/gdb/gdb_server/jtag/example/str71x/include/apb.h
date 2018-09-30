/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : apb.h
* Author             : MCD Application Team
* Date First Issued  : 05/30/2003
* Description        : This file contains all the functions prototypes for the
*                      APB bridge software library.
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
#ifndef __APB_H
#define __APB_H

#include "71x_map.h"

/* APB1 Peripherals */
#define  I2C0_Periph      0x0001
#define  I2C1_Periph      0x0002
// res			  
#define  UART0_Periph     0x0008
#define  UART1_Periph     0x0010
#define  UART2_Periph     0x0020
#define  UART3_Periph     0x0040
#define  USB_Periph       0x0080
#define  CAN_Periph       0x0100
#define  BSPI0_Periph     0x0200
#define  BSPI1_Periph     0x0400
// res
// res			  
#define  HDLC_Periph      0x2000
#define  ALL_APB1_Periph  0x27FB

/* APB2 Peripherals */
#define  XTI_Periph       0x0001
// res			  
#define  GPIO0_Periph     0x0004
#define  GPIO1_Periph     0x0008
#define  GPIO2_Periph     0x0010
// res			  
#define  ADC12_Periph     0x0040
#define  CKOUT_Periph     0x0080
#define  TIM0_Periph      0x0100
#define  TIM1_Periph      0x0200
#define  TIM2_Periph      0x0400
#define  TIM3_Periph      0x0800
#define  RTC_Periph       0x1000
#define  WDG_Periph       0x2000
#define  EIC_Periph       0x4000
#define  ALL_APB2_Periph  0x7FDD

/*******************************************************************************
* Function Name  : APB_ClockConfig
* Description    : Enables/Disables the Clock gating for peripherals on the APB
*                  bridge passed in parameters.
* Input          : APBx ( APB1 or APB2 )
*                  NewState ENABLE or DISABLE
*                  NewValue (uint16_t)
* Return         : None
*******************************************************************************/
static inline void APB_ClockConfig ( APB_TypeDef *, FunctionalState, uint16_t );
static inline void APB_ClockConfig ( APB_TypeDef *APBx,
                              FunctionalState NewState,
                              uint16_t NewValue )
{
  if (NewState == ENABLE) APBx->CKDIS &= ~NewValue;
    else APBx->CKDIS |= NewValue;
}

/*******************************************************************************
* Function Name  : APB_SwResetConfig
* Description    : Enables/Disables the software Reset for peripherals on the APB
*                  bridge passed in parameters.
* Input          : APBx ( APB1 or APB2 )
*                  NewState ENABLE or DISABLE
*                  NewValue (uint16_t)
* Return         : None
*******************************************************************************/
static inline void APB_SwResetConfig ( APB_TypeDef *, FunctionalState, uint16_t );
static inline void APB_SwResetConfig ( APB_TypeDef *APBx,
                                FunctionalState NewState,
                                uint16_t NewValue )
{
  if (NewState == ENABLE) APBx->SWRES |= NewValue;
    else APBx->SWRES &= ~NewValue;
}

#endif	// __APB_H

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
