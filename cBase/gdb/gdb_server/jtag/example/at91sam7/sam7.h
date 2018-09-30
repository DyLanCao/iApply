/*
 * sam7.h
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
 * 
 * for detailed information see document sheed
 *   at91sam7s64 user manual (6070A-ATARM-28-Oct-04)
 * available at www.atmel.com
 */
#include <inttypes.h>

#define READ_ONLY(_X_) (({(_X_);}))

/* Password value 0xA5 any other value aborts operation */
#define KEY_A5 0xA5000000 

/*********************************
 *********************************
 *********************************
 ***                           ***
 *** System Controller Mapping ***
 ***                           ***
 *********************************
 *********************************
 *********************************/

/************************************************************************
 * 0xFFFFF000 AIC Advanced Interrupt Controller 512 Bytes/128 registers *
 ************************************************************************/
#define PID_EXT_FIQ	0
#define PID_SYS_IRQ	1 // all System interrupts ored together 
			  // DGGU PMC PSTC RTT PIT WDT MC
#define PID_PIOA	2
#define PID_ADC		4
#define PID_SPI		5
#define PID_US0		6
#define PID_US1		7
#define PID_SSC		8
#define PID_TWI		9
#define PID_PWMC	10
#define PID_UDP		11
#define PID_TC0		12
#define PID_TC1		13
#define PID_TC2		14
#define PID_EXT_IRQ0	30
#define PID_EXT_IRQ1	31

/*
 * 0x00 .. 0x7C Source Mode Register 0 AIC_SMR0..31 Read/Write 0x0 
 * Bit 6,5:	SRCTYPE	; Interrupt Source Type
 * 		0 0 Low Level Sensitive 
 * 		0 1 Falling Edge Triggered 
 * 		1 0 High Level Sensitive 
 * 		1 1 Rising Edge Triggered
 * Bit 2,1,0:	PRIOR	; Priority Level (between 0 (lowest) and 7 (highest))
 */
#define AIC_SMR ((volatile uint32_t*) 0xFFFFF000)
#define AIC_SMR0 *((volatile uint32_t*) 0xFFFFF000)
#define AIC_SMR1 *((volatile uint32_t*) 0xFFFFF004)
#define AIC_SMR2 *((volatile uint32_t*) 0xFFFFF008)
#define AIC_SMR3 *((volatile uint32_t*) 0xFFFFF00C)
#define AIC_SMR4 *((volatile uint32_t*) 0xFFFFF010)
#define AIC_SMR5 *((volatile uint32_t*) 0xFFFFF014)
#define AIC_SMR6 *((volatile uint32_t*) 0xFFFFF018)
#define AIC_SMR7 *((volatile uint32_t*) 0xFFFFF01C)
#define AIC_SMR8 *((volatile uint32_t*) 0xFFFFF020)
#define AIC_SMR9 *((volatile uint32_t*) 0xFFFFF024)
#define AIC_SMR10 *((volatile uint32_t*) 0xFFFFF028)
#define AIC_SMR11 *((volatile uint32_t*) 0xFFFFF02C)
#define AIC_SMR12 *((volatile uint32_t*) 0xFFFFF030)
#define AIC_SMR13 *((volatile uint32_t*) 0xFFFFF034)
#define AIC_SMR14 *((volatile uint32_t*) 0xFFFFF038)
#define AIC_SMR15 *((volatile uint32_t*) 0xFFFFF03C)
#define AIC_SMR16 *((volatile uint32_t*) 0xFFFFF040)
#define AIC_SMR17 *((volatile uint32_t*) 0xFFFFF044)
#define AIC_SMR18 *((volatile uint32_t*) 0xFFFFF048)
#define AIC_SMR19 *((volatile uint32_t*) 0xFFFFF04C)
#define AIC_SMR20 *((volatile uint32_t*) 0xFFFFF050)
#define AIC_SMR21 *((volatile uint32_t*) 0xFFFFF054)
#define AIC_SMR22 *((volatile uint32_t*) 0xFFFFF058)
#define AIC_SMR23 *((volatile uint32_t*) 0xFFFFF05C)
#define AIC_SMR24 *((volatile uint32_t*) 0xFFFFF060)
#define AIC_SMR25 *((volatile uint32_t*) 0xFFFFF064)
#define AIC_SMR26 *((volatile uint32_t*) 0xFFFFF068)
#define AIC_SMR27 *((volatile uint32_t*) 0xFFFFF06C)
#define AIC_SMR28 *((volatile uint32_t*) 0xFFFFF070)
#define AIC_SMR29 *((volatile uint32_t*) 0xFFFFF074)
#define AIC_SMR30 *((volatile uint32_t*) 0xFFFFF078)
#define AIC_SMR31 *((volatile uint32_t*) 0xFFFFF07C)
#define AIC_SMR_SRCTYPE_LOW_LEVEL	(0)		// internal and external
#define AIC_SMR_SRCTYPE_FALLING_EDGE	(1<<5)		// internal and external
#define AIC_SMR_SRCTYPE_HIGH_LEVEL	(2<<5)		// external only
#define AIC_SMR_SRCTYPE_RISING_EDGE	(3<<5)		// external only
#define AIC_SMR_PRIOR(_X_)		( (_X_) & 0x7 )

/*
 * 0x80 .. 0xFC Source Vector Register 0..31 AIC_SVR0..AIC_SVR31 Read/Write 0x0 
 * (32 Bit) VECTOR: Source Vector of interrupt handler
 */
#define AIC_SVR   ((volatile uint32_t*) 0xFFFFF080)
#define AIC_SVR0 *((volatile uint32_t*) 0xFFFFF080)
#define AIC_SVR1 *((volatile uint32_t*) 0xFFFFF084)
#define AIC_SVR2 *((volatile uint32_t*) 0xFFFFF088)
#define AIC_SVR3 *((volatile uint32_t*) 0xFFFFF08C)
#define AIC_SVR4 *((volatile uint32_t*) 0xFFFFF090)
#define AIC_SVR5 *((volatile uint32_t*) 0xFFFFF094)
#define AIC_SVR6 *((volatile uint32_t*) 0xFFFFF098)
#define AIC_SVR7 *((volatile uint32_t*) 0xFFFFF09C)
#define AIC_SVR8 *((volatile uint32_t*) 0xFFFFF0A0)
#define AIC_SVR9 *((volatile uint32_t*) 0xFFFFF0A4)
#define AIC_SVR10 *((volatile uint32_t*) 0xFFFFF0A8)
#define AIC_SVR11 *((volatile uint32_t*) 0xFFFFF0AC)
#define AIC_SVR12 *((volatile uint32_t*) 0xFFFFF0B0)
#define AIC_SVR13 *((volatile uint32_t*) 0xFFFFF0B4)
#define AIC_SVR14 *((volatile uint32_t*) 0xFFFFF0B8)
#define AIC_SVR15 *((volatile uint32_t*) 0xFFFFF0BC)
#define AIC_SVR16 *((volatile uint32_t*) 0xFFFFF0C0)
#define AIC_SVR17 *((volatile uint32_t*) 0xFFFFF0C4)
#define AIC_SVR18 *((volatile uint32_t*) 0xFFFFF0C8)
#define AIC_SVR19 *((volatile uint32_t*) 0xFFFFF0CC)
#define AIC_SVR20 *((volatile uint32_t*) 0xFFFFF0D0)
#define AIC_SVR21 *((volatile uint32_t*) 0xFFFFF0D4)
#define AIC_SVR22 *((volatile uint32_t*) 0xFFFFF0D8)
#define AIC_SVR23 *((volatile uint32_t*) 0xFFFFF0DC)
#define AIC_SVR24 *((volatile uint32_t*) 0xFFFFF0E0)
#define AIC_SVR25 *((volatile uint32_t*) 0xFFFFF0E4)
#define AIC_SVR26 *((volatile uint32_t*) 0xFFFFF0E8)
#define AIC_SVR27 *((volatile uint32_t*) 0xFFFFF0EC)
#define AIC_SVR28 *((volatile uint32_t*) 0xFFFFF0F0)
#define AIC_SVR29 *((volatile uint32_t*) 0xFFFFF0F4)
#define AIC_SVR30 *((volatile uint32_t*) 0xFFFFF0F8)
#define AIC_SVR31 *((volatile uint32_t*) 0xFFFFF0FC)

/*
 * 0x100 Interrupt Vector Register AIC_IVR Read-only 0x0 
 * IRQV; Interrupt Vector Register
 */
#define AIC_IVR /*READ_ONLY*/(*((volatile uint32_t*) 0xFFFFF100))

/*
 * 0x104 Fast Interrupt Vector Register AIC_FVR Read-only 0x0 
 * FIQV; FIQ Vector Register
 */
#define AIC_FVR /*READ_ONLY*/(*((volatile uint32_t*) 0xFFFFF104))

/*
 * 0x108 Interrupt Status Register AIC_ISR Read-only 0x0 
 * Bit 4-0:	IRQID	; Current Interrupt Identifier (returns the current interrupt source number)
 */
#define AIC_ISR READ_ONLY(*((volatile uint32_t*) 0xFFFFF108))

/*
 * 0x10C Interrupt Pending Register AIC_IPR Read-only 0x0(1) 
 * Bit 31-2:	PID31-PID2: Interrupt Pending 
 * Bit 1:	SYS
 * Bit 0:	FIQ 
 */
#define AIC_IPR READ_ONLY(*((volatile uint32_t*) 0xFFFFF10C))

/*
 * 0x110 Interrupt Mask Register AIC_IMR Read-only 0x0 
 * Bit 31-2:	PID31-PID2: Interrupt Mask (0 = intr. disabled; 1 = intr. enabled) 
 * Bit 1:	SYS
 * Bit 0:	FIQ 
 */
#define AIC_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFFF110))

/*
 * 0x114 Core Interrupt Status Register AIC_CISR Read-only 0x0 
 * Bit 1:	NIRQ ; NIRQ Status
 * 		0 = nIRQ line is deactivated.
 * 		1 = nIRQ line is active.
 * Bit 0:	NFIQ ; NFIQ Status
 * 		0 = nFIQ line is deactivated.
 * 		1 = nFIQ line is active.
 */
#define AIC_CISR READ_ONLY(*((volatile uint32_t*) 0xFFFFF114))

/*
 * 0x120 Interrupt Enable Command Register AIC_IECR Write-only --- 
 * Bit 31-2:	PID31-PID2: Interrupt Enable (1 = Enables corresponding interrupt)
 * Bit 1:	SYS
 * Bit 0:	FIQ 
 */
#define AIC_IECR *((volatile uint32_t*) 0xFFFFF120)

/*
 * 0x124 Interrupt Disable Command Register AIC_IDCR Write-only --- 
 * Bit 31-2:	PID31-PID2: Interrupt Disable (1 = Disables corresponding interrupt)
 * Bit 1:	SYS
 * Bit 0:	FIQ 
 */
#define AIC_IDCR *((volatile uint32_t*) 0xFFFFF124)

/*
 * 0x128 Interrupt Clear Command Register AIC_ICCR Write-only --- 
 * Bit 31-2:	PID31-PID2: Interrupt Clear (1 = Clear corresponding interrupt)
 * Bit 1:	SYS
 * Bit 0:	FIQ 
 */
#define AIC_ICCR *((volatile uint32_t*) 0xFFFFF128)

/*
 * 0x12C Interrupt Set Command Register AIC_ISCR Write-only --- 
 * Bit 31-2:	PID31-PID2: Interrupt Set (1 = Set corresponding interrupt)
 * Bit 1:	SYS
 * Bit 0:	FIQ 
 */
#define AIC_ISCR *((volatile uint32_t*) 0xFFFFF12C)

/*
 * 0x130 End of Interrupt Command Register AIC_EOICR Write-only ---
 * Any value can be written to indicate that the interrupt treatment is complete.
 */
#define AIC_EOICR *((volatile uint32_t*) 0xFFFFF130)

/*
 * 0x134 Spurious Interrupt Vector Register AIC_SPU Read/Write 0x0 
 * The user may store the address of a spurious interrupt handler in this register. 
 * The written value is returned in AIC_IVR in case of a spurious interrupt 
 * and in AIC_FVR in case of a spurious fast interrupt.
 */
#define AIC_SPU *((volatile uint32_t*) 0xFFFFF134)

/*
 * 0x138 Debug Control Register AIC_DCR Read/Write 0x0 
 * Bit 1:	GMSK	; General Mask 
 * 		0 = nIRQ and nFIQ controlled by the AIC 
 * 		1 = nIRQ and nFIQ tied to inactive state
 * Bit 0:	PROT	; Protection Mode 
 * 		0 = Protection Mode disabled
 * 		1 = Protection Mode enabled
 */
