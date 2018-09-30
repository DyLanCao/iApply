/*
 * sam7_except.c
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
 * -- ARM mode code --
 */

#include "sam7_env.h"
#include "sam7.h"


void exceptionUndef(void)					__attribute__ ((interrupt ("UNDEF")));
void exceptionSWI(int arg0, int arg1, int arg2, int arg4)	__attribute__ ((interrupt ("SWI")));
void exceptionPabort(void)					__attribute__ ((interrupt ("ABORT")));
void exceptionDabort(void)					__attribute__ ((interrupt ("ABORT")));
void exceptionIRQ(void)						__attribute__ ((interrupt ("IRQ")));
void exceptionFIQ(void)						__attribute__ ((interrupt ("FIQ")));

extern void swi_syscall(int id, int arg0, int arg1, int arg2);
static void swi_hndl_Enable_IRQ(void);
static void swi_hndl_Disable_IRQ(void);

uint32_t undef_status,pabort_status,dabort_status;
uint32_t undef_addr,pabort_addr,dabort_addr;
uint32_t undef_aasr,pabort_aasr,dabort_aasr;

volatile int exceptionIrqCnt = 0;
volatile int exceptionFiqCnt = 0;

/*
 * Store LR
 */
void exceptionUndef(void)
{
	register uint32_t val;
	
	__asm__ __volatile__(
		"SUB    %0, LR, #0"
		:"=r" (val)
		:/*"r" (val)*/
		:/*"r0", "r1", "r2", "r3", */"r14"
	);
	undef_status = MC_ASR;
	undef_aasr = MC_AASR;
	undef_addr = val;
	while(1)
		;
}

/*
 * Store LR - 4 
 */
void exceptionPabort(void)
{
	// sub		lr, lr, #4	; 0x4
	register uint32_t val;
	
	__asm__ __volatile__(
		"SUB    %0, LR, #0"
		:"=r" (val)
		:/*"r" (val)*/
		:/*"r0", "r1", "r2", "r3", */"r14"
	);
	pabort_status = MC_ASR;
	pabort_aasr = MC_AASR;
	pabort_addr = val;
	while(1)
		;
}

/*
 * Store LR - 8 
 */
void exceptionDabort(void)
{
	// sub		lr, lr, #4	; 0x4
	// stmdb	sp!, {r0, r1, r2, r3, lr}

	register uint32_t val;
	__asm__ __volatile__(
		"SUB    %0, LR, #4"
		:"=r" (val)
		:/*"r" (val)*/
		:/*"r0", "r1", "r2", "r3", */"r14"
	);
	dabort_status = MC_ASR;
	dabort_aasr = MC_AASR;
	dabort_addr = val;
	while(1)
		;
}

/*
 *
 */
void exceptionIRQ(void)
{
	void (*intrfct)(void);
	/*   
	 * The Interrupt Service Routine code will come here.
	 * We read out the requestet Vector Address and call the function.
	 * A write must be performed on the End of Interrupt Controll Register 
	 * to update the AIC priority hardware. 
	 */
	intrfct = (void (*)(void))AIC_IVR;
	AIC_IVR = (uint32_t) &AIC_IVR;
	__asm__ ("mrs   lr, SPSR_cf");		// Save SPSR and intrfct (r0-r3)..
	__asm__ ("stmfd sp!, {lr}");	// ..  in IRQ stack
	__asm__ ("msr   CPSR_c, #0x13");	// SVC mode, interrupt enabled
	__asm__ ("stmfd sp!,{r0-r3,ip, lr}");
	__asm__ ("msr   CPSR_c, #0xD3");	// SVC mode, interrupt disable
	intrfct();
	exceptionIrqCnt++;
	__asm__ ("ldmia sp!,{r0-r3,ip, lr}");
	__asm__ ("msr   CPSR_c, #0xd2");	// Irq mod, interrupt disable
	__asm__ ("ldmia sp!,{lr}");	// Restore SPSR_irq and r0-r3 from IRQ stack
	__asm__ ("msr  SPSR_cxsf, lr");
	AIC_EOICR = (uint32_t)&AIC_EOICR;
	return;
}

/*
 *
 */
void exceptionFIQ(void)
{
	void (*intrfct)(void);
	/*   
	 * The Interrupt Service Routine code will come here.
	 * We read out the requestet Vector Address and call the function.
	 * A write must be performed on the End of Interrupt Controll Register 
	 * to update the AIC priority hardware. 
	 */
	intrfct = (void (*)(void))AIC_FVR;
	intrfct();
	AIC_EOICR=0; 
	exceptionFiqCnt++;
	return;
}


/*
 *
 */
void exceptionSWI(int arg0, int arg1, int arg2, int arg4)
{
	register uint32_t val;
	
	__asm__ __volatile__(
		"Mrs	%0,spsr		\n" /*Move SPSR into GP Register*/
		"Tst	%0,#0x20	\n" /*Test for Thumb mode syscall*/
		"Ldrneh	%0,[lr,#-2]	\n" /*T bit set, so load halfword (Thumb)*/
		"Bicne 	%0,%0,#0xFF00	\n" /*only the lower 8 Bits are usable*/
		"Ldreq 	%0,[lr,#-4]	\n" /*T bit clear, so load word (ARM)*/
		"Biceq 	%0,%0,#0xFF000000"  /*only the lower 24 Bits are usable*/
		:"=r" (val)
		:
		:"lr"
	);
	switch(val & 0xF)
	{
	case 0:
		swi_syscall(arg0, arg1, arg2, arg4);
		break;
	case 1:
		swi_hndl_Enable_IRQ();
		break;
	case 2:
		swi_hndl_Disable_IRQ();
		break;
	default:
		break;
	}
	return;
}


static void swi_hndl_Enable_IRQ(void)
{
	register uint32_t val;
	
	__asm__ __volatile__(
		"Mrs    %0, spsr	\n"
		"Bic    %0,%0,#0x80	\n"
		"Msr    spsr_cf,%0	"
		:"=r" (val)
		:
	);
	return;
}

static void swi_hndl_Disable_IRQ(void)
{
	register uint32_t val;
	
	__asm__ __volatile__(
		"Mrs    %0, spsr	\n"
		"Orr    %0,%0,#0x80	\n"
		"Msr    spsr_cf,%0	"
		:"=r" (val)
		:/*"r" (val)*/
		/*:"r0", "r1", "r2", "r3"*/
	);
	return;
}


