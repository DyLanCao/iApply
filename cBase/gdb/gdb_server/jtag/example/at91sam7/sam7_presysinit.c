/*
 * sam7_sysinit.c
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

#include "sam7_env.h"
#include "sam7.h"

static void SpuriousHandler(void);
static void DefaultInterruptHandler(void) __attribute__ ((noreturn)) ;

void sys_pre_init_hook(void); 

/*
 * Low level init routine called onece after reset.
 * The CPU is in System state.
 * A Stack exists but nither data nor bss is initialized yet.
 * This routine is called before entering sys_init_hook and main.
 */
void sys_pre_init_hook(void)
{
	uint32_t	cnt, val;
	
	val = MC_ASR;
	/* Set Flash Waite sate */
	//  Single Cycle Access at Up to 30 MHz
	//  if MCK = 48MHz I have 48 Cycle for 1 useconde
	MC_FMR = MC_FMR_FMCN(CYCLES_PER_MIKRO_SECOND) | MC_FMR_FWS(1);
	
	/* Watchdog Disable*/
	WDT_MR	= WDT_MR_WDDIS
		| WDT_MR_WDIDLEHLT
		| WDT_MR_WDDBGHLT;
	
	/* Set MCK at 47 923 200 */
	// Enabling the Main Oscillator:
	// SCK = 1/32768Hz = 30.51 uSeconde
	// Start up time = 6 * 8 / SCK = 56 * 30.51 = 1,46484375 ms
	CKGR_MOR = CKGR_MOR_OSCOUNT(6) | CKGR_MOR_MOSCEN;

	/* Wait the startup time */
	while(!(PMC_SR & PMC_SR_MOSCS))
		;
	
	// Setting PLL and divider:
	// - div by PLL_DIV  Fin = F_OSC / PLL_DIV
	// - Mul = PLL_MUL-1: Fout = Fin*PLL_MUL = 96 MHz
	// Field out NOT USED = 0
	// out = 0 (range 80 MHz - 160 MHz)
	// PLLCOUNT pll startup time estimate at : 0.844 ms
	// PLLCOUNT 28 = 0.000844 /(1/32768)
	CKGR_PLLR = CKGR_PLLR_DIV(PLL_DIV)
	          | CKGR_PLLR_COUNT(28)
		  | CKGR_PLLR_MUL(PLL_MUL-1);
	
	/* Wait the startup time */
	while(!(PMC_SR & PMC_SR_LOCK))
		;
	// Selection of Master Clock and Processor Clock
        // select the PLL clock divided by 2
	// MCK = 96 MHz / 2 = 48 MHz
	val = PMC_MCKR;
	PMC_MCKR = val | PMC_PRES_CLK_2 ;
	/* Wait until the MCLK is ready */
	while(!(PMC_SR & PMC_SR_MCKRDY))
		;
	// select the PLL as Master Clock
	val = PMC_MCKR;
	PMC_MCKR = val | PMC_CSS_PLL_CLK ;
	
	/* Wait until the MCLK is ready */
	while(!(PMC_SR & PMC_SR_MCKRDY))
		;

	// Set up the default interrupts handler vectors
	for (cnt=0;cnt < 31; cnt++)
		AIC_SVR[cnt] = (uint32_t) DefaultInterruptHandler;
	AIC_SPU  = (uint32_t) SpuriousHandler;
	return;
}

static void SpuriousHandler(void)
{
	return;
}

static void DefaultInterruptHandler(void)
{
	while(1)
		;
}

