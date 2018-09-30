/*
 *   Copyright 1995, 1996, 1998, 1999, 2000, 2003, 2004
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
 *   @ libgcc routines for ARM cpu.
 *   @ Division routines, written by Richard Earnshaw, (rearnsha@armltd.co.uk)
 *   samsung's example startup code (April.2000)
 * and
 *   newlib crt0.S version
 */
 
/* BUSWIDTH 16,32 */
#define BUSWIDTH 16 


/* "DRAM","SDRAM" */
#define BDRAMTYPE_SDRAM


/* This value has to be TRUE (1) on ROM program. */
/* This value has to be FALSE(0) in RAM program. */
#define PLLONSTART 1  

/*GBLA PLLCLK*/
#define PLLCLK 60000000

#if ( PLLCLK == 60000000 )
#define M_DIV	0x4C     /* Fin=14,318MHz Fout=60MHz Fvco=120MHz */
#define P_DIV	0x8
#define S_DIV	0x1
#endif

/*Interrupt Control*/
#define INTPND		0x01e00004
#define INTMOD		0x01e00008
#define INTMSK		0x01e0000c
#define I_ISPR		0x01e00020
#define I_ISPC		0x01e00024
#define I_CMST		0x01e0001c

/*Watchdog timer*/
#define WTCON		0x01d30000
#define WTCNT		0x01d30008

/*Clock Controller*/
#define PLLCON		0x01d80000
#define CLKCON		0x01d80004
#define LOCKTIME	0x01d8000c

/*Memory Controller*/
#define BWSCON		0x01c80000
#define REFRESH		0x01c80024

/*Pre-defined constants*/
#define USERMODE	0x10
#define FIQMODE		0x11
#define IRQMODE		0x12
#define SVCMODE		0x13
#define ABORTMODE	0x17
#define UNDEFMODE	0x1b
#define SYSMODE		0x1f
#define MODEMASK	0x1f
#define NOINT		0xc0

/*MEMORY CONTROL PARAMETERS*/
/*Bank 0 parameter	  Flash */
#define B0_Tacs 0x1     /*1clk*/
#define B0_Tcos 0x0     /*0clk*/
#define B0_Tacc 0x4     /*6clk*/
#define B0_Tcoh 0x0     /*0clk*/
#define B0_Tah  0x0     /*0clk*/
#define B0_Tacp 0x0     /*0clk*/
#define B0_PMC  0x0     /*normal(1data)*/

/*Bank 1 parameter	Unused second Flash*/
#define B1_Tacs 0x1     //1clk
#define B1_Tcos 0x0     //0clk
#define B1_Tacc 0x5     //8clk
#define B1_Tcoh 0x0     //0clk
#define B1_Tah  0x0     //0clk
#define B1_Tacp 0x0     //0clk
#define B1_PMC  0x0     //normal(1data)

//Bank 2 parameter	  Flash
#define B2_Tacs 0x1     //1clk
#define B2_Tcos 0x0     //0clk
#define B2_Tacc 0x6     //10clk
#define B2_Tcoh 0x0     //0clk
#define B2_Tah  0x0     //0clk
#define B2_Tacp 0x0     //0clk
#define B2_PMC  0x0     //normal(1data)

//Bank 3 parameter	  Device
#define B3_Tacs 0x1     //1clk
#define B3_Tcos 0x1     //1clk
#define B3_Tacc 0x5     //8clk
#define B3_Tcoh 0x2     //1clk
#define B3_Tah  0x2     //3clk
#define B3_Tacp 0x0     //0clk
#define B3_PMC  0x0     //normal(1data)

//Bank 4 parameter	  SRAM
#define B4_Tacs 0x0     //0clk
#define B4_Tcos 0x0     //0clk
#define B4_Tacc 0x2     //2clk
#define B4_Tcoh 0x0     //0clk
#define B4_Tah  0x0     //0clk
#define B4_Tacp 0x0     //0clk
#define B4_PMC  0x0     //normal(1data)

//Bank 5 parameter	  unused
#define B5_Tacs 0x3     //4clk
#define B5_Tcos 0x3     //4clk
#define B5_Tacc 0x7     //14clk
#define B5_Tcoh 0x3     //4clk
#define B5_Tah  0x3     //4clk
#define B5_Tacp 0x3     //6clk
#define B5_PMC  0x0     //normal(1data)