#define AIC_DCR *((volatile uint32_t*) 0xFFFFF138)
#define AIC_DCR_GMSK	(2)
#define AIC_DCR_PROT	(1)

/*
 * 0x140 Fast Forcing Enable Register AIC_FFER Write-only --- 
 * Bit 31-2:	PID31-PID2: Fast Forcing Enable (enables fast forcing feature on the corresponding interrupt)
 * Bit 1:	SYS
 */
#define AIC_FFER *((volatile uint32_t*) 0xFFFFF140)

/*
 * 0x144 Fast Forcing Disable Register AIC_FFDR Write-only --- 
 * Bit 31-2:	PID31-PID2: Fast Forcing Disable
 * Bit 1:	SYS
 */
#define AIC_FFDR *((volatile uint32_t*) 0xFFFFF144)

/*
 * 0x148 Fast Forcing Status Register AIC_FFSR Read-only 0x0
 * Bit 31-2:	PID31-PID2: Fast Forcing Status (0 = disabled; 1 = enabled)
 * Bit 1:	SYS
 */
#define AIC_FFSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF148))

/*******************************************************
 * 0xFFFFF200 DBGU Debug Unit (512 Bytes/128 registers *
 *******************************************************/

/*
 * 0x0000 DBGU_CR Debug Unit Control Register Write-Only
 * Bit 8:	RSTSTA	; 1 = Reset Status Bits (PARE,FRAME and OVRE)
 * Bit 7:	TXDIS	; 1 = Transmitter disable
 * Bit 6:	TXEN	; 1 = Transmitter enable
 * Bit 5:	RXDIS	; 1 = Receiver disable
 * Bit 4:	RXEN	; 1 = Receiver enable
 * Bit 3:	RSTTX	; 1 = Reset and disable Transmitter
 * Bit 2:	RSTRX	; 1 = Reset and disable Receiver
 */
#define DBGU_CR *((volatile uint32_t*) 0xFFFFF200)
#define DBGU_CR_RSTSTA	(1<<8)
#define DBGU_CR_TXDIS	(1<<7)
#define DBGU_CR_TXEN	(1<<6)
#define DBGU_CR_RXDIS	(1<<5)
#define DBGU_CR_RXEN	(1<<4)
#define DBGU_CR_RSTTX	(1<<3)
#define DBGU_CR_RSTRX	(1<<2)

/*
 * 0x0004 DBGU_MR Debug Unit Mode Register
 * Bit 15,14:	CHMODE	; Channel mode
 * Bit 11-9:	PAR	; Parity Type
 */
#define DBGU_MR *((volatile uint32_t*) 0xFFFFF204)
#define DBGU_MR_CHMODE_NORMAL		0
#define DBGU_MR_CHMODE_AUTO_ECHO	(1<<14)
#define DBGU_MR_CHMODE_LOCAL_LOOPBACK	(2<<14)
#define DBGU_MR_CHMODE_REMOTE_LOOPBACK	(3<<14)
#define DBGU_MR_PAR_EVEN	0
#define DBGU_MR_PAR_ODD		(1<<9)
#define DBGU_MR_PAR_SPACE	(2<<9)
#define DBGU_MR_PAR_MARK	(3<<9)
#define DBGU_MR_PAR_NOPAR	(1<<11)


/*************************************************************
 * 0xFFFFF400 PIOA PIO Controller A (512 Bytes/128 registers *
 *************************************************************/
#define PIO_P31	(1<<31)
#define PIO_P30	(1<<30)
#define PIO_P29	(1<<29)
#define PIO_P28	(1<<28)
#define PIO_P27	(1<<27)
#define PIO_P26	(1<<26)
#define PIO_P25	(1<<25)
#define PIO_P24	(1<<24)
#define PIO_P23	(1<<23)
#define PIO_P22	(1<<22)
#define PIO_P21	(1<<21)
#define PIO_P20	(1<<20)
#define PIO_P19	(1<<19)
#define PIO_P18	(1<<18)
#define PIO_P17	(1<<17)
#define PIO_P16	(1<<16)
#define PIO_P15	(1<<15)
#define PIO_P14	(1<<14)
#define PIO_P13	(1<<13)
#define PIO_P12	(1<<12)
#define PIO_P11	(1<<11)
#define PIO_P10	(1<<10)
#define PIO_P9	(1<<9)
#define PIO_P8	(1<<8)
#define PIO_P7	(1<<7)
#define PIO_P6	(1<<6)
#define PIO_P5	(1<<5)
#define PIO_P4	(1<<4)
#define PIO_P3	(1<<3)
#define PIO_P2	(1<<2)
#define PIO_P1	(1<<1)
#define PIO_P0	(1<<0)
	
#define PIO_A_NPCS1	(1<<31)
#define PIO_A_IRQ1	(1<<30)
#define PIO_A_RI1	(1<<29)
#define PIO_A_DSR1	(1<<28)
#define PIO_A_DTR1	(1<<27)
#define PIO_A_DCD1	(1<<26)
#define PIO_A_CTS1	(1<<25)
#define PIO_A_RTS1	(1<<24)
#define PIO_A_SCK1	(1<<23)
#define PIO_A_TXD1	(1<<22)
#define PIO_A_RXD1	(1<<21)
#define PIO_A_RF	(1<<20)
#define PIO_A_RK	(1<<19)
#define PIO_A_RD	(1<<18)
#define PIO_A_TD	(1<<17)
#define PIO_A_TK	(1<<16)
#define PIO_A_TF	(1<<15)
#define PIO_A_SPCK	(1<<14)
#define PIO_A_MOSI	(1<<13)
#define PIO_A_MISO	(1<<12)
#define PIO_A_NPCS0	(1<<11)
#define PIO_A_DTXD	(1<<10)
#define PIO_A_DRXD	(1<<9)
#define PIO_A_CTS0	(1<<8)
#define PIO_A_RTS0	(1<<7)
#define PIO_A_TXD0	(1<<6)
#define PIO_A_RXD0	(1<<5)
#define PIO_A_TWCK	(1<<4)
#define PIO_A_TWD	(1<<3)
#define PIO_A_PWM2	(1<<2)
#define PIO_A_PWM1	(1<<1)
#define PIO_A_PWM0	(1<<0)
	
/*
 * 0x0000 PIO Enable Register PIO_PER Write-only 
 * Bit 31-0:	P31-P0	; PIO pin enable 
 * 			(1= enables the PIO and 
 * 			disable peripheral control of the pin)
 */
#define PIO_PER *((volatile uint32_t*) 0xFFFFF400)

/*
 * 0x0004 PIO Disable Register PIO_PDR Write-only 
 * Bit 31-0:	P31-P0	; PIO pin disable 
 * 			(1= disables the PIO and 
 * 			enable peripheral control of the pin)
 */
#define PIO_PDR *((volatile uint32_t*) 0xFFFFF404)

/*
 * 0x0008 PIO Status Register (1) PIO_PSR Read-only 0x0
 * Bit 31-0:	P31-P0	; PIO pin status 
 * 			(1= PIO enabled; 0= peripheral control enabled)
 */
#define PIO_PSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF408))

/*
 * 0x0010 Output Enable Register PIO_OER Write-only 
 * Bit 31-0:	P31-P0	; PIO output enable 
 */
#define PIO_OER *((volatile uint32_t*) 0xFFFFF410)

/*
 * 0x0014 Output Disable Register PIO_ODR Write-only 
 * Bit 31-0:	P31-P0	; PIO output disable 
 */
#define PIO_ODR *((volatile uint32_t*) 0xFFFFF414)

/*
 * 0x0018 Output Status Register PIO_OSR Read-only 0x0
 * Bit 31-0:	P31-P0	; PIO output status
 * 			0 = input
 * 			1 = output
 */
#define PIO_OSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF418))

/*
 * 0x0020 Glitch Input Filter Enable Register PIO_IFER Write-only 
 * Bit 31-0:	P31-P0	; PIO input glitch filter enable
 */
#define PIO_IFER *((volatile uint32_t*) 0xFFFFF420)

/*
 * 0x0024 Glitch Input Filter Disable Register PIO_IFDR Write-only 
 * Bit 31-0:	P31-P0	; PIO input glitch filter disable
 */
#define PIO_IFDR *((volatile uint32_t*) 0xFFFFF424)

/*
 * 0x0028 Glitch Input Filter Status Register PIO_IFSR Read-only 0x0
 * Bit 31-0:	P31-P0	; PIO input glitch filter status
 * 			(0 = filter disabled; 1 = filter enabled)
 */
#define PIO_IFSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF428))

/*
 * 0x0030 Set Output Data Register PIO_SODR Write-only 
 * Bit 31-0:	P31-P0	; PIO output data high (or high impedance if open drain enabled)
 */
#define PIO_SODR *((volatile uint32_t*) 0xFFFFF430)

/*
 * 0x0034 Clear Output Data Register PIO_CODR Write-only 
 * Bit 31-0:	P31-P0	; PIO output data low
 */
#define PIO_CODR *((volatile uint32_t*) 0xFFFFF434)

/*
 * 0x0038 Output Data Status Register(2) PIO_ODSR Read/Write 0x0
 * Bit 31-0:	P31-P0	; PIO output data staus (0 = low; 1 = high)
 * 			if PIO_OWSR = 0 this is a read only register
 * 			if PIO_OWSR = 1 a write to the register affects the I/O line
 */
#define PIO_ODSR *((volatile uint32_t*) 0xFFFFF438)

/*
 * 0x003C Pin Data Status Register(3) PIO_PDSR Read-only 
 * Bit 31-0:	P31-P0	; PIO input pin data staus (0 = low; 1 = high)
 */
#define PIO_PDSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF43C))

/*
 * 0x0040 Interrupt Enable Register PIO_IER Write-only  
 * Bit 31-0:	P31-P0	; Enable interrupt on PIO input change
 */
#define PIO_IER *((volatile uint32_t*) 0xFFFFF440)

/*
 * 0x0044 Interrupt Disable Register PIO_IDR Write-only  
 * Bit 31-0:	P31-P0	; Disable interrupt on PIO input change
 */
#define PIO_IDR *((volatile uint32_t*) 0xFFFFF444)

/*
 * 0x0048 Interrupt Mask Register PIO_IMR Read-only 0x0
 * Bit 31-0:	P31-P0	; Interrupt mask of PIO input change
 * 			(0 = interrupt disabled; 1 = interrupt enabled)
 */
#define PIO_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFFF448))

/*
 * 0x004C Interrupt Status Register(4) PIO_ISR Read-only 0x0
 * Bit 31-0:	P31-P0	; PIO input change status
 * 			0 = no input change has been detected since last PIO_ISR
 * 			1 = a input change has been detected
 */
#define PIO_ISR READ_ONLY(*((volatile uint32_t*) 0xFFFFF44C))

/*
 * 0x0050 Multi-driver Enable Register PIO_MDER Write-only  
 * Bit 31-0:	P31-P0	; Enable open drain on PIO output
 */
#define PIO_MDER *((volatile uint32_t*) 0xFFFFF450)

/*
 * 0x0054 Multi-driver Disable Register PIO_MDDR Write-only  
 * Bit 31-0:	P31-P0	; Disable open drain on PIO output
 */
#define PIO_MDDR *((volatile uint32_t*) 0xFFFFF454)

/*
 * 0x0058 Multi-driver Status Register PIO_MDSR Read-only 0x0
 * Bit 31-0:	P31-P0	; Open drain status of PIO output
 * 			0 = Pin can be driven high or low
 * 			1 = Pin can be driven to low level only
 */
#define PIO_MDSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF458))

/*
 * 0x0060 Pull-up Disable Register PIO_PUDR Write-only  
 * Bit 31-0:	P31-P0	; Disable pull up on PIO pin
 */
#define PIO_PUDR *((volatile uint32_t*) 0xFFFFF460)

/*
 * 0x0064 Pull-up Enable Register PIO_PUER Write-only  
 * Bit 31-0:	P31-P0	; Enable pull up on PIO pin
 */
#define PIO_PUER *((volatile uint32_t*) 0xFFFFF464)

/*
 * 0x0068 Pad Pull-up Status Register PIO_PUSR Read-only 0x0
 * Bit 31-0:	P31-P0	; Pull up status of PIO pin
 * 			(0 = enabled; 1 = disabled)
 */
#define PIO_PUSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF468))

/*
 * 0x0070 Peripheral A Select Register(5) PIO_ASR Write-only 
 * Bit 31-0:	P31-P0	; Assign IO line to the Peripheral A function
 */
#define PIO_ASR *((volatile uint32_t*) 0xFFFFF470)

/*
 * 0x0074 Peripheral B Select Register(5) PIO_BSR Write-only  
 * Bit 31-0:	P31-P0	; Assign IO line to the Peripheral B function
 */
