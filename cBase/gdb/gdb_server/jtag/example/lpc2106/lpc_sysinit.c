/*
 * lpc_sysinit.c
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

#include "lpc_env.h"
#include "lpc21xx.h"


extern void timer1_irq(void);
void sys_init_hook(void); 

/*
 * Low level init routine called onece after reset.
 * The CPU is in System state.
 * This routine is called before entering main.
 */
void sys_init_hook(void)
{
	/*setup PLL*/
	PLLCFG  = (PSEL << PSEL_BIT_POS) | (MSEL << MSEL_BIT_POS);
	PLLCON  = (1 << PLLE_BIT_POS);
	PLLFEED = 0xaa;
	PLLFEED = 0x55;
	/*wait until PLL is Locked*/
	while( (PLLSTAT & (1 << PLOCK_BIT_POS)) == 0 )
		;
	/*connect PLL*/
	PLLCON  = (1 << PLLC_BIT_POS)|(1 << PLLE_BIT_POS);
	PLLFEED = 0xaa;
	PLLFEED = 0x55;

	/* 
	 * Enabling MAM and setting number of clocks used for 
	 * Flash memory fetch  
	 */
	MAMCR  = MAM_MODE_DISABLE; 
	MAMTIM = MAMTIM_MAX;
	MAMCR  = MAM_MODE_FULL; 
	
	/*
	 * Setting peripheral Clock (pclk) to System   
	 * Clock (cclk)  
	 */
	//VPBDIV=VPB_PCLK_IS_SAME_AS_CCLK;

	/* Initialize GPIO */ 
	IODIR=0x80;//=0xFFFF; 
	IOSET=0x80;//=0xFFFF;

	/* Initialize Timer 1 */ 
	T1_TCR=0x0; 
	T1_TC=0x0; 
	T1_PR=0x0; 
	T1_PC=0x0;

	/* End user has to fill in the match value */  
	T1_MR0=CCLK/4; // 1 sec

	/* Reset and interrupt on match  */ 
	T1_MCR= (1<<T1_MCR_BIT_POS_RESET_ON_MR(0))
	      | (1<<T1_MCR_BIT_POS_INTERRUPT_ON_MR(0)) ;

	/* Initialize VIC */ 
	VICINTSEL=0x0; // all sources using IRQ
	/* Timer 1 selected as IRQ */  
	VICINTEN= (1<<IRQ_TIMER1); //=0x20;
	/* Timer 1 interrupt enabled */  
	VICCNTL0=VECTORED_IRQ_TIMER1; // 0x25;

	/* Address of the ISR */  
	VICVADDR0=(uint32_t)timer1_irq;   

	return;
}

