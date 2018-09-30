/*
 * lpc21xx.h
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
 */
#include <inttypes.h>

/**********************************************************************************
 *                   Timer 0
 *********************************************************************************/
/*
 * T0_IR		0xE0004000
 * ---------------------------------
 *  cr3 cr2 cr1 cr0 mr3 mr2 mr1 mr0
 * ---------------------------------
 * cr? = Interrupt flag for capture channel ? event
 * mr? = Interrupt flag for match channel ?
 */
#define T0_IR	*((volatile uint8_t*) 0xE0004000)

/*
 * T0_TCR		0xE0004004
 * ------------------       
 *  x x x x  x x R E
 * ------------------
 * x= do not use 
 * R= counter reset
 * E= counter enable
 */
#define T0_TCR	*((volatile uint8_t*) 0xE0004004)

/*
 * T1_TC		0xE0004008
 * 32-Bit Timer Counter
 */
#define	T0_TC	*((volatile uint32_t*) 0xE0004008)

/*
 * T0_PR		0xE000400C
 * 32-Bit Prescale Reg (max. value of Prescale Counter)
 */
#define	T0_PR	*((volatile uint32_t*) 0xE000400C)

/*
 * T0_PC		0xE0004010
 * 32-Bit Prescale Counter
 */
#define	T0_PC	*((volatile uint32_t*) 0xE0004010)

/*
 *  T0_MCR		0xE0004014
 *  ----------------------------------------------------------------------
 *  x x x x stp3 rst3 int3 stp2  rst2 int2 stp1 rst1  int1 sto0 rst0 int0
 *  ----------------------------------------------------------------------
 *  x= do not use 
 *  stp? = if set:on match TC and PC will stoped and TCR? will be set to 0
 *  rst? = if set:on match TC will be resetted
 *  int? = if set:on match interrupt is generated
 */ 
#define	T0_MCR	*((volatile uint16_t*) 0xE0004014)
#define T0_MCR_BIT_POS_STOP_ON_MR( _MR_ )	(((_MR_)*3)+2)
#define T0_MCR_BIT_POS_RESET_ON_MR(_MR_)	(((_MR_)*3)+1)
#define T0_MCR_BIT_POS_INTERRUPT_ON_MR(_MR_)	((_MR_)*3)
	
/* T0 match registers */  
#define	T0_MR0	*((volatile uint32_t*) 0xE0004018)
#define	T0_MR1	*((volatile uint32_t*) 0xE000401C)
#define	T0_MR2	*((volatile uint32_t*) 0xE0004020)
#define	T0_MR3	*((volatile uint32_t*) 0xE0004024)

/*
 *  T0_CCR		0xE0004028
 *  ---------------------------------------------------------------
 *  x x x x  x x x int2  fe2 re2 int1 fe1  re1 int0 fe0 re0 
 *  ---------------------------------------------------------------
 *  int? = if set: generate interrupt on cature event
 *  fe?  = if set: capture event on falling edge of CAP? pin (store Timer Count value in Capture Register)
 *  re?  = if set: capture event on rising edge of CAP? pin (store Timer Count value in Capture Register)
 */ 
#define T0_CCR *((volatile uint16_t*) 0xE0004028)
#define T0_CCR_BIT_POS_INTERRUPT_ON_CAPTURE(_CR_)	(((_CR_)*3)+2)
#define T0_CCR_BIT_POS_CAPTURE_ON_FALLING_EDGE(_CR_)	(((_CR_)*3)+1)
#define T0_CCR_BIT_POS_CAPTURE_ON_RISING_EDGE(_CR_)	((_CR_)*3)

/* T0 capture registers */
#define	T0_CR0	*((volatile uint32_t*) 0xE000402C)
#define	T0_CR1	*((volatile uint32_t*) 0xE0004030)
#define	T0_CR2	*((volatile uint32_t*) 0xE0004034)


/**********************************************************************************
 *                   Timer 1
 *********************************************************************************/
/*
 * T1_IR		0xE0008000
 * ---------------------------------
 *  cr3 cr2 cr1 cr0 mr3 mr2 mr1 mr0
 * ---------------------------------
 * cr? = Interrupt flag for capture channel ? event
 * mr? = Interrupt flag for match channel ?
 */
#define T1_IR	*((volatile uint8_t*) 0xE0008000)

/*
 * T1_TCR		0xE0008004
 * ------------------       
 *  x x x x  x x R E
 * ------------------
 * x= do not use 
 * R= counter reset
 * E= counter enable
 */
#define T1_TCR	*((volatile uint8_t*) 0xE0008004)

/*
 * T1_TC		0xE0008008
 * 32-Bit Timer Counter
 */