#define PIO_BSR *((volatile uint32_t*) 0xFFFFF474)

/*
 * 0x0078 AB Status Register(5) PIO_ABSR Read-only 0x0
 * Bit 31-0:	P31-P0	; Assign status of IO line
 * 			0 = assigned to Peripheral A
 * 			1 = assigned to Peripheral B
 */
#define PIO_ABSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF478))

/*
 * 0x00A0 Output Write Enable PIO_OWER Write-only 
 * Bit 31-0:	P31-P0	; Enables Output Write via PIO_ODSR
 */
#define PIO_OWER *((volatile uint32_t*) 0xFFFFF4A0)

/*
 * 0x00A4 Output Write Disable PIO_OWDR Write-only  
 * Bit 31-0:	P31-P0	; Disables Output Write via PIO_ODSR
 */
#define PIO_OWDR *((volatile uint32_t*) 0xFFFFF4A4)

/*
 * 0x00A8 Output Write Status Register PIO_OWSR Read-only 0x0
 * Bit 31-0:	P31-P0	; Output Write Status. 
 * 			0 = Writing PIO_ODSR has not effect
 * 			1 = Writing PIO_ODSR affects the IO line
 */
#define PIO_OWSR READ_ONLY(*((volatile uint32_t*) 0xFFFFF4A8))

/**********************************************************************
 * 0xFFFFFC00 PMC Power Management Controller (256 Bytes/64 registers *
 **********************************************************************/
/*
 * 0x0000 System Clock Enable Register PMC_SCER Write-only 
 * Bit 10:	PCK2	; Programmable Clock x Output Enable
 * Bit 9:	PCK1	; 
 * Bit 8:	PCK0	; 
 * Bit 7:	UDP	; USB Device Port Clock Enable (Enables the 48 MHz USB clock)
 * Bit 0:	PCK	; Processor Clock Enable
 */
#define PMC_SCER *((volatile uint32_t*) 0xFFFFFC00)
#define PMC_SCER_PCK2	(1<<10)
#define PMC_SCER_PCK1	(1<<9)
#define PMC_SCER_PCK0	(1<<8)
#define PMC_SCER_UDP	(1<<7)
#define PMC_SCER_PCK	(1)

/*
 * 0x0004 System Clock Disable Register PMC_SCDR Write-only 
 * Bit 10:	PCK2	; Programmable Clock x Output Disable
 * Bit 9:	PCK1	; 
 * Bit 8:	PCK0	; 
 * Bit 7:	UDP	; USB Device Port Clock Disable
 * Bit 0:	PCK	; Processor Clock Disable
 */
#define PMC_SCDR *((volatile uint32_t*) 0xFFFFFC04)
#define PMC_SCDR_PCK2	(1<<10)
#define PMC_SCDR_PCK1	(1<<9)
#define PMC_SCDR_PCK0	(1<<8)
#define PMC_SCDR_UDP	(1<<7)
#define PMC_SCDR_PCK	(1)

/*
 * 0x0008 System Clock Status Register PMC _SCSR Read-only 0x01 
 * Bit 10:	PCK2	; Programmable Clock x Output Status
 * Bit 9:	PCK1	; (0 = disabled; 1 = enabled)
 * Bit 8:	PCK0	; 
 * Bit 7:	UDP	; USB Device Port Clock Status
 * Bit 0:	PCK	; Processor Clock Status
 */
#define PMC_SCSR READ_ONLY(*((volatile uint32_t*) 0xFFFFFC08))
#define PMC_SCSR_PCK2	(1<<10)
#define PMC_SCSR_PCK1	(1<<9)
#define PMC_SCSR_PCK0	(1<<8)
#define PMC_SCSR_UDP	(1<<7)
#define PMC_SCSR_PCK	(1)

/*
 * 0x0010 Peripheral Clock Enable Register PMC_PCER Write-only 
 * Bit 31-2: PID31-PID2
 */
#define PMC_PCER *((volatile uint32_t*) 0xFFFFFC10)
#define PMC_PCER_PID(_ID_)	(1<<(_ID_))

/*
 * 0x0014 Peripheral Clock Disable Register PMC_PCDR Write-only 
 * Bit 31-2: PID31-PID2
 */
#define PMC_PCDR *((volatile uint32_t*) 0xFFFFFC14)
#define PMC_PCDR_PID(_ID_)	(1<<(_ID_))

/*
 * 0x0018 Peripheral Clock Status Register PMC_PCSR Read-only 0x0 
 * Bit 31-2: PID31-PID2 ; (0 = disabled; 1 = enabled)
 */
#define PMC_PCSR READ_ONLY(*((volatile uint32_t*) 0xFFFFFC18))
#define PMC_PCSR_PID(_ID_)	(1<<(_ID_))

/*
 * 0x0020 Main Oscillator Register CKGR_MOR Read/Write 0x0 
 * Bit 15-8:	OSCOUNT		; Main Oscillator Start-up Time (= 8 * Slow_Clock cycles)
 * Bit 1:	OSCBYPASS	; Oscillator Bypass (MOSCEN must be set to 0)
 * Bit 0:	MOSCEN		; Main Oscillator Enable (crystal must be between XIN and XOUT)
 * 				  and OSCBYPASS must be set to 0
 */
#define CKGR_MOR *((volatile uint32_t*) 0xFFFFFC20)
#define CKGR_MOR_OSCOUNT_MSK		(0xFF00)
#define CKGR_MOR_OSCOUNT(_X_)		( ((_X_)<<8) & CKGR_MOR_OSCOUNT_MSK )
#define CKGR_MOR_OSCOUNT_VAL(_X_)	( ((_X_) & CKGR_MOR_OSCOUNT_MSK)>>8 )
#define CKGR_MOR_OSCBYPASS		(2)
#define CKGR_MOR_MOSCEN			(1)

/*
 * 0x0024 Main Clock Frequency Register CKGR_MCFR Read-only 0x0 
 * Bit 16:	MAINRDY	; Main Clock Ready
 * Bit 15-0:	MAINF	; Main Clock Frequency
 * 		(number of Main Clock cycles within 16 Slow Clock periods)
 */
#define CKGR_MCFR READ_ONLY(*((volatile uint32_t*) 0xFFFFFC24))
#define CKGR_MCFR_MAINRDY		(1<<16)
#define CKGR_MCFR_MAINF_VAL(_X_)	( (_X_) & 0xFFFF )

/*
 * 0x002C PLL Register CKGR_PLLR Read/Write 0x3F00 
 * Bit 29,28:	USBDIV	;Divider for USB Clock
 * 		0 0 USB_Clock = PLL_clock / 1
 * 		0 1 USB_Clock = PLL_clock / 2 
 * 		1 0 USB_Clock = PLL_clock / 4
 * Bit 26-16:	MUL	;PLL Multiplier
 * 		0 = PLL deactivated; else PLL_Clock = PLL_input * (MUL+ 1)
 * Bit 15,14:	OUTPLL	;Clock Frequency Range
 * 		0 0 = see DC spec (80 MHz - 160 MHz)
 * 		1 0 = see DC spec (150 MHz - 220 MHz)
 * Bit 13-8:	COUNT	;PLL Counter (number of slow_clock cycles before LOCK bit is set)
 * Bit 7-0:	DIV	;Divider 
 * 		0 Divider output is ???
 * 		1 Divider is bypassed 
 * 		2 - 255 Divider output is the selected clock divided by DIV. 
 */
#define CKGR_PLLR *((volatile uint32_t*) 0xFFFFFC2C)
#define CKGR_PLLR_USBDIV_MSK		(3<<28)
#define CKGR_PLLR_USBDIV(_X_)		( ((_X_)<<28) & CKGR_PLLR_USBDIV_MSK )
#define CKGR_PLLR_USBDIV_VAL(_X_)	( ((_X_) & CKGR_PLLR_USBDIV_MSK)>>28 )
#define CKGR_PLLR_MUL_MSK		(0x7ff0000)
#define CKGR_PLLR_MUL(_X_)		( ((_X_)<<16) & CKGR_PLLR_MUL_MSK )
#define CKGR_PLLR_MUL_VAL(_X_)		( ((_X_) & CKGR_PLLR_MUL_MSK)>>16 )
#define CKGR_PLLR_OUTPLL_MSK		(3<<14)
#define CKGR_PLLR_OUTPLL(_X_)		( ((_X_)<<14) & CKGR_PLLR_OUTPLL_MSK )
#define CKGR_PLLR_OUTPLL_VAL(_X_)	( ((_X_) & CKGR_PLLR_OUTPLL_MSK)>>14 )
#define CKGR_PLLR_COUNT_MSK		(0x3F00)
#define CKGR_PLLR_COUNT(_X_)		( ((_X_)<<8) & CKGR_PLLR_COUNT_MSK )
#define CKGR_PLLR_COUNT_VAL(_X_)	( ((_X_) & CKGR_PLLR_COUNT_MSK)>>8 )
#define CKGR_PLLR_DIV_MSK		(0xFF)
#define CKGR_PLLR_DIV(_X_)		( (_X_) & CKGR_PLLR_DIV_MSK )
#define CKGR_PLLR_DIV_VAL(_X_)		( (_X_) & CKGR_PLLR_DIV_MSK )

/*
 * 0x0030 Master Clock Register PMC_MCKR Read/Write 0x0 
 * Bit 4,3,2:	PRES	;Master Clock Prescaler
 * 			0 0 0 Selected clock / 1
 * 			0 0 1 Selected clock / 2 
 * 			0 1 0 Selected clock / 4 
 * 			0 1 1 Selected clock / 8 
 * 			1 0 0 Selected clock / 16 
 * 			1 0 1 Selected clock / 32 
 * 			1 1 0 Selected clock / 64 
 * Bit 1,0:	CSS	;Master Clock Selection
 * 			0 0 Slow Clock is selected 
 * 			0 1 Main Clock is selected 
 * 			1 1 PLL Clock is selected
 */
#define PMC_MCKR *((volatile uint32_t*) 0xFFFFFC30)

#define PMC_PRES_MSK		(7<<2)
#define PMC_PRES(_X_)		( ((_X_)<<2) & CKGR_MCKR_PRES_MSK )
#define PMC_PRES_VAL(_X_)	( ((_X_) & CKGR_MCKR_PRES_MSK)>>2 )
#define PMC_PRES_CLK_1		(0)
#define PMC_PRES_CLK_2		(1<<2)
#define PMC_PRES_CLK_4		(2<<2)
#define PMC_PRES_CLK_8		(3<<2)
#define PMC_PRES_CLK_16		(4<<2)
#define PMC_PRES_CLK_32		(5<<2)
#define PMC_PRES_CLK_64		(6<<2)
#define PMC_CSS_SLOW_CLK	(0)
#define PMC_CSS_MAIN_CLK	(1)
#define PMC_CSS_PLL_CLK		(3)

/*
 * 0x0040 Programmable Clock x Register PMC_PCKx Read/Write 0x0 
 * Bit 4,3,2:	PRES	;Clock Prescaler
 * 			0 0 0 Selected clock / 1
 * 			0 0 1 Selected clock / 2 
 * 			0 1 0 Selected clock / 4 
 * 			0 1 1 Selected clock / 8 
 * 			1 0 0 Selected clock / 16 
 * 			1 0 1 Selected clock / 32 
 * 			1 1 0 Selected clock / 64 
 * Bit 1,0:	CSS	;Clock Selection
 * 			0 0 Slow Clock is selected 
 * 			0 1 Main Clock is selected 
 * 			1 1 PLL Clock is selected
 */
#define PMC_PCK0 *((volatile uint32_t*) 0xFFFFFC40)
#define PMC_PCK1 *((volatile uint32_t*) 0xFFFFFC44)
#define PMC_PCK2 *((volatile uint32_t*) 0xFFFFFC48)

/*
 * 0x0060 Interrupt Enable Register PMC_IER Write-only -- 
 * Bit 10:	PCKRDY2	;
 * Bit 9:	PCKRDY1	;
 * Bit 8:	PCKRDY0	;Programmable Clock Ready x Interrupt Enable
 * Bit 3:	MCKRDY	;Master Clock Ready Interrupt Enable
 * Bit 2:	LOCK	;PLL Lock Interrupt Enable
 * Bit 0:	MOSCS	;Main Oscillator Status Interrupt Enable
 */
#define PMC_IER *((volatile uint32_t*) 0xFFFFFC60)
#define PMC_IER_PCKRDY2	(1<<10)
#define PMC_IER_PCKRDY1	(1<<9)
#define PMC_IER_PCKRDY0	(1<<8)
#define PMC_IER_MCKRDY	(1<<3)
#define PMC_IER_LOCK	(1<<2)
#define PMC_IER_MOSCS	(1)

