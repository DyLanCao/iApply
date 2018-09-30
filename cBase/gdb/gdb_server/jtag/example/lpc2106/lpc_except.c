/*
 * lpc_except.c
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

#include "lpc_env.h"
#include "lpc21xx.h"


void exceptionUndef(void)					__attribute__ ((interrupt ("UNDEF")));
void exceptionSWI(int arg0, int arg1, int arg2, int arg4)	__attribute__ ((interrupt ("SWI")));
void exceptionPabort(void)					__attribute__ ((interrupt ("ABORT")));
void exceptionDabort(void)					__attribute__ ((interrupt ("ABORT")));
void exceptionIRQ(void)						__attribute__ ((interrupt ("IRQ")));
void exceptionFIQ(void)						__attribute__ ((interrupt ("FIQ")));

extern void swi_syscall(int id, int arg0, int arg1, int arg2);
static void swi_hndl_Enable_IRQ(void);
static void swi_hndl_Disable_IRQ(void);

/*
 *
 */
void exceptionIRQ(void)
{
	void (*intrfct)(void);
	/*   
	 * The Interrupt Service Routine code will come here.
	 * We read out the requestet Vector Address and call the function.
	 * A write must be performed on the VIC Vector Address Register 
	 * to update the VIC priority hardware. 
	 */
	intrfct = (void (*)(void))VICVADDR;
	intrfct();
	VICVADDR=0xff; 
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