#define	T1_TC	*((volatile uint32_t*) 0xE0008008)

/*
 * T1_PR		0xE000800C
 * 32-Bit Prescale Reg (max. value of Prescale Counter)
 */
#define	T1_PR	*((volatile uint32_t*) 0xE000800C)

/*
 * T1_PC		0xE0008010
 * 32-Bit Prescale Counter
 */
#define	T1_PC	*((volatile uint32_t*) 0xE0008010)

/*
 *  T1_MCR		0xE0008014
 *  ----------------------------------------------------------------------
 *  x x x x stp3 rst3 int3 stp2  rst2 int2 stp1 rst1  int1 sto0 rst0 int0
 *  ----------------------------------------------------------------------
 *  x= do not use 
 *  stp? = if set:on match TC and PC will stoped and TCR? will be set to 0
 *  rst? = if set:on match TC will be resetted
 *  int? = if set:on match interrupt is generated
 */ 
#define	T1_MCR	*((volatile uint16_t*) 0xE0008014)
#define T1_MCR_BIT_POS_STOP_ON_MR( _MR_ )	(((_MR_)*3)+2)
#define T1_MCR_BIT_POS_RESET_ON_MR(_MR_)	(((_MR_)*3)+1)
#define T1_MCR_BIT_POS_INTERRUPT_ON_MR(_MR_)	((_MR_)*3)
	
/* T1 match registers */  
#define	T1_MR0	*((volatile uint32_t*) 0xE0008018)
#define	T1_MR1	*((volatile uint32_t*) 0xE000801C)
#define	T1_MR2	*((volatile uint32_t*) 0xE0008020)
#define	T1_MR3	*((volatile uint32_t*) 0xE0008024)

/*
 *  T1_CCR		0xE0008028
 *  ---------------------------------------------------------------
 *  x x x x  int3 fe3 re3 int2  fe2 re2 int1 fe1  re1 int0 fe0 re0 
 *  ---------------------------------------------------------------
 *  int? = if set: generate interrupt on cature event
 *  fe?  = if set: capture event on falling edge of CAP? pin (store Timer Count value in Capture Register)
 *  re?  = if set: capture event on rising edge of CAP? pin (store Timer Count value in Capture Register)
 */ 
#define T1_CCR *((volatile uint16_t*) 0xE0008028)
#define T1_CCR_BIT_POS_INTERRUPT_ON_CAPTURE(_CR_)	(((_CR_)*3)+2)
#define T1_CCR_BIT_POS_CAPTURE_ON_FALLING_EDGE(_CR_)	(((_CR_)*3)+1)
#define T1_CCR_BIT_POS_CAPTURE_ON_RISING_EDGE(_CR_)	((_CR_)*3)

/* T1 capture registers */
#define	T1_CR0	*((volatile uint32_t*) 0xE000802C)
#define	T1_CR1	*((volatile uint32_t*) 0xE0008030)
#define	T1_CR2	*((volatile uint32_t*) 0xE0008034)
#define	T1_CR3	*((volatile uint32_t*) 0xE0008038)


/**********************************************************************************
 *                   GPIO
 *********************************************************************************/
/*
 * IOPIN		0xE0028000
 * --------------------------------------------------------------------------       
 *  i i i i  i i i i  i i i i  i i i i    i i i i  i i i i  i i i i  i i i i  
 * --------------------------------------------------------------------------
 * i= Input (read only) 
 */
#define IOPIN	*((volatile uint32_t*) 0xE0028000)

/*
 * IOSET		0xE0028004
 * --------------------------------------------------------------------------       
 *  s s s s  s s s s  s s s s  s s s s    s s s s  s s s s  s s s s  s s s s 
 * --------------------------------------------------------------------------
 * s= Set Output 
 */
#define IOSET	*((volatile uint32_t*) 0xE0028004)

/*
 * IODIR		0xE0028008
 * --------------------------------------------------------------------------       
 *  d d d d  d d d d  d d d d  d d d d    d d d d  d d d d  d d d d  d d d d 
 * --------------------------------------------------------------------------
 * d= direction ( 0=Input; 1=Output )
 */
#define IODIR	*((volatile uint32_t*) 0xE0028008)

/*
 * IOCLR		0xE002800C
 * --------------------------------------------------------------------------       
 *  c c c c  c c c c  c c c c  c c c c    c c c c  c c c c  c c c c  c c c c  
 * --------------------------------------------------------------------------
 * c= Clear Output 
 */
#define IOCLR	*((volatile uint32_t*) 0xE002800C)


#if defined(REQ_CCLK) && defined(F_OSC)
/**********************************************************************************
 *                   PLL
 *********************************************************************************/