/*
 * 0x0064 Interrupt Disable Register PMC_IDR Write-only -- 
 * Bit 10:	PCKRDY2	;
 * Bit 9:	PCKRDY1	;
 * Bit 8:	PCKRDY0	;Programmable Clock Ready x Interrupt Disable
 * Bit 3:	MCKRDY	;Master Clock Ready Interrupt Disable
 * Bit 2:	LOCK	;PLL Lock Interrupt Disable
 * Bit 0:	MOSCS	;Main Oscillator Status Interrupt Disable
 */
#define PMC_IDR *((volatile uint32_t*) 0xFFFFFC64)
#define PMC_IDR_PCKRDY2	(1<<10)
#define PMC_IDR_PCKRDY1	(1<<9)
#define PMC_IDR_PCKRDY0	(1<<8)
#define PMC_IDR_MCKRDY	(1<<3)
#define PMC_IDR_LOCK	(1<<2)
#define PMC_IDR_MOSCS	(1)

/*
 * 0x0068 Status Register PMC_SR Read-only 0x08 
 * Bit 10:	PCKRDY2	;Programmable Clock Ready Status
 * Bit 9:	PCKRDY1	;(0 = Clock not ready)
 * Bit 8:	PCKRDY0	;(1 = Clock ready)
 * Bit 3:	MCKRDY	;Master Clock Status (0 = not ready; 1 = ready)
 * Bit 2:	LOCK	;PLL Lock Status (0 = not locked; 1 = locked)
 * Bit 0:	MOSCS	;Main oscillator flag (0 = not stable; 1 = stable)
 */
#define PMC_SR READ_ONLY(*((volatile uint32_t*) 0xFFFFFC68))
#define PMC_SR_PCKRDY2	(1<<10)
#define PMC_SR_PCKRDY1	(1<<9)
#define PMC_SR_PCKRDY0	(1<<8)
#define PMC_SR_MCKRDY	(1<<3)
#define PMC_SR_LOCK	(1<<2)
#define PMC_SR_MOSCS	(1)

/*
 * 0x006C Interrupt Mask Register PMC_IMR Read-only 0x0
 * Bit 10:	PCKRDY2	;Programmable Clock Ready x Interrupt Status
 * Bit 9:	PCKRDY1	;(0 = interrupt disabled)
 * Bit 8:	PCKRDY0	;(1 = interrupt enabled)
 * Bit 3:	MCKRDY	;Master Clock Ready Interrupt Status
 * Bit 2:	LOCK	;PLL Lock Interrupt Status
 * Bit 0:	MOSCS	;Main Oscillator Status Interrupt Status
 */
#define PMC_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFFFC6C))
#define PMC_IMR_PCKRDY2	(1<<10)
#define PMC_IMR_PCKRDY1	(1<<9)
#define PMC_IMR_PCKRDY0	(1<<8)
#define PMC_IMR_MCKRDY	(1<<3)
#define PMC_IMR_LOCK	(1<<2)
#define PMC_IMR_MOSCS	(1)


/***********************************************************
 * 0xFFFFFD00 RSTC Reset Controller (16 Bytes/4 registers) *
 ***********************************************************/

/*
 * 0x00 Control Register RSTC_CR Write-only - 
 * Bit 31-24:	KEY_A5 
 * Bit 3:	EXTRST	;External Reset (asserts the NRST pin) 
 * Bit 2:	PERRST	;Peripheral Reset (resets on chip peripherals)
 * Bit 0:	PROCRST	;Processor Reset (resets CPU core)
 */
#define RSTC_CR *((volatile uint32_t*) 0xFFFFFD00)
#define RSTC_CR_EXTRST (1<<3)
#define RSTC_CR_PERRST (1<<2)
#define RSTC_CR_PROCRST (1)

/*
 * 0x04 Status Register RSTC_SR Read-only 0x00000000 
 * Bit 17:	SRCMP	;Software Reset Command in Progress
 * Bit 16:	NRSTL	;NRST Pin Level
 * Bit 10,9,8:	RSTTYP	;Reset Type
 * 		0 0 0	Power-up Reset (VDDCORE rising)
 * 		0 1 0	Watchdog Reset (Watchdog fault occurred)
 * 		0 1 1	Software Reset (Processor reset required by the software)
 * 		1 0 0	User Reset (NRST pin detected low)
 * 		1 0 1	Brownout Reset (BrownOut reset occurred)
 * Bit 1:	BODSTS	;Brownout Detection Status
 * 		0 = No brownout high-to-low transition detected.
 * 		1 = A brownout high-to-low transition has been detected.
 * Bit 0:	URSTS	;User Reset Status
 * 		0 = No high-to-low edge on NRST detected.
 * 		1 = A high-to-low transition of NRST has been detected.
 * 		
 */
#define RSTC_SR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD04))
#define RSTC_SR_SRCMP		(1<<17)
#define RSTC_SR_NRSTL		(1<<16)
#define RSTC_SR_RSTTYP_MSK	(7<<8)
#define RSTC_SR_RSTTYP_POWERUP	0
#define RSTC_SR_RSTTYP_WATCHDOG (2<<8)
#define RSTC_SR_RSTTYP_SOFTWARE	(3<<8)
#define RSTC_SR_RSTTYP_USER	(4<<8)
#define RSTC_SR_RSTTYP_BROWNOUT	(5<<8)
#define RSTC_SR_BODSTS		(1<<1)
#define RSTC_SR_URSTS		(1)

/* 
 * 0x08 Mode Register RSTC_MR Read/Write 0x00000000
 * Bit 31-24:	KEY_A5 
 * Bit 16:	BODIEN	;Brownout Detection Interrupt Enable
 * 		0 = RSTC_SR_BODSTS bit has no effect on rstc_irq.
 * 		1 = RSTC_SR_BODSTS bit asserts rstc_irq. 
 * Bit 11-8:	ERSTL	;External Reset Length (during a time of 2(ERSTL+1) Slow Clock cycles)
 * Bit 4:	URSTIEN	;User Reset Interrupt Enable
 * 		0 = RSTC_SR_USRTS has no effect on rstc_irq
 * 		1 = RSTC_SR_USRTS asserts rstc_irq if URSTEN = 0. 
 * Bit 0:	URSTEN	;User Reset Enable
 * 		0 = ignore low level on pin NRST  
 * 		1 = low level on pin NRST triggers a User Reset
 */
#define RSTC_MR *((volatile uint32_t*) 0xFFFFFD08)
#define RSTC_MR_BODIEN		(1<<16)
#define RSTC_MR_URSTIEN		(1<<4)
#define RSTC_MR_URSTEN		(1)
#define RSTC_MR_ERSTL_MSK	(0xF<<8)
#define RSTC_MR_ERSTL(_X_)	( ((_X_)<<8) & RSTC_MR_ERSTL_MSK )
#define RSTC_MR_ERSTL_VAL(_X_)	( ((_X_)& RSTC_MR_ERSTL_MSK)>>8 )

/*********************************************************
 * 0xFFFFFD20 RTT Real-time Timer (16 Bytes/4 registers) *
 *********************************************************/

/*
 * 0x00 Mode Register RTT_MR Read/Write 0x00008000
 * Bit 18:	RTTRST	;Real-time Timer Restart
 * Bit 17:	RTTINCIEN;Real-time Timer Increment Interrupt Enable
 * Bit 16:	ALMIEN	;Alarm Interrupt Enable
 * Bit 15-0:	RTPRES	;Real-time Timer Prescaler Value (if 0 then it used 216 instead)
 */
#define RTT_MR *((volatile uint32_t*) 0xFFFFFD20)
#define RTT_MR_RTTRST		(1<<18)
#define RTT_MR_RTTINCIEN	(1<<17)
#define RTT_MR_ALMIEN		(1<<16)
#define RTT_MR_RTPRES_MSK	(0xFFFF)
#define RTT_MR_RTPRES(_X_)	( (_X_) & RTT_MR_RTPRES_MSK )
#define RTT_MR_RTPRES_VAL(_X_)	( (_X_) & RTT_MR_RTPRES_MSK )

/*
 * 0x04 Alarm Register RTT_AR Read/Write 0xFFFFFFFF
 * (32 Bit) ALMV; Alarm Value
 */
#define RTT_AR *((volatile uint32_t*) 0xFFFFFD24)

/*
 * 0x08 Value Register RTT_VR Read-only 0x00000000 
 * (32 Bit) CRTV; Current Real-time Value
 */
#define RTT_VR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD28))

/*
 * 0x0C Status Register RTT_SR Read-only 0x00000000
 * Bit 1:	RTTINC	;Timer has been incremented
 * Bit 0:	ALMS	;Alarm occurred
 */
#define RTT_SR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD2C))
#define RTT_SR_RTTINC	(2)
#define RTT_SR_ALMS	(1)

/*****************************************************************
 * 0xFFFFFD30 PIT Periodic Interval Timer (16 Bytes/4 registers) *
 *****************************************************************/

/*
 * 0x00 Mode Register PIT_MR Read/Write 0x000FFFFF 
 * Bit 25:	PITIEN	;Periodic Interval Timer Interrupt Enable
 * Bit 24:	PITEN	;Period Interval Timer Enabled
 * Bit 19-0:	PIV	;Periodic Interval Value (20 bit; The period is equal to (PIV + 1))
 */
#define PIT_MR *((volatile uint32_t*) 0xFFFFFD30)
#define PIT_MR_PITIEN		(1<<25)
#define PIT_MR_PITEN		(1<<24)
#define PIT_MR_PIV_MSK		(0xfFFFF)
#define PIT_MR_PIV(_X_)		( (_X_) & PIT_MR_PIV_MSK )
#define PIT_MR_PIV_VAL(_X_)	( (_X_) & PIT_MR_PIV_MSK )

/*
 * 0x04 Status Register PIT_SR Read-only 0x00000000 
 * Bit 0:	PITS;	Periodic Interval Timer Status (timer has reached PIV)
 */
#define PIT_SR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD34))
#define PIT_SR_PITS	(1)

/*
 * 0x08 Periodic Interval Value Register PIT_PIVR Read-only 0x00000000
 * Bit 32-20:	PICNT	;Periodic Interval Counter (number of intervals since last read)
 * Bit 19-0:	CPIV	;Current Periodic Interval Value
 */
#define PIT_PIVR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD38))
#define PIT_PIVR_PICNT_MSK	(0xfff00000)
#define PIT_PIVR_PICNT(_X_)	( ((_X_)<<20) & PIT_PIVR_PICNT_MSK)
#define PIT_PIVR_PICNT_VAL(_X_)	( ((_X_) & PIT_PIVR_PICNT_MSK)>>20 )
#define PIT_PIVR_CPIV_MSK	(0xfFFFF)
#define PIT_PIVR_CPIV(_X_)	( (_X_) & PIT_PIVR_PICNT_MSK )
#define PIT_PIVR_CPIV_VAL(_X_)	( (_X_) & PIT_PIVR_PICNT_MSK )

/*
 * 0x0C Periodic Interval Image Register PIT_PIIR Read-only 0x00000000
 * Bit 32-20:	PICNT	;Periodic Interval Counter (number of intervals since last read of PIT_PIVR)
 * Bit 19-0:	CPIV	;Current Periodic Interval Value
 */
#define PIT_PIIR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD3C))
#define PIT_PIIR_PICNT_MSK	(0xfff00000)
#define PIT_PIIR_PICNT(_X_)	( ((_X_)<<20) & PIT_PIIR_PICNT_MSK)
#define PIT_PIIR_PICNT_VAL(_X_)	( ((_X_) & PIT_PIIR_PICNT_MSK)>>20 )
#define PIT_PIIR_CPIV_MSK	(0xfFFFF)
#define PIT_PIIR_CPIV(_X_)	( (_X_) & PIT_PIIR_PICNT_MSK )
#define PIT_PIIR_CPIV_VAL(_X_)	( (_X_) & PIT_PIIR_PICNT_MSK )


/********************************************************
 * 0xFFFFFD40 WDT Watchdog Timer (16 Bytes/4 registers) *
 ********************************************************/
/*
 * 0x00 Control Register WDT_CR Write-only - 
 * Bit 31-23:	KEY_A5
 * Bit 0:	WDRSTT	;Watchdog Restart (retriggers Watchdog timer)
 */
#define WDT_CR *((volatile uint32_t*) 0xFFFFFD40)
#define WDT_CR_WDRSTT	(1)

