/*
 * s3c44b0x_sysinit.c
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
 * -- ARM mode
 */
#define skip do { \
	} while (0)

#include <inttypes.h>
#include "s3c44b0x.h"

extern void Timer5_ISR(void) __attribute__ ((long_call));

void sys_preinit_hook(void)	__attribute__ ((section (".textinit2")));
void sys_init_hook(void)	__attribute__ ((section (".textinit2")));
void StopTimer5(void)		__attribute__ ((section (".textinit2")));
void StartTimer5(int cnt)	__attribute__ ((section (".textinit2")));

int16_t  Timer01prescale	__attribute__ ((section (".datainit2"))) = 187; /*Base 3,1탎ec (321kHz) at 60 MHz*/
int16_t  Timer23prescale	__attribute__ ((section (".datainit2"))) = 150; /*Base 2,5탎ec (400kHz) at 60 MHz*/
int16_t  Timer45prescale	__attribute__ ((section (".datainit2"))) = 150; /*Base 2,5탎ec (400kHz) at 60 MHz*/

int16_t  Timer0_premux		__attribute__ ((section (".datainit2"))) = 0; /*1/2 - Base 6,5탎ec (160kHz) at 321kHz prescale*/
int16_t  Timer1_premux		__attribute__ ((section (".datainit2"))) = 4; /*1/32 - Base 99,7탎ec (10,03kHz) at 321kHz prescale*/
int16_t  Timer2_premux		__attribute__ ((section (".datainit2"))) = 0; /*1/2 - Base 5탎ec (200kHz) at 400kHz prescale*/
int16_t  Timer3_premux		__attribute__ ((section (".datainit2"))) = 3; /*1/16 - Base 40탎ec (25kHz) at 400kHz prescale*/    // Slow Clock Timer alias T_out2
int16_t  Timer4_premux		__attribute__ ((section (".datainit2"))) = 4; /*TCLK*/
int16_t  Timer5_premux		__attribute__ ((section (".datainit2"))) = 1; /*1/4 - Base 10탎ec (100kHz) at 400kHz prescale*/	  // Tick Timer

uint32_t ClockTick		__attribute__ ((section (".datainit2"))) = 0;
int32_t  TimeTick		__attribute__ ((section (".datainit2"))) = 1;

/*
 * Low level init routine called onece after reset.
 * The CPU is in System state.
 * (no data nor any bss can be accessed here)
 * This routine is called before entering main.
 */
