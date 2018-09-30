/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : rtc.h
* Author             : MCD Application Team
* Date First Issued  : 20/05/2003
* Description        : This file contains all the functions prototypes for the
*                      RTC software library.
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
#ifndef __RTC_H
#define __RTC_H

#include "71x_map.h"

typedef enum
{
  RTC_GIR  = 0x08,
  RTC_OWIR = 0x04,
  RTC_AIR  = 0x02,
  RTC_SIR  = 0x01
} RTC_FLAGS;

typedef enum
{
  RTC_GIT  = 0x08,
  RTC_OWIT = 0x04,
  RTC_AIT  = 0x02,
  RTC_SIT  = 0x01,
  RTC_NONE = 0x00
} RTC_IT;

/*******************************************************************************
* Function Name  : RTC_Delay
* Description    : This routine is used to insert a delay
* Input          : None
* Return         : None
*******************************************************************************/
extern void RTC_Delay( void );

/*******************************************************************************
* Function Name  : RTC_CounterClear
* Description    : This routine is used to clear the RTC counter
* Input          : None
* Return         : None
*******************************************************************************/
extern void RTC_CounterClear (void);
/*******************************************************************************
* Function Name  : RTC_CounterValue
* Description    : This routine is used to get the RTC counter value
* Input          : None
* Return         : The current counter value.
*******************************************************************************/
static inline uint32_t RTC_CounterValue (void);
static inline uint32_t RTC_CounterValue (void)
{
	return ( (uint32_t)RTC->CNTH << 16 ) | RTC->CNTL;
}
/*******************************************************************************
* Function Name  : RTC_CounterConfig
* Description    : This routine is used to update the RTC counter value
* Input          : The new counter value.
* Return         : None
*******************************************************************************/
extern void RTC_CounterConfig (uint32_t CounterValue);

/*******************************************************************************
* Function Name  : RTC_PrescalerValue
* Description    : This routine is used to get the RTC prescaler Value
* Input          : None
* Return         : an uint32_t value that holds the prescaler Value.
*******************************************************************************/
static inline uint32_t RTC_PrescalerValue (void);
static inline uint32_t RTC_PrescalerValue (void)
{
	return ( (uint32_t)(RTC->PRLH & 0x000F) << 16 ) | RTC->PRLL;
}

/*******************************************************************************
* Function Name  : RTC_PrescalerConfig
* Description    : This routine is used to set the Prescaler Value
* Input          : The New prescaler Value
* Return         : None
*******************************************************************************/
extern void RTC_PrescalerConfig (uint32_t Xprescaler);

/*******************************************************************************
* Function Name  : RTC_AlarmValue
* Description    : This routine is used to get the RTC alarm Value
* Input          : None
* Return         : an uint32_t value that holds the Real Time clock alarm time .
*******************************************************************************/
static inline uint32_t RTC_AlarmValue (void);
static inline uint32_t RTC_AlarmValue (void)
{
	return ( (uint32_t)RTC->ALRH << 16 ) | RTC->ALRL;
}

/*******************************************************************************
* Function Name  : RTC_AlarmConfig
* Description    : This routine is used to set the RTC alarm Value
* Input          : an uint32_t value that holds the Real Time clock alarm time .
* Return         : None
*******************************************************************************/
extern void RTC_AlarmConfig (uint32_t Xalarm);

/*******************************************************************************
* Function Name  : RTC_FlagStatus
* Description    : This routine is used to chek the RTC flag status
* Input          : an RTC flag
* Return         : Set or RESET
*******************************************************************************/
static inline FlagStatus RTC_FlagStatus (RTC_FLAGS Xflag);
static inline FlagStatus RTC_FlagStatus (RTC_FLAGS Xflag)
{
	return ( RTC->CRL & Xflag ) == 0 ? RESET : SET;
}

/*******************************************************************************
* Function Name  : RTC_FlagClear
* Description    : This routine is used to clear the RTC flags
* Input          : an RTC flag
* Return         : None
*******************************************************************************/
extern void RTC_FlagClear (RTC_FLAGS Xflag);

/*******************************************************************************
* Function Name  : RTC_ITConfig
* Description    : This routine is used to configure the RTC interrupts
* Input 1        : an RTC interrupt
* Input 2        : Enable or Disable
* Return         : None
*******************************************************************************/
static inline void RTC_ITConfig (RTC_IT Xrtcit, FunctionalState NewState);
static inline void RTC_ITConfig (RTC_IT Xrtcit, FunctionalState NewState)
{
  if (NewState == ENABLE) RTC->CRH |= Xrtcit; else RTC->CRH &= ~Xrtcit;
}

/*******************************************************************************
* Function Name  : RTC_ITStatus
* Description    : This routine is used to get the RTC interrupts status
* Input          : an RTC interrupt
* Return         : Enable or Disable
*******************************************************************************/
static inline FunctionalState RTC_ITStatus (RTC_IT Xrtcit);
static inline FunctionalState RTC_ITStatus (RTC_IT Xrtcit)
{
  return ( RTC->CRH & Xrtcit ) == 0 ? DISABLE : ENABLE;
}

/*******************************************************************************
* Function Name  : RTC_ITClear
* Description    : This routine is used to clear the RTC interrupts
* Input          : an RTC interrupt
* Return         : None
*******************************************************************************/
extern void RTC_ITClear (RTC_IT Xrtcit);

/*******************************************************************************
* Function Name  : RTC_EnterCfgMode
* Description    : This routine is used to enter in the Configuration Mode
* Input          : None
* Return         : None
*******************************************************************************/
extern void RTC_EnterCfgMode(void);

/*******************************************************************************
* Function Name  : RTC_ExitCfgMode
* Description    : This routine is used to exit from the Configuration Mode
* Input          : None
* Return         : None
*******************************************************************************/
extern void RTC_ExitCfgMode(void);

/*******************************************************************************
* Function Name  : RTC_WaitForLastTask
* Description    : This routine is waits for the last task completetion
* Input          : None
* Return         : None
*******************************************************************************/
extern void RTC_WaitForLastTask(void);

#endif // __RTC_H

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
