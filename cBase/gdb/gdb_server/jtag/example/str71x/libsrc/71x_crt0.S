/*
 *   Copyright 1995, 1996, 1998, 1999, 2000, 2003, 2004, 2005
 *   Free Software Foundation, Inc.
 *
 * This file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * In addition to the permissions in the GNU General Public License, the
 * Free Software Foundation gives you unlimited permission to link the
 * compiled version of this file into combinations with other programs,
 * and to distribute those combinations without any restriction coming
 * from the use of this file.  (The General Public License restrictions
 * do apply in other respects; for example, they cover modification of
 * the file, and distribution when not linked into a combine
 * executable.)
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * merge of:
 *   libgcc routines for ARM cpu.
 *   Division routines, written by Richard Earnshaw, (rearnsha@armltd.co.uk)
 *   newlib crt0.S version
 *   71x_init.s, STMicroelectronics, MCD Application Team (V3.0)
 *******************************************************************************/
 
/*The top of stack is given the loader (see coresponding script)*/
.equ ABORT_STACK_SIZE,	0x10	/* 16 Byte undef instr, data abort and prefetch abort using the same stack*/
.equ FIQ_STACK_SIZE,	0x20	/* 32 Byte FIQ - stack*/
.equ IRQ_STACK_SIZE,	0x100	/*256 Byte IRQ - stack*/
.equ SVC_STACK_SIZE,	0x100	/*256 Byte SWI - stack*/

.equ USERMODE,	0x10
.equ FIQMODE,	0x11
.equ IRQMODE,	0x12
.equ SVCMODE,	0x13
.equ ABORTMODE,	0x17
.equ UNDEFMODE,	0x1b
.equ SYSMODE,	0x1f
.equ MODEMASK,	0x1f
.equ NOINT,	0xc0


/* We need to know what prefix to add to function names.  */

#ifndef __USER_LABEL_PREFIX__
#error  "__USER_LABEL_PREFIX__ not defined"
#endif

/* ANSI concatenation macros.  */

#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b
#define FUNCTION( name ) CONCAT (__USER_LABEL_PREFIX__, name)


.section .vector,"ax",%progbits
	.code 32
	.align 	0

	.global Exception_Undef
	.global Exception_SWI
	.global Exception_ProgramAbort
	.global Exception_DataAbort
	.global Exception_IRQ
	.global Exception_FIQ

	.global Entry_VectorTable
	.type	Entry_VectorTable,function
Entry_VectorTable:	ldr pc,.LHandlerStart  /*Reset entry*/
Exception_Undef:	ldr pc,.LHandlerUndef  /*b HandleUndef */
Exception_SWI:		ldr pc,.LHandlerSWI    /*b HandleSWI  */
Exception_ProgramAbort:	ldr pc,.LHandlerPabort /*b HandlePabort */
Exception_DataAbort:	ldr pc,.LHandlerDabort /*b HandleDabort */
			ldr pc,.LBlockException/*Reserved */
Exception_IRQ:		ldr pc,.LHandlerIRQ    /*b HandlerIRQ */
Exception_FIQ:		ldr pc,.LHandlerFIQ    /*b HandlerFIQ */

	.global exception_VectorTable_LookupTable
exception_VectorTable_LookupTable:
.LHandlerStart:		.word	_start
.LHandlerUndef:		.word	exceptionUndef
.LHandlerSWI:		.word	exceptionSWI
.LHandlerPabort:	.word	exceptionPabort
.LHandlerDabort:	.word	exceptionDabort
.LBlockException:	b .
.LHandlerIRQ:		.word	exceptionIRQ
.LHandlerFIQ:		.word	exceptionFIQ

.set	exceptionUndef,		.LBlockException
.set	exceptionSWI,		.LBlockException
.set	exceptionPabort,	.LBlockException
.set	exceptionDabort,	.LBlockException
.set	exceptionIRQ,		.LBlockException
.set	exceptionFIQ,		.LBlockException

.weak	exceptionUndef
.weak	exceptionSWI
.weak	exceptionPabort
.weak	exceptionDabort
.weak	exceptionIRQ
.weak	exceptionFIQ

	/*********
	 * start *
	 *********/
.section .init
	.code 32
	.align 	0
	.global	_start
	.global	start
	.func start,_start
start:
_start:
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Annotation for EABI unwinding tables.  */
	.fnstart
#endif
         LDR     pc, =.LNextInst