void sys_preinit_hook(void)
{
	register int i;
	
	/*************
	 * Port init *
	 *************/

	//CAUTION:Follow the configuration order for setting the ports.
	// 1) setting value
	// 2) setting control register
	// 3) configure pull-up resistor.

	//16bit data bus configuration

	//PORT A GROUP
	//ADDR24 ADDR23 ADDR22 ADDR21 ADDR20 ADDR19 ADDR18 ADDR17 ADDR16 ADDR0
	//   1,     1,     1,    1,     1,     1,     1,     1,     1,     1
	rPCONA = 0x3ff;

	//PORT B GROUP
	//nGCS5 nGCS4 nGCS3 nGCS2 nGCS1  OUT  OUT  nSRAS nSCAS SCLK SCKE
	//  1,  1,  1,   1,	1,	0,	0,	1,   1 ,   1,   1
	rPDATB = 0x0;
	rPCONB = 0x7Cf;

#if (BUSWIDTH == 16)
	//PORT C GROUP
	// OUT OUT RXD1 TXD1 OUT OUT OUT IN B-IN B-IN B-IN B-IN OUT IN OUT OUT
	// 01  01  11   11   01  01  01  00 00   00   00   00   01  00 01  01
	rPDATC=0x0;
	rPCONC=0x5f540045;
	rPUPC=0xFFFF&(~0x21F4);  //should be enabled for all input
	// PC15 = OUT 
	// PC14 = OUT
	// PC11 = OUT
	// PC10 = OUT
	// PC9 = OUT
	// PC8 = IN
	// PC7 = IN 
	// PC6 = IN
	// PC5 = IN
	// PC4 = IN
	// PC3 = OUT
	// PC2 = IN 
	// PC1 = OUT
	// PC0 = OUT
#else 
#error "Wrong size. Buswith must be 16."
#endif

	//PORT D GROUP
	// OUT OUT OUT OUT OUT OUT OUT OUT
	// 01  01  01  01  01  01  01  01
	rPDATD=0x0;
	rPCOND=0x5555;
	rPUPD=0xFF&(~0x0);
	// PD7 = out7
	// PD6 = out6
	// PD5 = out5
	// PD4 = out4
	// PD3 = out3
	// PD2 = out2
	// PD1 = out1
	// PD0 = out0

	//PORT E GROUP
	// ENDIAN OUT TOUT3 TCLK TOUT1 TOUT0 RxD0 TxD0 FOUT
	// 00     01  10    11   10    10    10   10   11
	rPDATE=0x0;
	rPCONE=0x6EAB;
	rPUPE=0x1FF&(~0x124);	// ENDIAN, TCLK, RxD0
	// PE7 = out


	//PORT F GROUP
	// SIOCLK SIORxD SIORDY SIOTxD nBReq nBAck nWait IICSDA IICSCL
	// 011    011    011    011    10    10    10    10     10
	rPDATF=0x0;
	rPCONF=0x1B6EAA;
	rPUPF=0x1FF&(~0x1FF); // paranoia 0x0


	//PORT G GROUP
	// OUT EINT6 EINT5 EINT4 EINT3 EINT2 EINT1 EINT0
	// 01  11    11    11    11    11    11    11
	rPDATG=0x0;
	//rPCONG=0x43FF; // IN IN EINT4 EINT3 EINT2 EINT1 EINT0
	rPCONG=0x7FFF;
	rPUPG= 0xFF&(~0x7F);	  // IN IN EINT4 EINT3 EINT2 EINT1 EINT0
	// PG7 = out
	// PG6 = in
	// PG5 = in

	rSPUCR=0x7;  //pull-up disable in IDLE mode

	//rEXTINT=0x6DB241;   //EINT[4:7] falling edge
	//rEXTINT=0x241; //EINT[4:] will be level based
	//rEXTINT=0x2914;  //EINT[4:0] will be edge triggered.
	// EINT4 = low level or falling edge triggered
	// EINT3 = high level or rising edge triggered
	// EINT2 = high level or rising edge triggered
	// EINT1 = low level
	// EINT0 = high level or rising edge triggered
	rEXTINT= 0		// EINT7 - bit 30 - 28
		| (2<<24)	// EINT6 - bit 26-24 = falling edge triggered
		| (2<<20)	// EINT5 - bit 22-20 = falling edge triggered
		| (2<<16)	// EINT4 - bit 18-16 = falling edge triggered
		| (1<<12)	// EINT3 - bit 14-12 = high level
		| (1<<8)	// EINT2 - bit 10-8 = high level
		  		// EINT1 - bit 6-4 = low level
		| 1		// EINT0 - bit 2-0 = high level triggered
		;

	/**************
	 * init cache *
	 **************/
	
	/*Set Non cachable area (all but SDRAM)*/
	rNCACHBE0=0xC0000000;
	
	/*turn Cache off*/
	rSYSCFG=SYSCFG_0KB; 
	
	/* flush Cache */
	/*clear TAG0-1 and LRU*/
	for(i=0x10002000;i<0x10003000;i+=16)
	{
		*((int *)i)=0x0;
	}
	/*ignore Tag2-3 at 0x10003000 - 0x10004000*/
	/*clear whole ?? LRU*/
	for(i=0x10004000;i<0x10004800;i+=16)
	{
		*((int *)i)=0x0;
	}
	/*Set CACHE to 4KByte and intenal RAM to 4KByte*/
	rSYSCFG=SYSCFG_4KB;
	return;
}