/*
 * 0x04 Mode Register WDT_MR Read/Write Once 0x3FFF2FFF
 * (The Watchdog Mode Register can only be written once after reset)
 * Bit 29:	WDIDLEHLT ; Watchdog Idle Halt
 * 		0 = Watchdog runs even when the system is in idle mode. 
 * 		1 = Watchdog stops when the system is in idle state.
 * Bit 28:	WDDBGHLT ; Watchdog Debug Halt
 * 		0 = Watchdog runs even when the processor is in debug state. 
 * 		1 = Watchdog stops when the processor is in debug state.
 * Bit 27-16:	WDD	; Watchdog Delta Value
 * 		Defines the permitted range for reloading the Watchdog Timer. 
 * 		If Watchdog Timer value <= WDD, a Watchdog restarts is permitted. 
 * 		If the Watchdog Timer > WDD, a Watchdog restart causes a Watchdog error. 
 * Bit 15:	WDDIS	; Watchdog Disable 
 * Bit 14:	WDRPROC	; Watchdog Reset Processor
 * 		0 = if WDRSTEN == 1, underflow or error activates all resets. 
 * 		1 = if WDRSTEN == 1, underflow or error activates the processor reset. 
 * Bit 13:	WDRSTEN	; Watchdog Reset Enable (underflow or error triggers a Watchdog reset)
 * Bit 12:	WDFIEN	; Watchdog Fault Interrupt Enable (underflow or error asserts interrupt)
 * Bit 11-0:	WDV	; Watchdog Counter Value (12-bit restart value -- using slow_clock)
 */
#define WDT_MR *((volatile uint32_t*) 0xFFFFFD44)
#define WDT_MR_WDIDLEHLT	(1<<29)
#define WDT_MR_WDDBGHLT		(1<<28)
#define WDT_MR_WDD_MSK		(0x0fff0000)
#define WDT_MR_WDD(_X_)		( ((_X_)<<16) & WDT_MR_WDD_MSK)
#define WDT_MR_WDD_VAL(_X_)	( ((_X_) & WDT_MR_WDD_MSK)>>16 )
#define WDT_MR_WDDIS		(1<<15)
#define WDT_MR_WDRPROC		(1<<14)
#define WDT_MR_WDRSTEN		(1<<13)
#define WDT_MR_WDFIEN		(1<<12)
#define WDT_MR_WDV_MSK		(0xfff)
#define WDT_MR_WDV(_X_)		( (_X_) & WDT_MR_WDV_MSK )
#define WDT_MR_WDV_VAL(_X_)	( (_X_) & WDT_MR_WDV_MSK )

/*
 * 0x08 Status Register WDT_SR Read-only 0x00000000 
 * Bit 1:	WDERR	; Watchdog Error (a error occurred since last read)
 * Bit 0:	WDUNF	; Watchdog Underflow (a underflow occurred since last read)
 */
#define WDT_SR READ_ONLY(*((volatile uint32_t*) 0xFFFFFD48))
#define WDT_SR_WDERR	(2)
#define WDT_SR_WDUNF	(1)

/**************************************************************************
 * 0xFFFFFD60 VREG Voltage Regulator Mode Controller (4 Bytes/1 register) *
 **************************************************************************/
/*
 * 0x00 Voltage Regulator Mode Register VREG_MR Read/Write 0x0
 * Bit 0:	PSTDBY; Power Standby
 * 		0 = regulator running (normal mode)
 * 		1 = regulator enter standby (low-power mode). 
 */
#define VREG_MR *((volatile uint32_t*) 0xFFFFFD60)
#define VREG_MR_PSTDBY	(1)

/************************************************************
 * 0xFFFFFF00 MC Memory Controller (256 Bytes/64 registers) *
 ************************************************************/
/*
 * 0x00 MC Remap Control Register MC_RCR Write-only
 * Bit 0:	RCB; Remap Command Bit (Toggle Map at address 0) 
 */
#define MC_RCR *((volatile uint32_t*) 0xFFFFFF00)
#define MC_RCR_RCB	(1)

/*
 * 0x04 MC Abort Status Register MC_ASR Read-only 0x0
 * Bit 25:	SVMST1	; Saved ARM7TDMI Abort Source
 * Bit 24:	SVMST0	; Saved PDC Abort Source
 * Bit 17:	MST1	; ARM7TDMI Abort Source
 * Bit 16:	MST0	; PDC Abort Source
 * Bit 11,10:	ABTTYP	; Abort Type Status
 * 		0 0 Data Read 
 * 		0 1 Data Write 
 * 		1 0 Code Fetch 
 * Bit 9,8:	ABTSZ	; Abort Size Status
 * 		0 0 Byte 
 * 		0 1 Half-word 
 * 		1 0 Word 
 * Bit 1:	MISADD	; Misaligned Address Abort Status
 * Bit 0:	UNDADD	; Undefined Address Abort Status
 */
#define MC_ASR READ_ONLY(*((volatile uint32_t*) 0xFFFFFF04))

/*
 * 0x08 MC Abort Address Status Register MC_AASR Read-only 0x0
 * (32 Bit) ABTADD: Abort Address
 */
#define MC_AASR READ_ONLY(*((volatile uint32_t*) 0xFFFFFF08))

/*
 * 0x60 MC Flash Mode Register MC_FMR Read/Write 0x0
 * Bit 23-16:	FMCN	; Flash Microsecond Cycle Number 
 * 		(number of Master Clock cycles in 1 microsecond)
 * Bit 9,8:	FWS	; Flash Wait State
 * 		0 0 = read-1 cycle write-2 cycles
 * 		0 1 = read-2 cycles write-3 cycles 
 * 		1 0 = read-3 cycles write-4 cycles 
 * 		1 1 = read-4 cycles write-4 cycles
 * Bit 7:	NEBP	; No Erase Before Programming 
 * 		0 = A page erase is performed before programming.
 * 		1 = No erase is performed before programming.
 * Bit 3:	PROGE	; Programming Error Interrupt Enable
 * Bit 2:	LOCKE	; Lock Error Interrupt Enable 
 * Bit 0:	FRDY	; Flash Ready Interrupt Enable
 */
#define MC_FMR *((volatile uint32_t*) 0xFFFFFF60)
#define MC_FMR_FMCN_MSK		(0xff0000)
#define MC_FMR_FMCN(_X_)	( ((_X_)<<16) & MC_FMR_FMCN_MSK )
#define MC_FMR_FMCN_VAL(_X_)	( ((_X_) & MC_FMR_FMCN_MSK)>>16 )
#define MC_FMR_FWS_MSK		(0x300)
#define MC_FMR_FWS(_X_)		( ((_X_)<<8) & MC_FMR_FWS_MSK )
#define MC_FMR_FWS_VAL(_X_)	( ((_X_) & MC_FMR_FWS_MSK)>>8 )
#define MC_FMR_NEBP		(1<<7)
#define MC_FMR_PROGE		(1<<3)
#define MC_FMR_LOCKE		(1<<2)
#define MC_FMR_FRDY		(1)

/*
 * 0x64 MC Flash Command Register MC_FCR Write-only
 * Bit 31-23:	KEY_A5
 * Bit 17-8:	PAGEN	; Page Number 
 * 		WP      Command	=> PAGEN defines the page number to be written. 
 * 		WPL     Command	=> PAGEN defines the page number to be written 
 * 					and its associated lock region.
 * 		EA      Command	=> This field is meaningless 
 * 		S/C LB  Command	=> PAGEN defines one page number of the 
 * 					lock region to be locked or unlocked.
 * 		S/C GPB Command	=> PAGEN defines the general-purpose bit number. 
 * 		SSB     Command	=> This field is meaningless
 * Bit 3-0:	FCMD	; Flash Command
 * 		0x01 WP   Write page 
 * 		0x02 SLB  Set Lock Bit 
 * 		0x03 WPL  Write Page and Lock 
 * 		0x04 CLB  Clear Lock Bit 
 * 		0x08 EA   Erase all 
 * 		0x0B SGPB Set General-purpose NVM Bit  
 * 		0x0D CGPB Clear General-purpose NVM Bit 
 * 		0x0F SSB  Set Security Bit 
 */
#define MC_FCR *((volatile uint32_t*) 0xFFFFFF64)
#define MC_FCR_PAGEN_MSK	(0x3ff00)
#define MC_FCR_PAGEN(_X_)	( ((_X_)<<8) & MC_FCR_PAGEN_MSK )
#define MC_FCR_PAGEN_VAL(_X_)	( ((_X_) & MC_FCR_PAGEN_MSK)>>8 )
#define MC_FCR_FCMD_WP		(0x01)
#define MC_FCR_FCMD_SLB		(0x02)
#define MC_FCR_FCMD_WPL		(0x03)
#define MC_FCR_FCMD_CLB		(0x04)
#define MC_FCR_FCMD_EA		(0x08)
#define MC_FCR_FCMD_SGPB	(0x0B)
#define MC_FCR_FCMD_CGPB	(0x0D)
#define MC_FCR_FCMD_SSB		(0x0F)


/*
 * 0x68 MC Flash Status Register MC_FSR Read-only 
 * Bit 31:	LOCKS15
 * ..
 * Bit 16:	LOCKS0
 * Bit 9:	GPNVM1
 * Bit 8:	GPNVM0
 * Bit 4:	SECURITY
 * Bit 3:	PROGE
 * Bit 2:	LOCKE
 * Bit 0:	FRDY
 */
#define MC_FSR READ_ONLY(*((volatile uint32_t*) 0xFFFFFF68))
#define MC_FSR_LOCKS15	(1<<31)
#define MC_FSR_LOCKS14	(1<<30)
#define MC_FSR_LOCKS13	(1<<29)
#define MC_FSR_LOCKS12	(1<<28)
#define MC_FSR_LOCKS11	(1<<27)
#define MC_FSR_LOCKS10	(1<<26)
#define MC_FSR_LOCKS9	(1<<25)
#define MC_FSR_LOCKS8	(1<<24)
#define MC_FSR_LOCKS7	(1<<23)
#define MC_FSR_LOCKS6	(1<<22)
#define MC_FSR_LOCKS5	(1<<21)
#define MC_FSR_LOCKS4	(1<<20)
#define MC_FSR_LOCKS3	(1<<19)
#define MC_FSR_LOCKS2	(1<<18)
#define MC_FSR_LOCKS1	(1<<17)
#define MC_FSR_LOCKS0	(1<<16)
#define MC_FSR_GPNVM1	(1<<9)
#define MC_FSR_GPNVM0	(1<<8)
#define MC_FSR_SECURITY	(1<<4)
#define MC_FSR_PROGE	(1<<3)
#define MC_FSR_LOCKE	(1<<2)
#define MC_FSR_FRDY	(1)

/**************************
 **************************
 **************************
 ***                    ***
 *** Peripheral Mapping ***
 ***                    ***
 **************************
 **************************
 **************************/

/*****************************************************
 * 0xFFFA0000 TC0, TC1, TC2 Timer/Counter 0, 1 and 2 *
 *****************************************************/
/*
 * 0xC0 TC Block Control Register TC_BCR Write-only
 * Bit 0:	SYNC	; Software trigger to resets all counters
 * 			and starts all clocks
 */
#define TC_BCR *((volatile uint32_t*) 0xFFFA00C0)
#define TC_BCR_SYNC	(1)

/*
 * 0xC4 TC Block Mode Register TC_BMR Read/Write 0
 * Bit 5,4:	TC2XC2S	; External Clock Signal 2 Selection
 * 			0 0 TCLK2 
 * 			0 1 NONE 
 * 			1 0 TIOA0 
 * 			1 1 TIOA1
 * Bit 3,2:	TC1XC1S ; External Clock Signal 1 Selection
 * 			0 0 TCLK1 
 * 			0 1 NONE 
 * 			1 0 TIOA0 
 * 			1 1 TIOA2 
 * Bit 1,0:	TC0XC0S ; External Clock Signal 0 Selection
 * 			0 0 TCLK0
 * 			0 1 NONE 
 * 			1 0 TIOA1 
 * 			1 1 TIOA2 
 */
#define TC_BMR *((volatile uint32_t*) 0xFFFA00C4)
#define TC_BMR_TC2XC2S_TCLK2	(0)
#define TC_BMR_TC2XC2S_NONE	(1<<4)
#define TC_BMR_TC2XC2S_TIOA0	(2<<4)
#define TC_BMR_TC2XC2S_TIOA1	(3<<4)
#define TC_BMR_TC2XC2S_MSK	(3<<4)
#define TC_BMR_TC1XC1S_TCLK1	(0)
#define TC_BMR_TC1XC1S_NONE	(1<<2)
#define TC_BMR_TC1XC1S_TIOA0	(2<<2)
#define TC_BMR_TC1XC1S_TIOA2	(3<<2)
#define TC_BMR_TC1XC1S_MSK	(3<<2)
#define TC_BMR_TC0XC0S_TCLK0	(0)
#define TC_BMR_TC0XC0S_NONE	(1)
#define TC_BMR_TC0XC0S_TIOA1	(2)
#define TC_BMR_TC0XC0S_TIOA2	(3)
#define TC_BMR_TC0XC0S_MSK	(3)

