/*
 * str7_sysinit.c
 *
 * Copyright (C) 2005
 *  
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Written by Thomas Klein <ThRKlein@users.sf.net>, 2005.
 *
 * -- ARM mode --
 */

#include "71x_conf.h"
#include "71x_lib.h"

/*****************************************************************/
/* define remapping                                              */
/* If you need to remap memory before entring the main program   */
/* uncomment next ligne                                          */
/*****************************************************************/
// #define  remapping
/*****************************************************************/
/* Then define which memory to remap to address 0x00000000       */
/*  Uncomment next line if you want to remap RAM                 */
/*****************************************************************/
// #define  remap_ram
/*****************************************************************/
/*  Uncomment next line if you want to remap FLASH               */
/*****************************************************************/
// #define  remap_flash
/*****************************************************************/
/*  Uncomment next line if you want to remap the external memory */
/*****************************************************************/
// #define  remap_ext

#define FLASH_mask          0x0000    /* to remap FLASH at 0x0  */
#define RAM_mask            0x0002    /* to remap RAM at 0x0    */
#define EXTMEM_mask         0x0003    /* to remap EXTMEM at 0x0 */


void sys_preinit_hook(void); 

/*
 * Low level init routine called onece after reset.
 * The CPU is in System state.
 * A Stack exists but nither data nor bss is initialized yet.
 * This routine is called before entering sys_init_hook and main.
 */
void sys_preinit_hook(void)
{
	register uint32_t cnt;

	/*********************************
	 * RCCU peripheral configuration 
	 * Input external oscilator with 4 MHz
	 * MCLK  -> 48 MHz
	 * PCLK1 -> 24 MHz
	 * PCLK2 -> 12 MHz
	 *********************************/

	RCCU_RCLKSourceConfig (RCCU_CLOCK2);	/* Select CLK2 as RCLK clock */
	RCCU_Div2Config (DISABLE);		/* CLK2 = CK = 4MHz */
	RCCU_FCLKConfig (RCCU_RCLK_2);		/* Configure FCLK (APB1 fast peripherals PCLK1) = RCLK /2 */
	RCCU_PCLKConfig (RCCU_RCLK_4);		/* Configure PCLK (APB2 peripherals PCLK2) = RCLK /4 */
	RCCU_MCLKConfig (RCCU_DEFAULT);		/* Configure MCLK clock for the CPU, RCCU_DEFAULT = RCLK /1 */
	RCCU_PLL1Config (RCCU_PLL1_Mul_24, RCCU_Div_2) ; /* Configure the PLL1 = CLK2 * 24 / 2 */
	
	/* Wait for PLL to lock */
	while(RCCU_FlagStatus(RCCU_PLL1_LOCK)==RESET)
		;
	
	RCCU_RCLKSourceConfig (RCCU_PLL1_Output); /* Select PLL1_Output as RCLK clock */
	
	/*********************************************************
	 * PERIPHERAL_INIT
	 * Description    : This reset all device peripherals.
	 *********************************************************/
	APB1->CKDIS = ALL_APB1_Periph; // Clock Disabling for all APB1 peripherals
	APB2->CKDIS = ALL_APB2_Periph; // Clock Disabling for all APB2 peripherals
	APB1->SWRES = ALL_APB1_Periph; // Keep all APB1 peripherals under reset 
	APB2->SWRES = ALL_APB2_Periph; // Keep all APB2 peripherals under reset 
	
	/* Wait that the selected macrocells enter reset */
	for(cnt=4*10; cnt>0; cnt--)
		;
	
	APB1->SWRES = 0; // Enable all all APB1 peripherals
	APB2->SWRES = 0; // Enable all all APB2 peripherals
	APB1->CKDIS = 0; // Clock Enabling for all APB1 peripherals
	APB2->CKDIS = 0; // Clock Enabling for all APB2 peripherals

	/* Wait that the selected macrocells exit from reset */
	for(cnt=4*10; cnt>0; cnt--)
		;
	
        /***********************************************************************
	 * EMI_INIT
	 * Description    : This Initialise EMI bank 1: 16-bit 7 wait state
	 ***********************************************************************/
	GPIO2->PC0 |= 0x0F; // Configure P2.0 -> 3 in AF_PP mode
	GPIO2->PC1 |= 0x0F;
	GPIO2->PC2 |= 0x0F;
	
	/* Enable bank 1 16-bit 7 wait state */
	EMI->BCON1 = (EMI_WAITSTATE(6) | (EMI_ENABLE) | (EMI_SIZE_16)); 

	
	/*******************************************************************************
	 * REMAPPING
	 * Description  : Remapping  memory whether RAM,FLASH or External memory
	 *                at Address 0x0 after the application has started executing.
	 *                Remapping is generally done to allow RAM  to replace FLASH
	 *                or EXTMEM at 0x0.
	 *                the remapping of RAM allow copying of vector table into RAM
	 *******************************************************************************/
#ifdef	remapping
#ifdef	remap_flash
	PCU->BOOTCR = (PCU->BOOTCR & 0xFFFFC) | FLASH_mask;
#endif
#ifdef	remap_ram
	PCU->BOOTCR = (PCU->BOOTCR & 0xFFFFC) | RAM_mask;
#endif
#ifdef	remap_ext
	PCU->BOOTCR = (PCU->BOOTCR & 0xFFFFC) | EXTMEM_mask;
#endif
#endif
	return;
}