//Bank 6(if SROM) parameter	- ignored
#define B6_Tacs 0x3     //4clk
#define B6_Tcos 0x3     //4clk
#define B6_Tacc 0x7     //14clk
#define B6_Tcoh 0x3     //4clk
#define B6_Tah  0x3     //4clk
#define B6_Tacp 0x3     //6clk
#define B6_PMC  0x0     //normal(1data)

#ifdef  BDRAMTYPE_DRAM
#define B6_MT   0x2     //EDO DRAM ;MT=01(FP DRAM) or 10(EDO DRAM)
#define B6_Trcd 0x0     //1clk
#define B6_Tcas 0x0     //1clk
#define B6_Tcp  0x0     //1clk
#define B6_CAN  0x2     //10bit
#endif

#ifdef BDRAMTYPE_SDRAM
#define B6_MT   0x3     //MT=11 (SDRAM)
#define B6_Trcd 0x1     //3clk
#define B6_SCAN 0x0     //8bit
#endif

//Bank 7(if SROM) parameter
#define B7_Tacs 0x3     //4clk
#define B7_Tcos 0x3     //4clk
#define B7_Tacc 0x7     //14clk
#define B7_Tcoh 0x3     //4clk
#define B7_Tah  0x3     //4clk
#define B7_Tacp 0x3     //6clk
#define B7_PMC  0x0     //normal(1data)

#if defined (BDRAMTYPE_DRAM)
#define B7_MT   0x2     //EDO DRAM ;MT=01(FP DRAM) or 10(EDO DRAM)
#define B7_Trcd 0x0     //2clk
#define B7_Tcas 0x0     //2clk
#define B7_Tcp  0x0     //2clk
#define B7_CAN  0x2     //10bit
#endif

#if defined (BDRAMTYPE_SDRAM)
#define B7_MT   0x3     //MT=11(SDRAM)
#define B7_Trcd 0x1     //3clk
#define B7_SCAN 0x0     //8bit
#endif

//REFRESH parameter
#define REFEN   0x1     //Refresh enable
#define TREFMD  0x0     //CBR(CAS before RAS)/Auto refresh
#define Trp     0x2     //4clk
#define Trc     0x2     //6clk
#define Tchr    0x3     //4clk
#define REFCNT  1113     //period=15us, MCLK=60Mhz