/*
 * Low level init routine called onece after reset.
 * The CPU is in System state.
 * This routine is called before entering main.
 */
void sys_init_hook(void)
{	
	/*****************************
	 * init interruptVectorTable *
	 *****************************/

	HandleADC	= (volatile vf *)no_isr;
	HandleRTC	= (volatile vf *)no_isr;
	HandleUTXD1	= (volatile vf *)no_isr;
	HandleUTXD0	= (volatile vf *)no_isr;
	HandleSIO	= (volatile vf *)no_isr;
	HandleIIC	= (volatile vf *)no_isr;
	HandleURXD1	= (volatile vf *)no_isr;
	HandleURXD0	= (volatile vf *)no_isr;
	HandleTIMER5	= (volatile vf *)Timer5_ISR;
	HandleTIMER4	= (volatile vf *)no_isr;
	HandleTIMER3	= (volatile vf *)no_isr;
	HandleTIMER2	= (volatile vf *)no_isr;
	HandleTIMER1	= (volatile vf *)no_isr;
	HandleTIMER0	= (volatile vf *)no_isr;
	HandleUERR01	= (volatile vf *)no_isr;
	HandleWDT	= (volatile vf *)no_isr;
	HandleBDMA1	= (volatile vf *)no_isr;
	HandleBDMA0	= (volatile vf *)no_isr;
	HandleZDMA1	= (volatile vf *)no_isr;
	HandleZDMA0	= (volatile vf *)no_isr;
	HandleTICK	= (volatile vf *)no_isr;
	HandleEINT4567	= (volatile vf *)no_isr;
	HandleEINT3	= (volatile vf *)no_isr;
	HandleEINT2	= (volatile vf *)no_isr;
	HandleEINT1	= (volatile vf *)no_isr;
	HandleEINT0	= (volatile vf *)no_isr;

	rINTCON = 0x4; /*Enable S3C44B0X IRQ handling*/

	/* init timer defaults */
	
	/* No Dead zone*/
	/* Init Prescaler value for Timer 4 & 5 */
	/* Init Prescaler value for Timer 2 & 3 */
	/* Init Prescaler value for Timer 0 & 1 */
	rTCFG0 = (Timer45prescale<<16) | (Timer23prescale<<8) | Timer01prescale ;
	/* No DMA mode; Sel PreMux for Timer 0 - 5*/
	rTCFG1 = (Timer5_premux<<20)
			|(Timer4_premux<<16) 
			|(Timer3_premux<<12) 
			|(Timer2_premux<<8) 
			|(Timer1_premux<<4) 
			| Timer0_premux ;


	StartTimer5(9); // 9 downto 0 are 10 counts

	/*Release Defined S3C44B0X IRQ's*/
	rINTMSK = rINTMSK & ~(BIT_GLOBAL);

	return;
}

/*
 * Main Clock - Tick - Timer
 */
void StartTimer5(int cnt)
{
	/*Load counter value*/
	rTCNTB5 = cnt & 0xffff;

	rTCON = (rTCON & ~(7<<24))	// clear Timer 5 Control Bits
			| 1<<26 	// Auto reload
			| 1<<25 	// Update Counter
				   ;	// Stop
	rTCON = (rTCON & ~(7<<24))	// clear Timer 5 Control Bits
			| 1<<26 	// Auto reload
			 		// Update Counter - finish
			| 1<<24;	// Start

	/*Enable Timer5 Interrupt*/
	rINTMSK = rINTMSK & ~(BIT_TIMER5);
	return;
}

/*
 *
 */
void StopTimer5(void)
{
	/*Disable Timer5 Interrupt*/
	rINTMSK = rINTMSK | BIT_TIMER5;

	rTCON = (rTCON & ~(3<<24));	// clear Timer 5 Control Bits

	return;
}