.LNextInst:
		NOP		// Wait for OSC stabilization
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		
	/* Start by setting up a stack */
	/* Set up the stack pointer to a fixed value */

	ldr	r3, =stack_origin
	cmp	r3, #0
	beq	.
	
	/* Note: This 'mov' is essential when starting in User, and ensures we
		 always get *some* sp value for the initial mode, even if we 
		 have somehow missed it below (in which case it gets the same
		 value as FIQ - not ideal, but better than nothing.) */
	mov	sp, r3
	mrs	r2, CPSR
	tst	r2, #0x0F	/* Test mode bits - in User of all are 0 */
	beq	.		/* "eq" means r2 AND #0x0F is 0 -- so block executing here*/
	msr     CPSR_c, #(UNDEFMODE|NOINT)	/* Undefined mode, interrupts disabled */
	mov	sp, r3

	msr     CPSR_c, #(ABORTMODE|NOINT)	/* Abort mode, interrupts disabled */
	mov	sp, r3
	sub	r3, r3, #ABORT_STACK_SIZE

	msr     CPSR_c, #(FIQMODE|NOINT)	/* FIRQ mode, interrupts disabled */
	mov 	sp, r3
	sub	r3, sp, #FIQ_STACK_SIZE		

	msr     CPSR_c, #(IRQMODE|NOINT)	/* IRQ mode, interrupts disabled */
	mov	sp, r3
	sub	r3, r3, #IRQ_STACK_SIZE
		
	msr     CPSR_c, #(SVCMODE|NOINT)	/* Supervisory mode, interrupts disabled */
	mov	sp, r3
	sub	r3, r3, #SVC_STACK_SIZE

	msr     CPSR_c, #(SYSMODE|NOINT)
	mov	sp, r3

	/* call function "void sys_preinit_hook(void)" */
	ldr	r3, =FUNCTION(sys_preinit_hook)
	cmp	r3, #0
	beq	1f
	mov	lr, pc
	bx	r3 		/*mov	pc, r3*/

	/*copy .data section to RAM and Zero the memory in the .bss section.  */
	mov 	r2, #0			/* Second arg: fill value */
	mov	fp, r2			/* Null frame pointer */
	mov	r7, r2			/* Null frame pointer for Thumb */
	
	ldr	r0, =_flash_data_start	/* Get pointer to ROM data in Flash */
	ldr	r4, =_data_start	/* and its RAM copy */
	ldr	r1, =__bss_start__	/* zero data */
	
	cmp	r0, r4			/* Check that they are different */
	beq	1f
0:	cmp	r4, r1			/* Copy data from flash to ram */
	ldrcc	r5, [r0], #4
	strcc	r5, [r4], #4
	bcc	0b
1:	ldr	r3, =__bss_end__	
2:	cmp	r1, r3			/* Zero init */
	strcc   r2, [r1], #4
	bcc	2b

	ldr	r3, =FUNCTION(sys_init_hook)
	cmp	r3, #0
	beq	2f
	mov	lr, pc
	bx	r3 		/*mov	pc, r3*/
2:	

	/* enter User Mode */
	msr     CPSR_c, #(USERMODE|NOINT)
#ifdef __thumb__		/* Enter Thumb mode.... */

	add	a4, pc, #1	/* Get the address of the Thumb block */
	bx	a4		/* Go there and start Thumb decoding  */
	.endfunc

	.code 16
	.global thumb_start
thumb_start:
	.func thumb_start
	.thumb_func 
#endif

	/*call	FUNCTION (main)*/
	mov	r0, #0		/*  no arguments  */
	mov	r1, #0		/*  no argv either */

	ldr	r3, =FUNCTION(main)	
	mov	lr, pc
	bx	r3

#ifdef __thumb__
	/* Come out of Thumb mode.  This code should be redundant.  */

	ldr	r3, =.Lhalt	/*make sure we are in word boundary*/
	bx	r3
	.endfunc
#else
	.endfunc
#endif

	.code 32
	/* Halt the execution.  This code should never be executed.  */
	.balign 4
.Lhalt:	b	.
	
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Protect against unhandled exceptions.  */
	.cantunwind
	.fnend
#endif

	.align
LTORG_INIT_LookupTable: 
        .LTORG

/*******************************************************************************
 *              Peripherals IRQ handlers address table
 *******************************************************************************/
	.section ".irqvector"
	.balign 128
	.global	IRQ_Vector_table
IRQ_Vector_table:
T0TIMI_Addr:	.space         4	//T0TIMI_IRQHandler
FLASH_Addr:	.space         4	//FLASH_IRQHandler
RCCU_Addr:	.space         4	//RCCU_IRQHandler
WDG_Addr:	.space         4	//WDG_IRQHandler
XTI_Addr:	.space         4	//XTI_IRQHandler
USBHP_Addr:	.space         4	//USBHP_IRQHandler
I2C0ITERR_Addr:	.space         4	//I2C0ITERR_IRQHandler
I2C1ITERR_ADDR:	.space         4	//I2C1ITERR_IRQHandler
UART0_Addr:	.space         4	//UART0_IRQHandler
UART1_Addr:	.space         4	//UART1_IRQHandler
UART2_ADDR:	.space         4	//UART2_IRQHandler
UART3_ADDR:	.space         4	//UART3_IRQHandler
BSPI0_ADDR:	.space         4	//BSPI0_IRQHandler
BSPI1_Addr:	.space         4	//BSPI1_IRQHandler
I2C0_Addr:	.space         4	//I2C0_IRQHandler
I2C1_Addr:	.space         4	//I2C1_IRQHandler
CAN_Addr:	.space         4	//CAN_IRQHandler
ADC12_Addr:	.space         4	//ADC12_IRQHandler
T1TIMI_Addr:	.space         4	//T1TIMI_IRQHandler
T2TIMI_Addr:	.space         4	//T2TIMI_IRQHandler
T3TIMI_Addr:	.space         16	//T3TIMI_IRQHandler
					// reserved
					// reserved
					// reserved
HDLC_Addr:	.space         4	//HDLC_IRQHandler
USBLP_Addr:	.space         12	//USBLP_IRQHandler
					// reserved
					// reserved
T0TOI_Addr:	.space         4	//T0TOI_IRQHandler
T0OC1_Addr:	.space         4	//T0OC1_IRQHandler
T0OC2_Addr:	.space         4	//T0OC2_IRQHandler

        .END