/*
 * 0x00/0x40/0x80 Channel 0/1/2 Control Register TC_CCR Write-only
 * Bit 2:	SWTRG	;Software Trigger Command (reset counter and start clock)
 * Bit 1:	CLKDIS	;Counter Clock Disable Command
 * Bit 0:	CLKEN	;Counter Clock Enable Command (only if CLKDIS = 0)
 */
#define TC_CCR_SWTRG	4
#define TC_CCR_CLKDIS	2
#define TC_CCR_CLKEN	1
#define TC0_CCR *((volatile uint32_t*) 0xFFFA0000)
#define TC1_CCR *((volatile uint32_t*) 0xFFFA0040)
#define TC2_CCR *((volatile uint32_t*) 0xFFFA0080)
		
/*
 * 0x04/0x44/0x84 Channel Mode Register TC_CMR Read/Write 0 (Capture Mode)
 * Bit 19,18:	LDRB	; RB Loading Selection
 * 			0 0 none 
 * 			0 1 rising edge of TIOA 
 * 			1 0 falling edge of TIOA 
 * 			1 1 each edge of TIOA 
 * Bit 17,16:	LDRA	; RA Loading Selection
 * 			0 0 none 
 * 			0 1 rising edge of TIOA 
 * 			1 0 falling edge of TIOA 
 * 			1 1 each edge of TIOA 
 * Bit 15:	WAVE=0	; Capture Mode
 * 			0 = Capture Mode is enabled. 
 * 			1 = Waveform Mode is enabled.(see below)
 * Bit 14:	CPCTRG	; RC Compare Trigger resets the counter and starts the counter clock
 * Bit 10:	ABETRG	; External Trigger Selection 
 * 			0 = TIOB is used as an external trigger. 
 * 			1 = TIOA is used as an external trigger.
 * Bit 9,8:	ETRGEDG	; External Trigger Edge Selection
 * 			0 0 none 
 * 			0 1 rising edge 
 * 			1 0 falling edge 
 * 			1 1 each edge 
 * Bit 7:	LDBDIS	; Counter clock disabled when RB loading occurs
 * Bit 6:	LDBSTOP ; Counter clock stopped when RB loading occurs
 * Bit 5,4:	BURST	; Burst Signal Selection 
 * 			0 0 The clock is not gated by an external signal. 
 * 			0 1 XC0 is ANDed with the selected clock. 
 * 			1 0 XC1 is ANDed with the selected clock. 
 * 			1 1 XC2 is ANDed with the selected clock.
 * Bit 3:	CLKI	; Clock Invert 
 * 			0 = incr. counter on rising edge of clock
 * 			1 = incr. counter on falling edge of clock
 * Bit 2,1,0:	TCCLKS	; Clock Selection 
 * 			0 0 0 TIMER_CLOCK1 
 * 			0 0 1 TIMER_CLOCK2 
 * 			0 1 0 TIMER_CLOCK3 
 * 			0 1 1 TIMER_CLOCK4 
 * 			1 0 0 TIMER_CLOCK5 
 * 			1 0 1 XC0 
 * 			1 1 0 XC1 
 * 			1 1 1 XC2 
 */

#define TC_CMR_LDRB_NONE	(0)
#define TC_CMR_LDRB_RISING	(1<<18)
#define TC_CMR_LDRB_FALLING	(2<<18)
#define TC_CMR_LDRB_BOTH	(3<<18)
#define TC_CMR_LDRA_NONE	(0)
#define TC_CMR_LDRA_RISING	(1<<16)
#define TC_CMR_LDRA_FALLING	(2<<16)
#define TC_CMR_LDRA_BOTH	(3<<16)
#define TC_CMR_CAPTURE_MODE	(0)
#define TC_CMR_WAVE_MODE	(1<<15)
#define TC_CMR_CPCTRG		(1<<14)
#define TC_CMR_ABETRG_TIOB	(0)
#define TC_CMR_ABETRG_TIOA	(1<<10)
#define TC_CMR_ETRGEDG_NONE	(0)
#define TC_CMR_ETRGEDG_RISING	(1<<8)
#define TC_CMR_ETRGEDG_FALLING	(2<<8)
#define TC_CMR_ETRGEDG_BOTH	(3<<8)
#define TC_CMR_LDBDIS		(1<<7)
#define TC_CMR_LDBSTOP		(1<<6)
#define TC_CMR_BURST_NOT_GATED	(0)
#define TC_CMR_BURST_XC0_TCCLK	(1<<4)
#define TC_CMR_BURST_XC1_TCCLK	(2<<4)
#define TC_CMR_BURST_XC2_TCCLK	(3<<4)
#define TC_CMR_CLKI		(1<<3)
#define TC_CMR_TCCLKS_TIMER_CLOCK1_MCK_2 (0)
#define TC_CMR_TCCLKS_TIMER_CLOCK2_MCK_8 (1)
#define TC_CMR_TCCLKS_TIMER_CLOCK3_MCK_32 (2)
#define TC_CMR_TCCLKS_TIMER_CLOCK4_MCK_128 (3)
#define TC_CMR_TCCLKS_TIMER_CLOCK5_MCK_1024 (4)
#define TC_CMR_TCCLKS_XC0	(5)
#define TC_CMR_TCCLKS_XC1	(6)
#define TC_CMR_TCCLKS_XC2	(7)

#define TC0_CMR *((volatile uint32_t*) 0xFFFA0004)
#define TC1_CMR *((volatile uint32_t*) 0xFFFA0044)
#define TC2_CMR *((volatile uint32_t*) 0xFFFA0084)

/*
 * 0x04/0x44/0x84 Channel Mode Register TC_CMR Read/Write 0 (Waveform Mode)
 * Bit 31,30:	BSWTRG	; Software Trigger Effect on TIOB 
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 29,28:	BEEVT	; External Event Effect on TIOB 
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 27,26:	BCPC	; RC Compare Effect on TIOB 
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 25,24:	BCPB	; RB Compare Effect on TIOB 
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 23,22:	ASWTRG	; Software Trigger Effect on TIOA
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 21,20:	AEEVT	; External Event Effect on TIOA
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 19,18:	ACPC	; RC Compare Effect on TIOA 
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 17,16:	ACPA	; RA Compare Effect on TIOA 
 * 			0 0 none 
 * 			0 1 set 
 * 			1 0 clear 
 * 			1 1 toggle
 * Bit 15:	WAVE=1	;
 * 			0 = Capture Mode is enabled (see above). 
 * 			1 = Waveform Mode is enabled.
 * Bit 14,13:	WAVSEL	; Waveform Selection 
 * 			0 0 UP mode
 * 			1 0 UP mode with automatic trigger on RC Compare 
 * 			0 1 UPDOWN mode
 * 			1 1 UPDOWN mode with automatic trigger on RC Compare 
 * Bit 12:	ENETRG	; External Event Trigger Enable
 * 			0 = The external event has no effect on the counter
 * 			    and its clock. In this case, the selected 
 * 			    external event only controls the TIOA output. 
 * 			1 = The external event resets the counter 
 * 			    and starts the counter clock.
 * Bit 11,10:	EEVT	; External Event Selection 
 * 			0 0 TIOB input  (If it is chosen as the external 
 * 			    event signal, so no longer generates waveforms.) 
 * 			0 1 XC0 output 
 * 			1 0 XC1 output 
 * 			1 1 XC2 output 
 * Bit 9,8:	EEVTEDG	; External Event Edge Selection 
 * 			0 0 none 
 * 			0 1 rising edge 
 * 			1 0 falling edge 
 * 			1 1 each edge 
 * Bit 7:	CPCDIS	; Counter Clock Disable with RC Compare 
 * 			1 = clock is disabled when counter reaches RC
 * 			0 = clock not disabled 
 * Bit 6:	CPCSTOP	; Counter Clock Stopped with RC Compare 
 * 			1 = clock stopped when counter reaches RC 
 * 			0 = clock not stopped
 * Bit 5,4:	BURST	; (same as Capture Mode)
 * Bit 3:	CLKI	; (same as Capture Mode)
 * Bit 2,1,0:	TCCLKS	; (same as Capture Mode)
 */
#define TC_CMR_BSWTRG_NONE	(0)
#define TC_CMR_BSWTRG_SET	(1<<30)
#define TC_CMR_BSWTRG_CLEAR	(2<<30)
#define TC_CMR_BSWTRG_TOGGLE	(3<<30)
#define TC_CMR_BEEVT_NONE	(0)
#define TC_CMR_BEEVT_SET	(1<<28)
#define TC_CMR_BEEVT_CLEAR	(2<<28)
#define TC_CMR_BEEVT_TOGGLE	(3<<28)
#define TC_CMR_BCPC_NONE	(0)
#define TC_CMR_BCPC_SET		(1<<26)
#define TC_CMR_BCPC_CLEAR	(2<<26)
#define TC_CMR_BCPC_TOGGLE	(3<<26)
#define TC_CMR_BCPB_NONE	(0)
#define TC_CMR_BCPB_SET		(1<<24)
#define TC_CMR_BCPB_CLEAR	(2<<24)
#define TC_CMR_BCPB_TOGGLE	(3<<24)
#define TC_CMR_ASWTRG_NONE	(0)
#define TC_CMR_ASWTRG_SET	(1<<22)
#define TC_CMR_ASWTRG_CLEAR	(2<<22)
#define TC_CMR_ASWTRG_TOGGLE	(3<<22)
#define TC_CMR_AEEVT_NONE	(0)
#define TC_CMR_AEEVT_SET	(1<<20)
#define TC_CMR_AEEVT_CLEAR	(2<<20)
#define TC_CMR_AEEVT_TOGGLE	(3<<20)
#define TC_CMR_ACPC_NONE	(0)
#define TC_CMR_ACPC_SET		(1<<18)
#define TC_CMR_ACPC_CLEAR	(2<<18)
#define TC_CMR_ACPC_TOGGLE	(3<<18)
#define TC_CMR_ACPA_NONE	(0)
#define TC_CMR_ACPA_SET		(1<<16)
#define TC_CMR_ACPA_CLEAR	(2<<16)
#define TC_CMR_ACPA_TOGGLE	(3<<16)
#define TC_CMR_WAVSEL_UP	(0)
#define TC_CMR_WAVSEL_UPDOWN	(1<<13)
#define TC_CMR_WAVSEL_UP_AUTO	(2<<13)
#define TC_CMR_WAVSEL_UPDOWN_AUTO (3<<13)
#define TC_CMR_ENETRG		(1<<12)
#define TC_CMR_EEVT_TIOB	(0)
#define TC_CMR_EEVT_XC0		(1<<10)
#define TC_CMR_EEVT_XC1		(2<<10)
#define TC_CMR_EEVT_XC2		(3<<10)
#define TC_CMR_EEVTEDG_NONE	(0)
#define TC_CMR_EEVTEDG_RISING	(1<<8)
#define TC_CMR_EEVTEDG_FALLING	(2<<8)
#define TC_CMR_EEVTEDG_BOTH	(3<<8)
#define TC_CMR_CPCDIS_	(1<<7)
#define TC_CMR_CPCSTOP_	(1<<6)

/*
 * 0x10/0x50/0x90 Counter Value TC_CV Read-only 0
 * Bit 15-0:	CV; Counter Value 
 */
#define TC0_CV READ_ONLY(*((volatile uint32_t*) 0xFFFA0010))
#define TC1_CV READ_ONLY(*((volatile uint32_t*) 0xFFFA0050))
#define TC2_CV READ_ONLY(*((volatile uint32_t*) 0xFFFA0090))
 
/*
 * 0x14/0x54/0x94 Register A TC_RA Read/Write 0
 * Bit 15-0:	RA	; Register A value
 * 	Read-only if WAVE = 0 (Capture mode)
 * 	Read/Write if WAVE = 1 (Waveform mode)
 */
#define TC0_RA *((volatile uint32_t*) 0xFFFA0014)
#define TC1_RA *((volatile uint32_t*) 0xFFFA0054)
#define TC2_RA *((volatile uint32_t*) 0xFFFA0094)

/*
 * 0x18/0x58/0x98 Register B TC_RB Read/Write 0
 * Bit 15-0:	RB	; Register B value
 * 	Read-only if WAVE = 0 (Capture mode)
 * 	Read/Write if WAVE = 1 (Waveform mode)
 */
#define TC0_RB *((volatile uint32_t*) 0xFFFA0018)
#define TC1_RB *((volatile uint32_t*) 0xFFFA0058)
#define TC2_RB *((volatile uint32_t*) 0xFFFA0098)

/*
 * 0x1C/0x5C/0x9C Register C TC_RC Read/Write 0
 * Bit 15-0:	RC	; Register C value
 */