.MACRO HANDLER HandlerLabel HandleLabel LookupTable
	.global \HandlerLabel
	.global \HandleLabel
	.global \LookupTable
	.align
	\HandlerLabel:
	sub    sp,sp,#4       //decrement sp(to store jump address)
	stmfd  sp!,{r0}       //PUSH the work register to stack(lr does't push because it return to original address)
	ldr r0,=\HandleLabel  //load the address of HandleXXX to r0
	ldr r0,[r0]           //load the contents(service routine start address) of HandleXXX
	str r0,[sp,#4]        //store the contents(ISR) of HandleXXX to stack
	ldmfd sp!,{r0,pc}     //POP the work register and pc(jump to ISR)
	\LookupTable:    
	.LTORG
.ENDM


/* We need to know what prefix to add to function names.  */

#ifndef __USER_LABEL_PREFIX__
#error  __USER_LABEL_PREFIX__ not defined
#endif

/* ANSI concatenation macros.  */

#define CONCAT(a, b) CONCAT2(a, b)
#define CONCAT2(a, b) a ## b
#define FUNCTION( name ) CONCAT (__USER_LABEL_PREFIX__, name)

.section ".init"
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
Entry_VectorTable:	b start               /*Resetentry*/
Exception_Undef:	ldr pc,=HandlerUndef  /*b HandleUndef */
Exception_SWI:		ldr pc,=HandlerSWI    /*b HandleSWI  */
Exception_ProgramAbort:	ldr pc,=HandlerPabort /*b HandlePabort */
Exception_DataAbort:	ldr pc,=HandlerDabort /*b HandleDabort */
			b   .                 /*Reserved */
Exception_IRQ:		ldr pc,=HandlerIRQ    /*b HandlerIRQ */
Exception_FIQ:		ldr pc,=HandlerFIQ    /*b HandlerFIQ */
.LEntry_VectorTable_end:   
	.size Entry_VectorTable,Entry_VectorTable - .LEntry_VectorTable_end

	/*.code 32 */
.LBlockException:	b .
	.align
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
	
	.global exception_VectorTable_LookupTable
exception_VectorTable_LookupTable:
LTORG_INIT_LookupTable_1_start:    
	.LTORG

	/*************************************
	 * exeption Handler (slow execution) *
	 *************************************/
HANDLER HandlerUndef     HandleUndef	LTHandleUndef
HANDLER HandlerDabort    HandleDabort	LTHandleDabort
HANDLER HandlerPabort    HandlePabort	LTHandlePabort

	/*********
	 * start *
	 *********/
	.global	_start
	.global	start
	.func start,_start
start:
_start:
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Annotation for EABI unwinding tables.  */
	.fnstart
#endif
	ldr	r0,=WTCON       //watch dog disable
	ldr	r1,=0x0 
	str	r1,[r0]
	
	ldr	r0,=WTCNT       //watch dog counter = 0x8000
	ldr	r1,=0x8000 
	str	r1,[r0]
	
	ldr	r0,=INTMSK
	ldr	r1,=0x07ffffff  //all interrupt disable
	str	r1,[r0]
	
	/* Set memory control registers */
	ldr	r0,=SMRDATA
	ldmia	r0,{r1-r13}
	ldr	r0,=BWSCON
	stmia	r0,{r1-r13}
	
	/* Set clock control registers */
	ldr	r0,=LOCKTIME
	ldr	r2,=0x1658 /*must be more than 400usec -> 400usec * 14.3MHz = 5720*/ 
	str	r2,[r0]
	
#if PLLONSTART
	ldr	r0,=PLLCON
	ldr	r1,=((M_DIV<<12)+(P_DIV<<4)+S_DIV)
	str	r1,[r0]
#endif
	ldr	r0,=CLKCON
	ldr	r1,=0x7ff8          //All unit block CLK enable
	str	r1,[r0]

#if PLLONSTART
	/*Wait until PLL is running -- before we are going to copy data to SDRAM*/
	ldr	r1,=1
0:	sub	r2,r2,r1
	cmp	r2, #0
	bne	0b
#endif

	/* Start by setting up a stack (except the user stack)*/
	/* Set up the stack pointer to a fixed value */

	mrs	r2, CPSR
	tst	r2, #0x0F	/* Test mode bits - in User of all are 0 */
	beq	.		/* "eq" means r2 AND #0x0F is 0 -- so block executing here*/
	msr     CPSR_c, #(UNDEFMODE|NOINT)	/* Undefined mode, interrupts disabled */
	ldr	sp, =UndefStack
	msr     CPSR_c, #(ABORTMODE|NOINT)	/* Abort mode, interrupts disabled */
	ldr	sp, =AbortStack
	msr     CPSR_c, #(FIQMODE|NOINT)	/* FIRQ mode, interrupts disabled */
	ldr 	sp, =FIQStack
	msr     CPSR_c, #(IRQMODE|NOINT)	/* IRQ mode, interrupts disabled */
	ldr	sp, =IRQStack
	msr     CPSR_c, #(SVCMODE|NOINT)	/* Supervisory mode, interrupts disabled */
	ldr	sp, =SVCStack
	msr     CPSR_c, #(SYSMODE|NOINT)
	ldr	sp, =SVCStack
	
	/* call function "void sys_preinit_hook(void)" */
	ldr	r3, =FUNCTION(sys_preinit_hook)
	cmp	r3, #0
	beq	1f
	mov	lr, pc
	bx	r3 		/*mov	pc, r3*/

	/*Copy isr.text,isr.data and isr.bss */
1:	ldr	r0, =_isr_text_rom	//Get pointer to ROM data
	ldr	r1, =_ldinfo_iram_start	//and RAM copy
	ldr	r3, =_isr_bss_start	//Zero init base => top of initialised data
	
	cmp	r0, r1			//Check that they are different
	beq	1f
0:	cmp	r1, r3			//Copy isr.text and isr.data
	ldrcc	r2, [r0], #4
	strcc	r2, [r1], #4
	bcc	0b
1:	ldr	r1, =_isr_bss_end	//Top of zero isr.bss segment
	mov	r2, #0
2:	cmp	r3, r1			//Zero isr.bss
	strcc	r2, [r3], #4
	bcc	2b
	
	/*Copy user program .data and .text sections to SDRAM*/
	ldr	r0, =_phase2_init_text_end @ Get pointer to ROM data
	ldr	r1, =_ldinfo_sdram_start @ and RAM copy
	ldr	r3, =_bss_start        @ Zero init base => top of initialised data
	
	cmp	r0, r1                 @ Check that they are different
	beq	1f
0:	cmp	r1, r3                 @ Copy init data
	ldrcch	r2, [r0], #2           @--> LDRCC r2, [r0] + ADD r0, r0, #2
	strcch	r2, [r1], #2           @--> STRCC r2, [r1] + ADD r1, r1, #2
	bcc	0b
	
	/*Zero the memory in the .bss section*/
1:	ldr	r1, =_end              @ Top of zero init segment
	mov	r2, #0
2:	cmp	r3, r1                 @ Zero init
	strcch	r2, [r3], #2
	bcc	2b

	/* Setup IRQ handler */
	ldr    r0,=HandleIRQ
	ldr    r1,=exceptionIRQ
	str    r1,[r0]
	
	/* Setup SWI handler */
	ldr    r0,=HandleSWI
	ldr    r1,=exceptionSWI 
	str    r1,[r0]
	
	/* Setup Undef intruction handler */ 
	ldr    r0,=HandleUndef
	ldr    r1,=exceptionUndef
	str    r1,[r0]
	
	/* Setup Instruction Abort handler */
	ldr    r0,=HandlePabort
	ldr    r1,=exceptionPabort
	str    r1,[r0]
	
	/* Setup Data Abort handler */
	ldr    r0,=HandleDabort
	ldr    r1,=exceptionDabort
	str    r1,[r0]
	
	/* Setup FIQ handler */
	ldr    r0,=HandleFIQ
	ldr    r1,=exceptionFIQ
	str    r1,[r0]
	
	/* call function "void sys_init_hook(void)" */
	ldr sp,=stack_origin	//use the user level stack
	ldr	r3, =FUNCTION(sys_init_hook)
	cmp	r3, #0
	beq	2f
	mov	lr, pc
	bx	r3 		/*mov	pc, r3*/
2:	
	/* enter User Mode */
	msr     CPSR_c, #(USERMODE|NOINT)
	ldr	sp,=stack_origin
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
	.align 4
.Lhalt:	b	.
	
#if defined(__ELF__) && !defined(__USING_SJLJ_EXCEPTIONS__)
	/* Protect against unhandled exceptions.  */
	.cantunwind
	.fnend
#endif

	.align
LTORG_INIT_LookupTable_2_start:    
	.LTORG
//LTORG_INIT_LookupTable_2_end:

//.section ".datainit"
	.align
SMRDATA:	/* Memory configuration @ BWSCON */
#if ( BUSWIDTH == 16)
.word 0x1111110A       //Bank0=OM[1:0], Bank1~Bank7=16bit
#else // @BUSWIDTH=32
#error "wrong BUSWITH"
#endif

.word ((B0_Tacs<<13)+(B0_Tcos<<11)+(B0_Tacc<<8)+(B0_Tcoh<<6)+(B0_Tah<<4)+(B0_Tacp<<2)+(B0_PMC))
.word ((B1_Tacs<<13)+(B1_Tcos<<11)+(B1_Tacc<<8)+(B1_Tcoh<<6)+(B1_Tah<<4)+(B1_Tacp<<2)+(B1_PMC))
.word ((B2_Tacs<<13)+(B2_Tcos<<11)+(B2_Tacc<<8)+(B2_Tcoh<<6)+(B2_Tah<<4)+(B2_Tacp<<2)+(B2_PMC))
.word ((B3_Tacs<<13)+(B3_Tcos<<11)+(B3_Tacc<<8)+(B3_Tcoh<<6)+(B3_Tah<<4)+(B3_Tacp<<2)+(B3_PMC))
.word ((B4_Tacs<<13)+(B4_Tcos<<11)+(B4_Tacc<<8)+(B4_Tcoh<<6)+(B4_Tah<<4)+(B4_Tacp<<2)+(B4_PMC))
.word ((B5_Tacs<<13)+(B5_Tcos<<11)+(B5_Tacc<<8)+(B5_Tcoh<<6)+(B5_Tah<<4)+(B5_Tacp<<2)+(B5_PMC))

#if defined(BDRAMTYPE_DRAM)
.word ((B6_MT<<15)+(B6_Trcd<<4)+(B6_Tcas<<3)+(B6_Tcp<<2)+(B6_CAN)) 
.word ((B7_MT<<15)+(B7_Trcd<<4)+(B7_Tcas<<3)+(B7_Tcp<<2)+(B7_CAN))
#elif defined (BDRAMTYPE_SDRAM)
.word ((B6_MT<<15)+(B6_Trcd<<2)+(B6_SCAN))
.word ((B7_MT<<15)+(B7_Trcd<<2)+(B7_SCAN))
#else
#error "neither BDRAMTYPE_DRAM nor BDRAMTYPE_SDRAM"
#endif

/* REFRESH */
.word ((REFEN<<23)+(TREFMD<<22)+(Trp<<20)+(Trc<<18)+(Tchr<<16)+REFCNT)   
.word 0x14 //SCLK power mode, BANKSIZE 2M/2M
.word 0x20 //MRSR6 CAS Latency =2clk
.word 0x20 //MRSR7


	.section ".isr_text"
	.CODE 32
	
	/***************************************
	 * exeption Handler (faster execution) *
	 ***************************************/
HANDLER HandlerSWI       HandleSWI	LTHandleSWI
HANDLER HandlerIRQ       HandleIRQ	LTHandleIRQ
HANDLER HandlerFIQ       HandleFIQ	LTHandleFIQ


	.section ".isr_bss"
	.align
_ISR_STACKADDRESS:

	//stack_origin 
	//Note that the UserStack is not in RAM it is in the SDRAM space.
	//SVCStack, UndefStack, AbortStack and IRQStack uses the same space 
	.global SVCStack
	.global IRQStack
	.global UndefStack
	.global AbortStack
	.global FIQStack

	.space	256
SVCStack:   
IRQStack:   
	.space	16
UndefStack: 
AbortStack: 

//.space	256
FIQStack:   
	.space	4
	.align
	
	//Exeption Vector Table
	.global _ISR_STARTADDRESS
_ISR_STARTADDRESS:
	.global HandleReset
HandleReset:     .space         4
	.global HandleUndef
HandleUndef:     .space         4
	.global HandleSWI
HandleSWI:       .space         4
	.global HandlePabort
HandlePabort:    .space         4
	.global HandleDabort
HandleDabort:    .space         4
	.global HandleReserved
HandleReserved:  .space         4
	.global HandleIRQ
HandleIRQ:       .space         4
	.global HandleFIQ
HandleFIQ:       .space         4

	//Interrupt (IRQ/FIQ) Vector Table
	.global interruptVectorTable
interruptVectorTable:
	.global HandleADC
HandleADC:      .space          4
	.global HandleRTC
HandleRTC:      .space          4
	.global HandleUTXD1
HandleUTXD1:    .space          4
	.global HandleUTXD0
HandleUTXD0:    .space          4
	.global HandleSIO
HandleSIO:      .space          4
	.global HandleIIC
HandleIIC:      .space          4
	.global HandleURXD1
HandleURXD1:    .space          4
	.global HandleURXD0
HandleURXD0:    .space          4
	.global HandleTIMER5
HandleTIMER5:   .space          4
	.global HandleTIMER4
HandleTIMER4:   .space          4
	.global HandleTIMER3
HandleTIMER3:   .space          4
	.global HandleTIMER2
HandleTIMER2:   .space          4
	.global HandleTIMER1
HandleTIMER1:   .space          4
	.global HandleTIMER0
HandleTIMER0:   .space          4
	.global HandleUERR01
HandleUERR01:   .space          4
	.global HandleWDT
HandleWDT:      .space          4
	.global HandleBDMA1
HandleBDMA1:    .space          4
	.global HandleBDMA0
HandleBDMA0:    .space          4
	.global HandleZDMA1
HandleZDMA1:    .space          4
	.global HandleZDMA0
HandleZDMA0:    .space          4
	.global HandleTICK
HandleTICK:     .space          4
	.global HandleEINT4567
HandleEINT4567: .space          4
	.global HandleEINT3
HandleEINT3:    .space          4
	.global HandleEINT2
HandleEINT2:    .space          4
	.global HandleEINT1
HandleEINT1:    .space          4
	.global HandleEINT0
HandleEINT0:    .space          4   //0xc1(c7)fff84

.END

	
