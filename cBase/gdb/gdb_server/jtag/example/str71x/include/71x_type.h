/******************** (C) COPYRIGHT 2003 STMicroelectronics ********************
* File Name          : 71x_type.h
* Author             : MCD Application Team
* Date First Issued  : 05/16/2003
* Description        : Common data types
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
#ifndef _71x_type_H
#define _71x_type_H

#include "inttypes.h"
/*===================================================================*/
typedef enum { FALSE = 0, TRUE } BoolStatus;
/*===================================================================*/
typedef enum { RESET = 0, SET } FlagStatus;
/*===================================================================*/
typedef enum { DISABLE = 0, ENABLE } FunctionalState;
/*===================================================================*/
typedef enum { INDIRECT = 0, DIRECT } RegisterAccess;
/*===================================================================*/

#endif /* _71x_type_H */

/******************* (C) COPYRIGHT 2003 STMicroelectronics *****END OF FILE****/