#define TC0_RC *((volatile uint32_t*) 0xFFFA001C)
#define TC1_RC *((volatile uint32_t*) 0xFFFA005C)
#define TC2_RC *((volatile uint32_t*) 0xFFFA009C)

/*
 * 0x20/0x60/0xA0 Status Register TC_SR Read-only 0
 * Bit 18:	MTIOB	; TIOB (0 = TIOB is low;1 = TIOB is high)
 * Bit 17:	MTIOA	; TIOA (0 = TIOA is low;1 = TIOA is high)
 * Bit 16:	CLKSTA	; Clock Status (0 = disabled; 1 = enabled)
 * Bit 7:	ETRGS	; External trigger occurred (since last read of TC_SR)
 * Bit 6:	LDRBS	; RB Load occurred (since last read of TC_SR)
 * Bit 5:	LDRAS	; RA Load occurred (since last read of TC_SR)
 * Bit 4:	CPCS	; RC Compare occurred (since last read of TC_SR)
 * Bit 3:	CPBS	; RB Compare occurred (since last read of TC_SR)
 * Bit 2:	CPAS	; RA Compare occurred (since last read of TC_SR)
 * Bit 1:	LOVRS	; Load Overrun (RA or RB have been loaded twice)
 * Bit 0:	COVFS	; Counter Overflow (since last read of TC_SR)
 */
#define TC_SR_MTIOB	(1<<18)
#define TC_SR_MTIOA	(1<<17)
#define TC_SR_CLKSTA	(1<<16)
#define TC_SR_ETRGS	(1<<7)
#define TC_SR_LDRBS	(1<<6)
#define TC_SR_LDRAS	(1<<5)
#define TC_SR_CPCS	(1<<4)
#define TC_SR_CPBS	(1<<3)
#define TC_SR_CPAS	(1<<2)
#define TC_SR_LOVRS	(1<<1)
#define TC_SR_COVFS	(1)
 
#define TC0_SR READ_ONLY(*((volatile uint32_t*) 0xFFFA0020))
#define TC1_SR READ_ONLY(*((volatile uint32_t*) 0xFFFA0060))
#define TC2_SR READ_ONLY(*((volatile uint32_t*) 0xFFFA00A0))

/*
 * 0x24/0x64/0xA4 Interrupt Enable Register TC_IER Write-only
 * Bit 7:	ETRGS	; Enable External Trigger Interrupt
 * Bit 6:	LDRBS	; Enable RB Load Interrupt
 * Bit 5:	LDRAS	; Enable RA Load Interrupt
 * Bit 4:	CPCS	; Enable RC Compare Interrupt
 * Bit 3:	CPBS	; Enable RB Compare Interrupt
 * Bit 2:	CPAS	; Enable RA Compare Interrupt
 * Bit 1:	LOVRS	; Enable Load Overrun Interrupt
 * Bit 0:	COVFS	; Enable Counter Overflow Interrupt
 */
#define TC_IER_ETRGS	(1<<7)
#define TC_IER_LDRBS	(1<<6)
#define TC_IER_LDRAS	(1<<5)
#define TC_IER_CPCS	(1<<4)
#define TC_IER_CPBS	(1<<3)
#define TC_IER_CPAS	(1<<2)
#define TC_IER_LOVRS	(1<<1)
#define TC_IER_COVFS	(1)

#define TC0_IER *((volatile uint32_t*) 0xFFFA0024)
#define TC1_IER *((volatile uint32_t*) 0xFFFA0064)
#define TC2_IER *((volatile uint32_t*) 0xFFFA00A4)

/*
 * 0x28/0x68/0xA8 Interrupt Disable Register TC_IDR Write-only
 * Bit 7:	ETRGS	; Disable External Trigger Interrupt
 * Bit 6:	LDRBS	; Disable RB Load Interrupt
 * Bit 5:	LDRAS	; Disable RA Load Interrupt
 * Bit 4:	CPCS	; Disable RC Compare Interrupt
 * Bit 3:	CPBS	; Disable RB Compare Interrupt
 * Bit 2:	CPAS	; Disable RA Compare Interrupt
 * Bit 1:	LOVRS	; Disable Load Overrun Interrupt
 * Bit 0:	COVFS	; Disable Counter Overflow Interrupt
 */
#define TC_IDR_ETRGS	(1<<7)
#define TC_IDR_LDRBS	(1<<6)
#define TC_IDR_LDRAS	(1<<5)
#define TC_IDR_CPCS	(1<<4)
#define TC_IDR_CPBS	(1<<3)
#define TC_IDR_CPAS	(1<<2)
#define TC_IDR_LOVRS	(1<<1)
#define TC_IDR_COVFS	(1)
#define TC_IDR_ALL	( TC_IDR_ETRGS | TC_IDR_LDRBS | TC_IDR_LDRAS | TC_IDR_CPCS \
			| TC_IDR_CPBS | TC_IDR_CPAS | TC_IDR_LOVRS | TC_IDR_COVFS)

#define TC0_IDR *((volatile uint32_t*) 0xFFFA0028)
#define TC1_IDR *((volatile uint32_t*) 0xFFFA0068)
#define TC2_IDR *((volatile uint32_t*) 0xFFFA00A8)

/*
 * 0x2C/0x6C/0xAC Interrupt Mask Register TC_IMR Read-only 0
 * Bit 7:	ETRGS	; External Trigger Interrupt Mask
 * Bit 6:	LDRBS	; RB Load Interrupt Mask
 * Bit 5:	LDRAS	; RA Load Interrupt Mask
 * Bit 4:	CPCS	; RC Compare Interrupt Mask
 * Bit 3:	CPBS	; RB Compare Interrupt Mask
 * Bit 2:	CPAS	; RA Compare Interrupt Mask
 * Bit 1:	LOVRS	; Load Overrun Interrupt Mask
 * Bit 0:	COVFS	; Counter Overflow Interrupt Mask
 * 		(0 = interrupt disabled; 1 = interrupt enabled)
 */
#define TC_IMR_ETRGS	(1<<7)
#define TC_IMR_LDRBS	(1<<6)
#define TC_IMR_LDRAS	(1<<5)
#define TC_IMR_CPCS	(1<<4)
#define TC_IMR_CPBS	(1<<3)
#define TC_IMR_CPAS	(1<<2)
#define TC_IMR_LOVRS	(1<<1)
#define TC_IMR_COVFS	(1)

#define TC0_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFA002C))
#define TC1_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFA006C))
#define TC2_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFA00AC))


/**********************************
 * 0xFFFB0000 UDP USB Device Port *
 **********************************/
/*************************************
 * 0xFFFB8000 TWI Two-Wire Interface *
 *************************************/
/********************************************************************************
 * 0xFFFC0000 USART0 Universal Synchronous Asynchronous Receiver Transmitter 0  *
 ********************************************************************************/
/*******************************************************************************
 * 0xFFFC4000 USART1 Universal Synchronous Asynchronous Receiver Transmitter 1 *
 *******************************************************************************/

/**********************************
 * 0xFFFCC000 PWMC PWM Controller *
 **********************************/
/*
 * 0x00 PWM_MR		PWM Mode Register (Read/Write)       0
 * Bit 27-24:	PREB	; Divider Input Clock
 * 				0000 = MCK
 * 				0001 = MCK/2
 * 				0010 = MCK/4
 * 				0011 = MCK/8
 * 				0100 = MCK/16
 * 				0101 = MCK/32
 * 				0110 = MCK/64
 * 				0111 = MCK/128
 * 				1000 = MCK/256
 * 				1001 = MCK/512
 * 				1010 = MCK/1024
 * Bit 23-16:	DIVB	; CLKB Divide Factor
 * 				0: CLKB = clock turned off
 * 				1: CLKB = clock selected by PREB
 * 				2-255: CLKB = PREB divided by DIVB factor.
 * Bit 11-8:	PREA	; Divider Input Clock
 * 				0000 = MCK
 * 				0001 = MCK/2
 * 				0010 = MCK/4
 * 				0011 = MCK/8
 * 				0100 = MCK/16
 * 				0101 = MCK/32
 * 				0110 = MCK/64
 * 				0111 = MCK/128
 * 				1000 = MCK/256
 * 				1001 = MCK/512
 * 				1010 = MCK/1024
 * Bit 7-0:	DIVA	; CLKA Divide Factor
 * 				0: CLKA = clock turned off
 * 				1: CLKA = clock selected by PREA
 * 				2-255: CLKA = PREA divided by DIVA factor.
 */
#define PWM_MR_PREB_MCK		(0)
#define PWM_MR_PREB_MCK_2	(1<<24)
#define PWM_MR_PREB_MCK_4	(2<<24)
#define PWM_MR_PREB_MCK_8	(3<<24)
#define PWM_MR_PREB_MCK_16	(4<<24)
#define PWM_MR_PREB_MCK_32	(5<<24)
#define PWM_MR_PREB_MCK_64	(6<<24)
#define PWM_MR_PREB_MCK_128	(7<<24)
#define PWM_MR_PREB_MCK_256	(8<<24)
#define PWM_MR_PREB_MCK_512	(9<<24)
#define PWM_MR_PREB_MCK_1024	(10<<24)
#define PWM_MR_DIVB(_X_)	(((_X_)&0xFF)<<16)
#define PWM_MR_PREA_MCK		(0)
#define PWM_MR_PREA_MCK_2	(1<<8)
#define PWM_MR_PREA_MCK_4	(2<<8)
#define PWM_MR_PREA_MCK_8	(3<<8)
#define PWM_MR_PREA_MCK_16	(4<<8)
#define PWM_MR_PREA_MCK_32	(5<<8)
#define PWM_MR_PREA_MCK_64	(6<<8)
#define PWM_MR_PREA_MCK_128	(7<<8)
#define PWM_MR_PREA_MCK_256	(8<<8)
#define PWM_MR_PREA_MCK_512	(9<<8)
#define PWM_MR_PREA_MCK_1024	(10<<8)
#define PWM_MR_DIVA(_X_)	((_X_)&0xFF)

#define PWM_MR *((volatile uint32_t*) 0xFFFCC000)

/*
 * 0x04 PWM_ENA		PWM Enable Register (Write-only)     -
 * 0x08 PWM_DIS		PWM Disable Register (Write-only)    -
 * Bit 3-0:	CHIDx	; Channel ID x ( with x = 3,2,1 or 0) 
 * 			  If bit x set enables (or disables) output on Channel x.
 */
#define PWM_CHID3	(1<<3)
#define PWM_CHID2	(1<<2)
#define PWM_CHID1	(1<<1)
#define PWM_CHID0	(1)
 
#define PWM_ENA *((volatile uint32_t*) 0xFFFCC004)
#define PWM_DIS *((volatile uint32_t*) 0xFFFCC008)
 
/*
 * 0x0C PWM_SR		PWM Status Register (Read-only)      0
 * Bit 3-0:	CHIDx	; Channel ID x ( with x = 3,2,1 or 0) 
 * 			  1 = output for Channel enabled
 * 			  0 = output for Channel disabled
 */
#define PWM_SR *((volatile uint32_t*) 0xFFFCC00C)
 
/*
 * 0x10 PWM_IER		PWM Interrupt Enable Register (Write-only)     -
 * 0x14 PWM_IDR		PWM Interrupt Disable Register (Write-only)    -
 * Bit 3-0:	CHIDx	; Channel ID x ( with x = 3,2,1 or 0) 
 * 			  If bit x set enables (or disables) interrupt for PWM at Channel x.
 */
#define PWM_IER *((volatile uint32_t*) 0xFFFCC010)
#define PWM_IDR *((volatile uint32_t*) 0xFFFCC014)
 
/*
 * 0x18 PWM_IMR		PWM Interrupt Mask Register (Read-only)        0
 * Bit 3-0:	CHIDx	; Channel ID x ( with x = 3,2,1 or 0) 
 * 			  1 = interrupt for Channel enabled
 * 			  0 = interrupt for Channel disabled
 */
#define PWM_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFCC018))
 
/*
 * 0x1C PWM_ISR		PWM Interrupt Status Register (Read-only)      0
 * Bit 3-0:	CHIDx	; Channel ID x ( with x = 3,2,1 or 0) 
 * 			  1 = interrupt for Channel pending
 * 			  0 = no interrupt
 */