#define MSEL ((REQ_CCLK) / (F_OSC))

#define CCLK ((F_OSC) * (MSEL))

#if (MSEL < 1) || (F_OSC < 10000000) || (F_OSC > 25000000) || (CCLK > 60000000)
#error "wrong frequency settings\n"
#endif

#if ( (CCLK * 2 ) > 156000000 ) && ( (CCLK * 2 ) < 320000000 )
#define PSEL 1
#elif ( (CCLK * 2 * 2 ) > 156000000 ) && ( (CCLK * 2 * 2 ) < 320000000 )
#define PSEL 2
#elif ( (CCLK * 2 * 4 ) > 156000000 ) && ( (CCLK * 2 * 4 ) < 320000000 )
#define PSEL 4
#elif ( (CCLK * 2 * 8 ) > 156000000 ) && ( (CCLK * 2 * 8 ) < 320000000 )
#define PSEL 8
#else
#error "no PSEL setting found\n"
#endif

/*       
 * PLLCON                0xE01FC080       
 * ------------------       
 *  x x x x  x x C E        
 * ------------------
 * x= do not use 
 * C= PLLC  
 * E= PLLE
 */
#define PLLCON *((volatile uint8_t*) 0xE01FC080)
#define PLLE_BIT_POS	(0)
#define PLLC_BIT_POS	(1)

/* 
 * PLL initialization       
 * cclk = Fosc*MSEL       
 * Fcco = cclk*2*PSEL       
 * 
 * PLLCFG                0xE01FC084       
 * ------------------       
 *  x P P M  M M M M        
 * ------------------ 
 * x= do not use 
 * P= PSEL  
 * M= MSEL        
 */
#define PLLCFG  *((volatile uint8_t*) 0xE01FC084)
#define MSEL_BIT_POS	(0)
#define PSEL_BIT_POS	(5)

/*
 * PLLSTAT                      0xE01FC088  
 * -------------------------------------  
 *  x x x x  x L*c*e   x*p*p*m *m*m*m*m  
 * -------------------------------------
 * x= do not use 
 * L = PLOCK 
 * *c=PLLC bit after feed 
 * *e=PLLE bit after feed 
 * *p=PSEL bits after feed 
 * *m=MSEL bits after feed    
 */
#define PLLSTAT	*((volatile uint16_t*) 0xE01FC088)
#define PLOCK_BIT_POS	(10)

/*
 * PLLFEED		0xE01FC08C
 * 8 Bit Write only Feed sequence 0xAA,0x55
 */
#define PLLFEED	*((volatile uint8_t*) 0xE01FC08C)

/**********************************************************************************
 *                   MAM
 *********************************************************************************/
/*
 * MAMCR		0xE01FC000
 * ------------------       
 *  x x x x  x x M M 
 * ------------------
 * x= do not use 
 * M= MAM_MODE_CTR
 */
#define	MAMCR *((volatile uint8_t*) 0xE01FC000)
#define MAM_MODE_DISABLE	0
#define MAM_MODE_PARTIALLY	1
#define MAM_MODE_FULL		2 
/*
 * MAMTIM		0xE01FC000
 * ------------------       
 *  x x x x  x f f f 
 * ------------------
 * x= do not use 
 * f= fetch cycle timming
 */
#define	MAMTIM *((volatile uint8_t*) 0xE01FC004)

#if ( CCLK < 20000000 )
#define MAMTIM_MAX	1
#elif (CCLK < 40000000 )
#define MAMTIM_MAX	2
#elif (CCLK < 60000000 )
#define MAMTIM_MAX	3
#elif (CCLK < 80000000 )
#define MAMTIM_MAX	4
#elif (CCLK < 100000000 )
#define MAMTIM_MAX	5
#elif (CCLK < 200000000 )
#define MAMTIM_MAX	6
#else
#define MAMTIM_MAX	7
#endif

#endif /*defined(REQ_CCLK) && defined(F_OSC)*/

/*
 * VPBDIV		0xE01FC100
 * ------------------       
 *  x x x x  x x d d 
 * ------------------
 * x= do not use 
 * m= div_mode
 */
#define VPBDIV	*((volatile uint8_t*) 0xE01FC100)
#define VPB_PCLK_IS_QUARTER_OF_CCLK	0
#define VPB_PCLK_IS_SAME_AS_CCLK	1
#define VPB_PCLK_IS_HALF_OF_CCLK	2

/**********************************************************************************
 *                   Vectored Interrupt Controller
 *********************************************************************************/
