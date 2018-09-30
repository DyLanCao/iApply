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


extern void timer0_irq(void);
extern void adc_irq(void);
extern void pwm_irq(void);
extern void sys_irq_dispatcher(void);

static void pioa_init(void);
static void timer_init (void);
static void pwm_init (void);
static void adc_init (void);

void sys_init_hook(void); 

/*
 * Low level init routine called once after reset.
 * The CPU is in System state.
 * This routine is called before entering main.
 */
void sys_init_hook(void)
{
	/* Set up the interrupts handler vector */
	//AIC_IDCR = 1<<PID_SYS_IRQ;
	AIC_SVR[PID_SYS_IRQ] = (uint32_t) sys_irq_dispatcher;
	AIC_SMR[PID_SYS_IRQ] = AIC_SMR_SRCTYPE_RISING_EDGE
			     | AIC_SMR_PRIOR(0);
	AIC_ICCR = 1<<PID_SYS_IRQ;
	AIC_IECR = 1<<PID_SYS_IRQ;
	
	pioa_init();
	timer_init();
	adc_init();
	pwm_init();
	return;
}

static void pioa_init(void)
{
	/* Enable the clock of the PIOA, to be able to input data*/
	PMC_PCER = PMC_PCER_PID(PID_PIOA);
	
	/* configure the PIO Lines LED3 being outputs. */
	PIO_PER = LED3 | LED2;
	PIO_OER = LED3 | LED2;
	/* Set the LED's to switch it off */
	PIO_SODR = LED3 | LED2;
	
	/* configure the PWM Channel 0 output */
	PIO_ASR = PIO_A_PWM0; // assign to Function A
	PIO_PDR = PIO_A_PWM0; // Disable Output Function
	
	/* no need to configure the ADC input; we are useing fixed ports AD4 and AD5 */
	return;
}


static void timer_init (void)
{
	/* First, enable the clock of the TIMER */
	PMC_PCER = PMC_PCER_PID(PID_TC0);
	
	/* Disable the clock and the interrupts */
	TC0_CCR = TC_CCR_CLKDIS ;
	TC0_IDR = TC_IDR_ALL ;
	
	/* Clear status bit (and check that clock is disabled)*/
	while ((TC0_SR & TC_SR_CLKSTA) != 0)
		;
	
	/* Set the Mode of the Timer Counter */
	// - capture mode
	// - trigger on capture comprare (using register C)
	// - TCLK = MCK / 1025 = 47,9232 MHz / 1024 = 46800 Hz
	TC0_CMR = TC_CMR_CAPTURE_MODE
		| TC_CMR_CPCTRG
		| TC_CMR_TCCLKS_TIMER_CLOCK5_MCK_1024;
	
	// set 2 Hz interval
	TC0_RC = 23399; 
	
	/* setup interrupt handler */
	AIC_SVR[PID_TC0] = (uint32_t) timer0_irq;
	AIC_SMR[PID_TC0] = AIC_SMR_SRCTYPE_LOW_LEVEL
			 | AIC_SMR_PRIOR(2);
	AIC_ICCR = 1<<PID_TC0;
	
	/* enable interrupt */
	TC0_IER = TC_IER_CPCS;
	AIC_IECR = 1<<PID_TC0;

	return;
}


static void pwm_init (void)
{
	/* First, enable the clock of the PWM */
	PMC_PCER = PMC_PCER_PID(PID_PWMC);
	
	/*config for clock generation if DIVA and DIVB are required*/
	PWM_MR = 0; // CLKA and CLKB switched off
	
	/*Select clock for channel*/
	PWM_CMR0 = PWM_CMR_CPRE_MCK_512 // Prescale = MCK/512 = 48MHz/512 = 94kHz
					// Channel Alignment (0 = period left aligned)
		 		;	// Channel Polarity (0 = output waveform starts at low level)
	PWM_CPRD0 = 48; // Periode = 48/94kHz = 510usec
	PWM_CDTY0 = 24; // duty cycle 50%
	
	/* setup interrupt handler */
	AIC_SVR[PID_PWMC] = (uint32_t) pwm_irq;
	AIC_SMR[PID_PWMC] = AIC_SMR_SRCTYPE_LOW_LEVEL
			 | AIC_SMR_PRIOR(1);
	AIC_ICCR = 1<<PID_PWMC;
	
	/* enable interrupt */
	PWM_IER = PWM_CHID0; // Enable interrupt 
	AIC_IECR = 1<<PID_PWMC;
	PWM_ENA = PWM_CHID0; // Enable output for channel
	return;
}

static void adc_init (void)
{
	/* First, enable the clock of the ADC */
	PMC_PCER = PMC_PCER_PID(PID_ADC);
	
	/* Reset ADC */
	ADC_CR = ADC_CR_SWRST;
	
	/* setup ADC Clock's */
	ADC_MR  = ADC_MR_PRESCAL(4)		// ADCClock = MCK / ( (PRESCAL+1) * 2 ) = 48 MHz /((4 + 1)*2) = 4.8MHz
		| ADC_MR_STARTUP(11)		// Startup Time = (STARTUP+1) * 8 / ADCClock = 12*8/4.8 MHz = 20 usec
		| ADC_MR_SHTIM(9)		// Sample and Hold Time = (SHTIM+1) / ADCClock = 10 / 4.8 MHz = 2 usec
		| ADC_MR_TRGSEL_TIOA_TC_CHAN2	// reserve timer channel 2 for hardware trigger
		;				// disable hardware trigger 
	
	/* select active channel */
	ADC_CHER = ADC_CH(5);
	
	/* setup interrupt handler */
	AIC_SVR[PID_ADC] = (uint32_t) adc_irq;
	AIC_SMR[PID_ADC] = AIC_SMR_SRCTYPE_LOW_LEVEL
			 | AIC_SMR_PRIOR(1);
	AIC_ICCR = 1<<PID_ADC;
	
	/* enable interrupt */
	ADC_IER = ADC_EOC(5);
	AIC_IECR = 1<<PID_ADC;

	return;
}

