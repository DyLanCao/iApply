/*
 * sam7_irq.c
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
 */
#include "sam7_env.h"
#include "sam7.h"

#define MAX_SAMPLE_BUFFER_SIZE (8)

static int period_cnt = 0;

uint16_t sampleBuffer[MAX_SAMPLE_BUFFER_SIZE]; 
int samplePos = 0;

void timer0_irq(void);
void adc_irq(void);
void pwm_irq(void);
void sys_irq_dispatcher(void);

/*
 *
 */
void timer0_irq(void)
{
	/*   
	 * The Timer Interrupt Service Routine code will come here. 
	 * The interrupt needs to be cleared in Timer0.
	 * Here the user could  blink a LED or toggle some 
	 * port pins as an indication of being in the ISR  
	 */  
	if(TC0_SR & TC_SR_CPCS)
	{
		if( (PIO_PDSR & LED3) != 0 )
			PIO_CODR = LED3;
		else
			PIO_SODR = LED3;
	}
	return;
}


void adc_irq(void)
{
	uint32_t data;
	
	if(ADC_SR & ADC_EOC(5))
	{
		/*move sample data into buffer*/
		//data = PWM_ISR;
		data = ADC_CDR5;
		sampleBuffer[samplePos] = data & 0xFFFF;
		if(++samplePos >= MAX_SAMPLE_BUFFER_SIZE)
			samplePos = 0;
		if((++period_cnt & 0x1FF) == 0) // 0.26 sec interval
		{
			if( (PIO_PDSR & LED2) != 0 )
				PIO_CODR = LED2;
			else
				PIO_SODR = LED2;
		}
	}
	return;
}

void pwm_irq(void)
{
	if((PWM_SR & PWM_CHID0) && (PWM_ISR & PWM_CHID0))
	{
		/* start ADC conversion */
		ADC_CR = ADC_CR_START;
		PWM_CUPD0 = 24; // duty cycle 50%
	}
	return;
}


void sys_irq_dispatcher(void)
{
	return;
}