#define VICIRQSTATUS	*((volatile uint32_t*) 0xFFFFF000)
#define VICFIQSTATUS	*((volatile uint32_t*) 0xFFFFF004)
#define VICRAWSTATUS	*((volatile uint32_t*) 0xFFFFF008)
/* Select VIC Interrupt type FIQ (=1) or IRQ (=0)*/ 
#define	VICINTSEL	*((volatile uint32_t*) 0xFFFFF00C)
/* Enable Interrupt */  
#define	VICINTEN	*((volatile uint32_t*) 0xFFFFF010)
/* Disable Interrupt */  
#define	VICINTDIS	*((volatile uint32_t*) 0xFFFFF014)

#define	VICVADDR	*((volatile uint32_t*) 0xFFFFF030)

#define	VICDEFVADDR	*((volatile uint32_t*) 0xFFFFF034)

/* Address of the ISR */  
#define	VICVADDR0	*((volatile uint32_t*) 0xFFFFF100)
#define	VICVADDR1	*((volatile uint32_t*) 0xFFFFF104)
#define	VICVADDR2	*((volatile uint32_t*) 0xFFFFF108)
#define	VICVADDR3	*((volatile uint32_t*) 0xFFFFF10C)
#define	VICVADDR4	*((volatile uint32_t*) 0xFFFFF110)
#define	VICVADDR5	*((volatile uint32_t*) 0xFFFFF114)
#define	VICVADDR6	*((volatile uint32_t*) 0xFFFFF118)
#define	VICVADDR7	*((volatile uint32_t*) 0xFFFFF11C)
#define	VICVADDR8	*((volatile uint32_t*) 0xFFFFF120)
#define	VICVADDR9	*((volatile uint32_t*) 0xFFFFF124)
#define	VICVADDR10	*((volatile uint32_t*) 0xFFFFF128)
#define	VICVADDR11	*((volatile uint32_t*) 0xFFFFF12C)
#define	VICVADDR12	*((volatile uint32_t*) 0xFFFFF130)
#define	VICVADDR13	*((volatile uint32_t*) 0xFFFFF134)
#define	VICVADDR14	*((volatile uint32_t*) 0xFFFFF138)
#define	VICVADDR15	*((volatile uint32_t*) 0xFFFFF13C)

/* Vector Control 0 */  
#define	VICCNTL0	*((volatile uint32_t*) 0xFFFFF200)
#define	VICCNTL1	*((volatile uint32_t*) 0xFFFFF204)
#define	VICCNTL2	*((volatile uint32_t*) 0xFFFFF208)
#define	VICCNTL3	*((volatile uint32_t*) 0xFFFFF20C)
#define	VICCNTL4	*((volatile uint32_t*) 0xFFFFF210)
#define	VICCNTL5	*((volatile uint32_t*) 0xFFFFF214)
#define	VICCNTL6	*((volatile uint32_t*) 0xFFFFF218)
#define	VICCNTL7	*((volatile uint32_t*) 0xFFFFF21C)
#define	VICCNTL8	*((volatile uint32_t*) 0xFFFFF220)
#define	VICCNTL9	*((volatile uint32_t*) 0xFFFFF224)
#define	VICCNTL10	*((volatile uint32_t*) 0xFFFFF228)
#define	VICCNTL11	*((volatile uint32_t*) 0xFFFFF22C)
#define	VICCNTL12	*((volatile uint32_t*) 0xFFFFF230)
#define	VICCNTL13	*((volatile uint32_t*) 0xFFFFF234)
#define	VICCNTL14	*((volatile uint32_t*) 0xFFFFF238)
#define	VICCNTL15	*((volatile uint32_t*) 0xFFFFF23C)

#define IRQ_WDT		0x0
#define IRQ_TIMER0	0x4
#define IRQ_TIMER1	0x5
#define IRQ_UART0	0x6
#define IRQ_UART1	0x7
#define IRQ_PWM0	0x8
#define IRQ_IIC		0x9
#define IRQ_SPI		0xA
#define IRQ_RTC		0xD
#define IRQ_EINT0	0xE
#define IRQ_EINT1	0xF
#define IRQ_EINT2	0x10

#define VECTORED_IRQ_WDT	0x20
#define VECTORED_IRQ_TIMER0	0x24
#define VECTORED_IRQ_TIMER1	0x25
#define VECTORED_IRQ_UART0	0x26
#define VECTORED_IRQ_UART1	0x27
#define VECTORED_IRQ_PWM0	0x28
#define VECTORED_IRQ_IIC	0x29
#define VECTORED_IRQ_SPI	0x2A
#define VECTORED_IRQ_RTC	0x2D
#define VECTORED_IRQ_EINT0	0x2E
#define VECTORED_IRQ_EINT1	0x2F
#define VECTORED_IRQ_EINT2	0x30