#define PWM_ISR READ_ONLY(*((volatile uint32_t*) 0xFFFCC01C))

 
/*
 * 0x200 PWM_CMR0	Channel 0 Mode Register  (Read/Write)  0x0
 * 0x220 PWM_CMR1	Channel 1 Mode Register  (Read/Write)  0x0
 * 0x240 PWM_CMR2	Channel 2 Mode Register  (Read/Write)  0x0
 * 0x260 PWM_CMR3	Channel 3 Mode Register  (Read/Write)  0x0
 * Bit 10:	CPD	; Channel Update Period
 * 			 1 = writing to PWM_CUPDx modify dury cycle at next start event
 * 			 0 = writing to PWM_CUPDx modify period at next start event
 * Bit 9:	CPOL	; Channel Polarity
 * 			 1 = output waveform starts at high level
 * 			 0 = output waveform starts at low level
 * Bit 8:	CALG	; Channel Alignment
 * 			 1 = period center aligned
 * 			 0 = period left aligned
 * Bit 3-0:	CPRE	; Channel Pre scalar
 * 				0000 = MCK
 * 				0001 = MCK/2
 * 				0010 = MCK/4
 * 				0011 = MCK/8
 * 				0100 = MCK/16
 * 				0101 = MCK/32
 * 				0110 = MCK/64
 * 				0111 = MCK/128
 * 				1000 = MCK/256
 * 				1001 = MCK/512
 * 				1010 = MCK/1024
 * 				1011 = CLKA
 * 				1100 = CLKB
 */
#define PWM_CMR_CPD		(1<<10)
#define PWM_CMR_CPOL		(1<<9)
#define PWM_CMR_CALG		(1<<8)
#define PWM_CMR_CPRE_MCK	(0)
#define PWM_CMR_CPRE_MCK_2	(1)
#define PWM_CMR_CPRE_MCK_4	(2)
#define PWM_CMR_CPRE_MCK_8	(3)
#define PWM_CMR_CPRE_MCK_16	(4)
#define PWM_CMR_CPRE_MCK_32	(5)
#define PWM_CMR_CPRE_MCK_64	(6)
#define PWM_CMR_CPRE_MCK_128	(7)
#define PWM_CMR_CPRE_MCK_256	(8)
#define PWM_CMR_CPRE_MCK_512	(9)
#define PWM_CMR_CPRE_MCK_1024	(10)
#define PWM_CMR_CPRE_CLKA	(11)
#define PWM_CMR_CPRE_CLKB	(12)

#define PWM_CMR0 *((volatile uint32_t*) 0xFFFCC200)
#define PWM_CMR1 *((volatile uint32_t*) 0xFFFCC220)
#define PWM_CMR2 *((volatile uint32_t*) 0xFFFCC240)
#define PWM_CMR3 *((volatile uint32_t*) 0xFFFCC260)

/*
 * 0x204 PWM_CDTY0	Channel 0 Duty Cycle Register (Read/Write) 0x0
 * 0x224 PWM_CDTY1	Channel 1 Duty Cycle Register (Read/Write) 0x0
 * 0x244 PWM_CDTY2	Channel 2 Duty Cycle Register (Read/Write) 0x0
 * 0x264 PWM_CDTY3	Channel 3 Duty Cycle Register (Read/Write) 0x0
 * Bit 15-0:	CDTY	; Channel Duty Cycle (counter)
 */
#define PWM_CDTY0 *((volatile uint32_t*) 0xFFFCC204)
#define PWM_CDTY1 *((volatile uint32_t*) 0xFFFCC224)
#define PWM_CDTY2 *((volatile uint32_t*) 0xFFFCC244)
#define PWM_CDTY3 *((volatile uint32_t*) 0xFFFCC264)
 
/*
 * 0x208 PWM_CPRD0	Channel 0 Period Register  (Read/Write) 0x0
 * 0x228 PWM_CPRD1	Channel 1 Period Register  (Read/Write) 0x0
 * 0x248 PWM_CPRD2	Channel 2 Period Register  (Read/Write) 0x0
 * 0x268 PWM_CPRD3	Channel 3 Period Register  (Read/Write) 0x0
 * Bit 15-0:	CPRD	; Channel Period (counter)
 */
#define PWM_CPRD0 *((volatile uint32_t*) 0xFFFCC208)
#define PWM_CPRD1 *((volatile uint32_t*) 0xFFFCC228)
#define PWM_CPRD2 *((volatile uint32_t*) 0xFFFCC248)
#define PWM_CPRD3 *((volatile uint32_t*) 0xFFFCC268)

/*
 * 0x20C PWM_CCNT0	Channel 0 Counter Register  (Read-only)  0x0
 * 0x22C PWM_CCNT1	Channel 1 Counter Register  (Read-only)  0x0
 * 0x24C PWM_CCNT2	Channel 2 Counter Register  (Read-only)  0x0
 * 0x26C PWM_CCNT3	Channel 3 Counter Register  (Read-only)  0x0
 */
#define PWM_CCNT0 READ_ONLY(*((volatile uint32_t*) 0xFFFCC20C))
#define PWM_CCNT1 READ_ONLY(*((volatile uint32_t*) 0xFFFCC22C))
#define PWM_CCNT2 READ_ONLY(*((volatile uint32_t*) 0xFFFCC24C))
#define PWM_CCNT3 READ_ONLY(*((volatile uint32_t*) 0xFFFCC26C))

/*
 * 0x210 PWM_CUPD0	Channel 0 Update Register   (Write-only)   -
 * 0x230 PWM_CUPD1	Channel 1 Update Register   (Write-only)   -
 * 0x250 PWM_CUPD2	Channel 2 Update Register   (Write-only)   -
 * 0x270 PWM_CUPD3	Channel 3 Update Register   (Write-only)   -
 */
#define PWM_CUPD0 *((volatile uint32_t*) 0xFFFCC210)
#define PWM_CUPD1 *((volatile uint32_t*) 0xFFFCC230)
#define PWM_CUPD2 *((volatile uint32_t*) 0xFFFCC250)
#define PWM_CUPD3 *((volatile uint32_t*) 0xFFFCC270)



/************************************************
 * 0xFFFD4000 SSC Serial Synchronous Controller *
 ************************************************/


 
/**********************************************
 * 0xFFFD8000 ADC Analog-to-Digital Converter *
 **********************************************/
/*
 * 0x00 ADC_CR	ADC Control Register (Write-only)
 * Bit 1:	START	; Start Conversion
 * Bit 0:	SWRST	; Reset ADC
 */
#define ADC_CR_START	(1<<1)
#define ADC_CR_SWRST	(1)
#define ADC_CR *((volatile uint32_t*) 0xFFFD8000)
 
/*
 * 0x04 ADC_MR	ADC Mode Register (Read/Write)  0x00000000
 * Bit 27-24:	SHTIM	; Sample and Hold Time = (SHTIM+1) / ADCClock
 * Bit 20-16:	STARTUP	; Startup Time = (STARTUP+1) * 8 / ADCClock
 * Bit 13-8:	PRESCAL	; Clock Prescaler => ADCClock = MCK / ( (PRESCAL+1) * 2 )
 * Bit 5:	SLEEP	; Sleep Mode (1 = Sleep Mode; 0 = Normal Mode)
 * Bit 4:	LOWRES	; Selected Resolution
 * 			  1 = 8-bit resolution
 * 			  0 = 10-bit resolution
 * Bit 3-1:	TRGSEL	; Hardware Trigger Selection
 *			  000	TIOA Ouput of Timer Counter Channel 0
 *			  001	TIOA Ouput of Timer Counter Channel 1
 *			  010	TIOA Ouput of Timer Counter Channel 2
 *			  110	External trigger
 * Bit 0:	TRGEN	; Hardware Trigger Enable (selected by TRGSEL field)
 */
#define ADC_MR_SHTIM(_X_)		(((_X_)&0xF)<<24)
#define ADC_MR_STARTUP(_X_)		(((_X_)&0x1F)<<16)
#define ADC_MR_PRESCAL(_X_)		(((_X_)&0x3F)<<8)
#define ADC_MR_SLEEP			(1<<5)
#define ADC_MR_LOWRES			(1<<4)
#define ADC_MR_TRGSEL_TIOA_TC_CHAN0	(0)
#define ADC_MR_TRGSEL_TIOA_TC_CHAN1	(1<<1)
#define ADC_MR_TRGSEL_TIOA_TC_CHAN2	(2<<1)
#define ADC_MR_TRGSEL_EXT		(3<<2)
#define ADC_MR_TRGSEL_MSK		(7<<1)
#define ADC_MR_TRGEN			(1)
#define ADC_MR *((volatile uint32_t*) 0xFFFD8004)


/* 
 * 0x10 ADC_CHER Channel Enable Register (Write-only)
 * Bit 7-0:	CHx	; Channel x ( with x = 7-0) 
 * 			  If bit x set enables Channel x.
 */
#define ADC_CH(_X_)	(1<<(_X_))
#define ADC_CHER *((volatile uint32_t*) 0xFFFD8010)

/*
 * 0x14 ADC_CHDR Channel Disable Register (Write-only)
 * Bit 7-0:	CHx	; Channel x ( with x = 7-0) 
 * 			  If bit x set disables Channel x.
 */
#define ADC_CHDR *((volatile uint32_t*) 0xFFFD8014)

/*
 * 0x18 ADC_CHSR Channel Status Register (Read-only) 0x00000000
 * Bit 7-0:	CHx	; Channel x Statut ( with x = 7-0)
 * 			  1 = Channel x enabled
 * 			  0 = Channel x disabled
 */
#define ADC_CHSR READ_ONLY(*((volatile uint32_t*) 0xFFFD8018))
 
/* 
 * 0x1C ADC_SR ADC Status Register (Read-only) 0x000C0000
 * Bit 19:	RXBUFF	; RX Buffer Full (Both ADC_RCR and ADC_RNCR have a value of 0)
 * Bit 18:	ENDRX	; End of RX Buffer (Receive Counter Register has reached 0)
 * Bit 17:	GOVRE	; General Overrun Error
 * Bit 16:	DRDY	; Data Ready (data has been converted and is available in ADC_LCDR)
 * Bit 15-8:	OVREx	; Overrun Error Channel x since the last read of ADC_SR ( with x = 7-0)
 * Bit 7-0:	EOCx	; End of Conversion Channel x ( with x = 7-0)
 */
#define ADC_RXBUFF	(1<<19)
#define ADC_ENDRX	(1<<18)
#define ADC_GOVRE	(1<<17)
#define ADC_DRDY	(1<<16)
#define ADC_OVRE(_X_)	(0x100<<(_X_))
#define ADC_EOC(_X_)	(1<<(_X_))
#define ADC_SR READ_ONLY(*((volatile uint32_t*) 0xFFFD801C))

/*
 * 0x20 ADC_LCDR Last Converted Data Register (Read-only) 0x00000000
 * Bit 9-0:	LCDR	; Last Converted Data
 */
#define ADC_LCDR READ_ONLY(*((volatile uint32_t*) 0xFFFD8020))

/*
 * 0x24 ADC_IER Interrupt Enable Register (Write-only)
 * 0x28 ADC_IDR Interrupt Disable Register (Write-only)
 * 0x2C ADC_IMR Interrupt Mask Register (Read-only) 0x00000000
 * Bit 19:	RXBUFF	; RX Buffer Full (Both ADC_RCR and ADC_RNCR have a value of 0)
 * Bit 18:	ENDRX	; End of RX Buffer (Receive Counter Register has reached 0)
 * Bit 17:	GOVRE	; General Overrun Error
 * Bit 16:	DRDY	; Data Ready (data has been converted and is available in ADC_LCDR)
 * Bit 15-8:	OVREx	; Overrun Error Channel x since the last read of ADC_SR ( with x = 7-0)
 * Bit 7-0:	EOCx	; End of Conversion Channel x ( with x = 7-0)
 */
#define ADC_IER *((volatile uint32_t*) 0xFFFD8024)
#define ADC_IDR *((volatile uint32_t*) 0xFFFD8028)
#define ADC_IMR READ_ONLY(*((volatile uint32_t*) 0xFFFD802C))

/*
 * 0x30 - 0x4C ADC_CDRx Channel Data Register x (Read-only) 0x00000000
 * Bit 9-0:	Data	;	Converted Data
 */
#define ADC_CDR0 READ_ONLY(*((volatile uint32_t*) 0xFFFD8030))
#define ADC_CDR1 READ_ONLY(*((volatile uint32_t*) 0xFFFD8034))
#define ADC_CDR2 READ_ONLY(*((volatile uint32_t*) 0xFFFD8038))
#define ADC_CDR3 READ_ONLY(*((volatile uint32_t*) 0xFFFD803C))
#define ADC_CDR4 READ_ONLY(*((volatile uint32_t*) 0xFFFD8040))
#define ADC_CDR5 READ_ONLY(*((volatile uint32_t*) 0xFFFD8044))
#define ADC_CDR6 READ_ONLY(*((volatile uint32_t*) 0xFFFD8048))
#define ADC_CDR7 READ_ONLY(*((volatile uint32_t*) 0xFFFD804C))

 
/**********************************************
 * 0xFFFE0000 SPI Serial Peripheral Interface *
 **********************************************/



