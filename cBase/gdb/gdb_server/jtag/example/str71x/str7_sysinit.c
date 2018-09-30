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
#include "71x_it.h"

static void pioa_init(void);
static void timer_init (void);

void sys_init_hook(void); 

/*
 * Low level init routine called once after reset.
 * The CPU is in System state.
 * This routine is called before entering main.
 */
void sys_init_hook(void)
{
	int i;

	/*****************************************************************
	 * EIC_INIT
	 * This Initialize the EIC as following :
	 *          - IRQ disabled	
	 *          - FIQ disabled
	 *          - IVR contain the common base address
	 *          - Current priority level equal to 0
	 *          - All channels are disabled
	 *          - All channels priority equal to 0
	 *          - All SIR registers contain offset to the related IRQ
	 *          table entry
	 *****************************************************************/
	
	EIC->ICR = 0; /* IRQ disabled; FIQ disabled */
	EIC->IER = 0; /* All channels are disabled */
	EIC->IPR = 0xFFFFFFFF; /* Clear all pending bits */
	EIC->FIR = 0x0000000C; /* Disable all FIQ channels interrupts and clear FIQ channels pending bits */
	EIC->CIPR = 0x00000000; /* Set the current priority level to zero */

	/* Write common Base of all IRQ's in IVR[31:16] */
	EIC->IVR = (uint32_t)IRQ_Vector_table & 0xFFFF0000;		
	
	/* 32 Channel to initialize */
	for(i=0;i<32;i++)
	{
		/* Write individual IVR[15:0] and prio level 0 (into each SIR) */
		EIC->SIR[i] = (((uint32_t)(&IRQ_Vector_table[i]) & 0xFFFF) << 16) | 0;
		/* Set up the interrupts handler vector */
		IRQ_Vector_table[i] = (uint32_t)Default_IRQHandler;
	}

	pioa_init();
	timer_init();

	EIC_IRQConfig(ENABLE); /* Enable interrupts on the EIC */
	return;
}

static void pioa_init(void)
{
	/*P0 all input*/
	GPIO_Config(GPIO0, 0xFFFF, GPIO_IPUPD_WP);	/* configure all GPIO0 pins as input mode with weak push/pull*/
	GPIO_WordWrite(GPIO0, 0xFFFF);			/* Set all GPIO0 pins to the weak pull up level */
	
	/*P1.8 LED output; all other input*/
	GPIO_Config(GPIO1, 0x0100, GPIO_OUT_PP);	/* configure GPIO1 pin P1.8 as push-pull output mode */
	GPIO_Config(GPIO0, 0xFeFF, GPIO_IPUPD_WP);	/* configure all GPIO0 pins as input mode with weak push/pull*/
	GPIO_WordWrite(GPIO1, 0xFFFF);			/* Set GPIO1 pin P1.8 to the high level and all other to weak pull up level*/
	return;
}


static void timer_init (void)
{
	/* Timer 0 Configuration */
	IRQVectors->T0TIMI_IRQHandler = (uint32_t)T0TIMI_IRQHandler;
	
	/* Set the Timer 0 interrupt channel priority of channel 0 level to 1 */
	EIC_IRQChannelPriorityConfig(T0TIMI_IRQChannel,1);

	/* Enable the interrupt on Timer 0 IRQ channel 0interrupts */
	EIC_IRQChannelConfig(T0TIMI_IRQChannel, ENABLE);
	
	/* Clear Initialize the overflow an output-compare-B flagTimer */
	TIM_FlagClear (TIM0, TIM_TOF | TIM_OCFB);
	TIM_Init (TIM0);
	
	/* Configure Timer Prescaler TimClk = PCLK2 / (PRESCALE+1) = 12MHz / (239+1) = 50kHz*/
	TIM_PrescalerConfig (TIM0,239);
	
	/* Full period = CNT / TimClk = 25000 / 50kHz = 0.5sec*/
	TIM_PWM_ModeConfig ( TIM0, 12500, TIM_HIGH, 25000, TIM_LOW, TIM_TIMING);
	TIM_ITConfig(TIM0,TIM_OCB_IT,ENABLE);	/* Enable Timer 0 Output Compare Channel B Interrupt */
	return;
}



