/*
 * jt_io.h
 * 
 * Support functions to access the JTAG interface via parallel port
 *
 * Copyright (C) 2005,2006
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
 * This is a rewritten device driver part, collecting its infomation from GPL sources
 * of OpenWinCE Project and Ramon Longo's Parallel port test program 
 * see
 * http://openwince.sourceforge.net/jtag
 * http://bdm4gdb.sourceforge.net
 * http://jtag-arm9.sourceforge.net/hardware.html
 * and
 * http://www.sourceforge.net/project/gdbice
 * for more details
 * Support for Amontec Chameleon POD added by Radoslaw Szczygiel.
 * see http://www.amontec.com for more details
 */

/*Define some Macro's*/

#if defined( TINKER_LEVEL_SHIFTER ) \
 || defined( TINKER_BDM ) \
 || defined( TINKER_LONGO ) \
 || defined( OLIMEX_MSP430 ) \
 || defined( OCDEMON_WIGGLER ) \
 || defined( ALTERA_BYTEBLASTER ) \
 || defined( LATTICE_ISPDLC ) \
 || defined( XILINX_DLC) \
 || defined( AMONTEC_EPP_ACCELERATOR) \
 || defined( DO_NOT_CREATE_IO_FUNCTIONS ) 
#else
#error "missing definition of driver"
#endif

extern unsigned port_base;

//EPP Parallel Port Definition
#define LPT_DATA_PORT		(port_base)
#define LPT_STATUS_PORT		(port_base+1)
#define LPT_CONTROL_PORT	(port_base+2)
#ifdef LPT_EPP_MODE
#define EPP_ADDR_PORT		(port_base+3)
#define EPP_DATA_PORT		(port_base+4)
#endif

// set default value for bits 4,5,6 and 7 
// bit 4 Interrupt enable
// bit 5 Direction input (at BiDir Ports)
// bit 6 and 7 ???
#define LPTCTR_DEFAULT 0x00

#if defined(DO_NOT_CREATE_IO_FUNCTIONS)
#define inb( P )	0
#define outb( P , V )
#define insb( P , A , C)
#define outsb( P , A , C)
extern int ioperm( unsigned long from, unsigned long num, int permit );
#define ENTER_IOPERM
#define LEAVE_IOPERM

#elif defined(HAVE_I386_SET_IOPERM)
extern int ioperm( unsigned long from, unsigned long num, int permit );
/*
 * Inside of the header file machine/sysarch.h we already have defined the static inline functions
 * inb, outb .. etc. so we are happy with this.
 */
#ifdef LPT_EPP_MODE
#define ENTER_IOPERM \
	if (ioperm( port_base, 5, 1 )) \
	{ \
		fprintf( stderr, "\tmake sure you are super-user\n" ); \
		exit(EX_NOPERM); \
	} \
	usleep(1);

#define LEAVE_IOPERM	ioperm( port_base, 5, 0 );
#else /*non LPT_EPP_MODE*/
#define ENTER_IOPERM \
	if (ioperm( port_base, 3, 1 )) \
	{ \
		fprintf( stderr, "\tmake sure you are super-user\n" ); \
		exit(EX_NOPERM); \
	} \
	usleep(1);

#define LEAVE_IOPERM	ioperm( port_base, 3, 0 );
#endif /*LPT_EPP_MODE*/

#elif defined(HAVE_IOPERM)
extern int ioperm( unsigned long from, unsigned long num, int permit );
/*nothing to do*/
#ifdef LPT_EPP_MODE
#define ENTER_IOPERM \
	if (ioperm( port_base, 5, 1 )) \
	{ \
		fprintf( stderr, "Error: ioperm() failed. Please install ioperm.sys driver.\n" ); \
		fprintf( stderr, "\ti.g. use command ioperm -v -i\n" ); \
		fprintf( stderr, "\tmake sure you are super-user\n" ); \
		exit(EX_NOPERM); \
	} \
	usleep(1);

#define LEAVE_IOPERM	ioperm( port_base, 5, 0 );
#else /*non LPT_EPP_MODE*/
#define ENTER_IOPERM \
	if (ioperm( port_base, 3, 1 )) \
	{ \
		fprintf( stderr, "Error: ioperm() failed. Please install ioperm.sys driver.\n" ); \
		fprintf( stderr, "\ti.g. use command ioperm -v -i\n" ); \
		fprintf( stderr, "\tmake sure you are super-user\n" ); \
		exit(EX_NOPERM); \
	} \
	usleep(1);

#define LEAVE_IOPERM	ioperm( port_base, 3, 0 );
#endif /*LPT_EPP_MODE*/
#ifdef _SYS_IO_H
#error "don't use sys/io.h -> due to swaped port and value parameter"
#else
/*
 * We have not yet definded the static inline functions
 * inb, outb .. etc. so we must do this now.
 * This is quite ugly but it works -- 
 */
static inline unsigned char inb( unsigned short int port )
{
	unsigned char _v;

	__asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
	return _v;
}

static inline void outb(unsigned short int port, unsigned char value)
{
	__asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}

static inline void insb(unsigned short int port, char *addr, int cnt)
{
	__asm__ __volatile__ ("cld; rep; insb"
			 : "=D" (addr), "=c" (cnt)
			 :  "0" (addr),  "1" (cnt), "Nd" (port)
			 : "memory");
}

static inline void outsb(unsigned short int port, char *addr, int cnt)
{
	__asm__ __volatile__ ("cld; rep; outsb"
			 : "=S" (addr), "=c" (cnt)
			 :  "0" (addr),  "1" (cnt), "Nd" (port));
}
#endif /*_SYS_IO_H*/
	
#else
#define ENTER_IOPERM
#define LEAVE_IOPERM
extern int ioperm( unsigned long from, unsigned long num, int permit );

#ifdef _SYS_IO_H
#error "don't use sys/io.h -> due to swaped port and value parameter"
#else
/*
 * We have not yet definded the static inline functions
 * inb, outb .. etc. so we must do this now.
 * This is quite ugly but it works -- 
 */
static inline unsigned char inb( unsigned short int port )
{
	unsigned char _v;

	__asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
	return _v;
}

static inline void outb(unsigned short int port, unsigned char value)
{
	__asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}

static inline void insb(unsigned short int port, char *addr, int cnt)
{
	__asm__ __volatile__ ("cld; rep; insb"
			 : "=D" (addr), "=c" (cnt)
			 :  "0" (addr),  "1" (cnt), "Nd" (port)
			 : "memory");
}

static inline void outsb(unsigned short int port, char *addr, int cnt)
{
	__asm__ __volatile__ ("cld; rep; outsb"
			 : "=S" (addr), "=c" (cnt)
			 :  "0" (addr),  "1" (cnt), "Nd" (port));
}
#endif /*_SYS_IO_H*/
#endif /*HAVE_IOPERM*/ /*HAVE_I386_SET_IOPERM*/  /*DO_NOT_CREATE_IO_FUNCTIONS*/

#if defined( TINKER_LEVEL_SHIFTER ) \
 || defined( TINKER_BDM ) \
 || defined( TINKER_LONGO ) \
 || defined( OLIMEX_MSP430 ) \
 || defined( OCDEMON_WIGGLER ) \
 || defined( ALTERA_BYTEBLASTER ) \
 || defined( LATTICE_ISPDLC ) \
 || defined( XILINX_DLC) \
 || defined( AMONTEC_EPP_ACCELERATOR)
static inline void raw_device_on(void);
static inline void raw_device_off(void);
static inline void raw_device_disable(void);
#ifndef AMONTEC_EPP_ACCELERATOR
static inline void raw_Tout(int outbyte);
#endif
static inline char raw_Tin(void);
#endif

#ifdef TINKER_LEVEL_SHIFTER // Thomas own jtag cable driver

/* 
 * The cabel bus driver is powered via D Pin of the LPT
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[3:0] - used to power the levelshifter at LPT side
 *      D[4]   - Direction of output (must be 0 if enabled)
 *      D[5]   - Enable output (=1) / Disable output (=0)
 *      D[6]   - Enable input (=1) / Disable input (=0)
 *      D[7]   - Direction of input (must be 1 if enabled)
 */
#define DEV_ON      0x2F /*DATA 0010 1111 - Power on , set direction and enable signals*/
#define DEV_DISABLE 0x7F /*DATA 0111 1111 - Power on , set direction and disable signals*/
#define DEV_OFF     0x00 /*DATA 0000 0000 - Power off*/

/* 
 * The Ctr Signals are used to transfer data to the device
 * PORT_BASE + 2
 * 0 - STROBE (/pin 1)   - TCLK
 * 1 - AUTOFD (/pin 14)  - TMS
 * 2 - /INIT  (pin 16)   - TRESET
 * 3 - SELECT (/pin 17)  - TDTO (TDI)
 */
#define TCLK	0x01
#define TMS	0x02
#define TRESET	0x04 /*active LOW*/
#define TDTO	0x08 /*to device:   at the target device this is the TDI - Pin*/

#define RESET_OFF	0x0

/* 
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 * 7 - BUSY  (/pin 11)
 * 6 - ACK   (pin 10)
 * 5 - PE    (pin 12)
 * 4 - ONLINE(pin 13) - TDFROM (TDO)
 * 3 - ERROR (pin 15)
 */
#define TDFROM	0x10 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;

	outb(LPT_CONTROL_PORT,LPTCTR_DEFAULT | TMS | TCLK | TDTO );
	outb(LPT_DATA_PORT,DEV_ON);
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_CONTROL_PORT,LPTCTR_DEFAULT | TMS | TCLK | TDTO );
	outb(LPT_DATA_PORT,DEV_OFF);
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT,LPTCTR_DEFAULT | TMS | TCLK | TDTO );
	outb(LPT_DATA_PORT,DEV_DISABLE);
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte = LPTCTR_DEFAULT | (~outbyte & 0x0F);
	outb(LPT_CONTROL_PORT, outbyte);
	return;
}

#ifdef FASTER_IO
static inline void raw_ToutSeq(enum RawTouSeq RawTouSeq)
{
	switch(RawTouSeq)
	{
	case	SEQ_selDR_selIR_captureIR_shiftIR:
		outsb(LPT_CONTROL_PORT, seqString_selDR_selIR_captureIR_shiftIR, SEQ_LEN_selDR_selIR_captureIR_shiftIR);
		break;
	case	SEQ_selDR_captureDR_shiftDR:
		outsb(LPT_CONTROL_PORT, seqString_selDR_captureDR_shiftDR, SEQ_LEN_selDR_captureDR_shiftDR);
		break;
	case	SEQ_exit1_H: /*	(tdto = 1)*/
		outsb(LPT_CONTROL_PORT, seqString_exit1_H, SEQ_LEN_exit1);
		break;
	case	SEQ_exit1_L: /* (tdto = 0)*/
		outsb(LPT_CONTROL_PORT, seqString_exit1_L, SEQ_LEN_exit1);
		break;
	case	SEQ_shift_H: /* (tdto = 1)*/
		outsb(LPT_CONTROL_PORT, seqString_shift_H, SEQ_LEN_shift);
		break;
	case	SEQ_shift_L: /* (tdto = 0)*/
		outsb(LPT_CONTROL_PORT, seqString_shift_L, SEQ_LEN_shift);
		break;
	case	SEQ_update:
		outsb(LPT_CONTROL_PORT, seqString_update, SEQ_LEN_update);
		break;
	}
	return;
}
#endif

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 1;
	else
		return 0;
}

#endif // TINKER_LEVEL_SHIFTER -- Thomas own jtag cable driver

#ifdef TINKER_BDM 
/*
 * The cabel bus driver is powered from the target device
 * visite http://bdm4gdb.sourceforge.net for more informations
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - TCLK
 *      D[1] - TDTO (TDI)
 *      D[2] - TMS
 */
#define	TCLK	0x01
#define	TDTO	0x02 /*to device:   at the target device this is the TDI - Pin*/
#define	TMS	0x04

/*
 * The Ctr Signals are used to transfer data to the device
 * PORT_BASE + 2
 * 0 - STROBE (/pin 1)  - N_TRST
 * 1 - AUTOFD (/pin 14)  
 * 2 - /INIT  (pin 16)  
 * 3 - SELECT (/pin 17)  
 */
#define	N_TRST	0x01
#define TRESET	0x100 // pseudo 

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY  (/pin 11)
 * 6 - ACK   (pin 10)
 * 5 - PE    (pin 12) - TDFROM (TDO)
 * 4 - SEL   (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDFROM	0x20 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;

	outb(LPT_DATA_PORT, 0);
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_TRST);
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_DATA_PORT, 0 );
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_TRST);
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT);
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;

	outb(LPT_DATA_PORT, 0 );
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_TRST);
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT );
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x07;
	outb(LPT_DATA_PORT, outbyte);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 1;
	else
		return 0;
}

#endif //TINKER_BDM

#ifdef TINKER_LONGO
/*
 *  Ramon Longo, 2000 
 *      SCHEMATIC FOR THE PARALLEL PORT INTERFACE CIRCUIT
 *
 *  NOTE: Power comes from the JTAG connector (3.3 V) and feeds
 *        just the 74HC240 and the pullups. The 74HC240 works OK
 *        at 3.3 Volts. Instead of a 74HC240, a pair of 74HC14's
 *        can also be used. Put also some decoupling on VCC
 *        (10 uF will do).
 *
 *
 *  JTAG VCC _______________________________________  +3.3V
 *  (PIN 1+2)              |                    |
 *                    74HC240 VCC              [5K]
 *  JTAG TMS  ________/|______/|_________[22K]__|___  D0 (LPT)
 *   (PIN 7)          \|      \|                       (PIN 2)
 *                74HC240    74HC240          +3.3V
 *                                              |
 *                                             [5K]
 *  JTAG TCK  ________/|______/|_________[22K]__|___  D1 (LPT)
 *   (PIN 9)          \|      \|                       (PIN 3)
 *                74HC240    74HC240          +3.3V
 *                                              |
 *                                             [5K]
 *  JTAG TDI  ________/|______/|_________[22K]__|___  D2 (LPT) - TDTO
 *   (PIN 5)          \|      \|                       (PIN 4)
 *                74HC240    74HC240
 *		 
 *  JTAG TDO  ________|\______|\____________________  SELECT (LPT) - TDFROM
 *   (PIN 13)         |/      |/                       (PIN 13)
 *                74HC240    74HC240
 *                    74HC240 GND
 *  JTAG GND  _____________|________________________  GND  (LPT)
 *   (PIN 20)                                          (PINS 18..25)
 * The cabel bus driver is powered from the target device
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - TMS
 *      D[1] - TCLK
 *      D[2] - TDTO (TDI)
 */
#define	TMS	0x01
#define	TCLK	0x02
#define	TDTO	0x04 /*to device:   at the target device this is the TDI - Pin*/

#define TRESET	0x100 // pseudo not present

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY   (/pin 11)
 * 6 - ACK    (pin 10)
 * 5 - PE     (pin 12)
 * 4 - SEL    (pin 13) - TDFROM (TDO)
 * 3 - ERROR  (pin 15)
 */
#define TDFROM	0x10 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT );
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x07;
	outb(LPT_DATA_PORT, outbyte);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 1;
	else
		return 0;
}

#endif // TINKER_LONGO

#ifdef	OLIMEX_MSP430
/*
 * The cabel bus driver is powered from the target device
 * visite http://www.olimex.com/dev/msp-jtag.html for more informations
 *
 * The IDC14 input Pin 4 might be used to make sure that the driver did not drive 
 * the JTAG signals in case of the target device is switched off.
 * Well, I did not used it, since the OP Amp had have an offset of 0.5V.
 * So maximum driver output voltage reached 3.8V (at 3.3V input at Pin 4).
 * For that I'm leaving the Pin in the open state and reaching a maximum Voltage of 3.5V (with 3V at Pin 2).
 * This should be OK for LVTTL range 3V - 3.6V.
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - TDTO (TDI)	- IDC14 output Pin 3
 *      D[1] - TMS		- IDC14 output Pin 5
 *      D[2] - TCLK		- IDC14 output Pin 7
 *      D[3] - TCLK2 		- IDC14 output Pin 6 (NC)
 *      D[4] - used to power the levelshifter at LPT side
 *      D[7] - used to power the levelshifter at LPT side
 */
#define	TDTO	0x01 /*to device:   at the target device this is the TDI - Pin*/
#define	TMS	0x02
#define	TCLK	0x04 
#define TCLK2	0x08
#define LPTDATA_DEFAULT 0xF0

/*
 * The Ctr Signals are used to transfer data to the device
 * PORT_BASE + 2
 * 0 - STROBE (/pin 1)  - N_RST		- IDC14 output Pin 11
 * 1 - AUTOFD (/pin 14) - N_ENA_JTAG
 * 2 - /INIT  (pin 16)  - TST		- IDC14 output Pin 8
 * 3 - SELECT (/pin 17) - N_ENA_EXT
 */
#define	N_RST		0x01
#define N_ENA_JTAG	0x02
#define N_ENA_EXT	0x08
#define TRESET		0x100 // pseudo 

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY  (/pin 11)
 * 6 - ACK   (pin 10)
 * 5 - PE    (pin 12) - TDFROM (TDO)	- IDC14 input Pin 1
 * 4 - SEL   (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDFROM	0x20 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	int i;
	
	ENTER_IOPERM ;
	
	outb(LPT_DATA_PORT, LPTDATA_DEFAULT | TMS | TCLK | TDTO);
	for(i=0; i<5; i++)
	{
		inb(LPT_STATUS_PORT);
		outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_ENA_JTAG | N_ENA_EXT );
	}
	// we are going to grant the /reset now
	for(i=0; i<5; i++)
	{
		inb(LPT_STATUS_PORT);
		outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_RST | N_ENA_JTAG | N_ENA_EXT );
	}
	inb(LPT_STATUS_PORT);
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_ENA_JTAG | N_ENA_EXT );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | N_RST);
	outb(LPT_DATA_PORT, 0 );
	inb(LPT_STATUS_PORT);
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT );
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)	
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT );
	outb(LPT_DATA_PORT, LPTDATA_DEFAULT );
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x0F;
	outb(LPT_DATA_PORT, outbyte | LPTDATA_DEFAULT);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 1;
	else
		return 0;
}

#endif	//OLIMEX_MSP430

#ifdef OCDEMON_WIGGLER
/*
 * Collected from OpenWinCE and JTAG ARM9 Project
 * Macraigor Wiggler JTAG Cable Driver
 *
 * Documentation:
 * [1] http://www.ocdemon.net/
 * [2] http://jtag-arm9.sourceforge.net/hardware.html
 * 
 * PL1 25wayD Male                                                          PL2     20wayIDC
 * 
 * PL1/18-25 (GND)  <-------+----+-----------------------------+--+-----------<  PL2/4,6,8,  (GND)
 *                          |    |                             |  |                10,12,14,
 *                          0v   |   VHC244              200nF =  = 4.7uF           16,18,20
 *                               | +----------------+  Vcc     |  |
 *               TDTO            +-| 1 /1G   VCC 20 |-+--------+--+-----------<  PL2/1,2     (VCC)
 * PL1/5  (D3) >--------XXXX-------| 2 1A1   /2G 19 |-+
 *               TMS    10k        | 3       1Y1 18 |-----XXXX---------------->  PL2/5       (TDI)
 * PL1/3  (D1) >--------XXXX-------| 4 1A2       17 |     51R
 *               TCLK   10k        | 5       1Y2 16 |-----XXXX---------------->  PL2/7       (TMS)
 * PL1/4  (D2) >--------XXXX-------| 6 1A3       15 |     51R
 *                      10k        | 7       1Y3 14 |-----XXXX---------------->  PL2/9       (TCLK)
 *                            +----| 8 1A4       13 |     51R
 *                            |    | 9       1Y4 12 |-----XXXX---+
 *                            |  +-| 10 GND      11 |     51R    |
 *                            |  | +----------------+            |
 *                            |  0v                              |
 *                            +----------------------------------------------<   PL2/13      (TDO)
 *                 TDFROM                                        |
 * PL1/11 (BUSY) <-----------------------------------------------+
 * 
 * 
 *                                      DTC114  /---------xxxx---------------<   PL2/15      (/RESET)
 *                   RST            4k7       |/          51R
 * PL1/2  (D0) >--------------------XXXX--+---|
 *                                        |   |\
 *                                        X     V
 *                                   22k  X      |
 *                                        X      |
 *                                        |      |
 *                                        V 0v   V 0v
 *
 *  
 *  Pin's 11,13,15 and 17 of IC 74VHC244 are Input signals and should be feeded with a valid signal
 *  either high or low. So for example they migth be connect with GND.
 *  There are many 74*244 Variants so make sure you are use the right one.
 *  E.g. some 
 *  - ACT types uses 5V VCC power and its input pins are 0 to VCC tolerant
 *  - AC types uses 3 - 5V VCC power and its input pins are 0 to VCC tolerant
 *  - HC types uses 2 - 6V VCC power and its input pins are 0 to VCC tolerant
 *  - LVT and LVX types uses 3.3V VCC powre and its input pins are 5V TTL tolerant
 *  - VHC types uses 3 - 5V VCC power and its input pins are 5V TTL tolerant
 *  but this vary from one chip Vendor to an other Vendor so it's better to take a look at the
 *  datasheed of the chip you are going to use
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - SRESET
 *      D[1] - TMS
 *      D[2] - TCLK
 *      D[3] - TDTO (TDI)
 *      D[4] - (N_TRESET)
 *      D[5] - 
 *      D[6] - (wire to SEL pin 15)
 *      D[7] - (POWER Enable)
 */
#define SRESET	0x01
#define	TMS	0x02
#define	TCLK	0x04
#define	TDTO	0x08 /*to device:   at the target device this is the TDI - Pin*/
#define T_POWER	(0x80 | 0x40)
#define TRESET	0x10

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY  (/pin 11) - TDFROM (TDO)
 * 6 - ACK   (pin 10)
 * 5 - PE    (pin 12)
 * 4 - SEL   (pin 13) - (Target POWER_OK)
 * 3 - ERROR (pin 15) - (wire from D[6] pin 8)
 */
#define TDFROM	0x80 /*from device: at the target device this is the TDO - Pin*/
/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | 0xF);
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO | T_POWER );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT | 0xF);
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT);
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO  | T_POWER);
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x1F;

	if(outbyte & TRESET) // invert TRESET
		outbyte &= ~TRESET;
	else
		outbyte |= TRESET;
	outbyte |= T_POWER;
	outb(LPT_DATA_PORT, outbyte);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 0;
	else
		return 1;
}


#endif //OCDEMON_WIGGLER

#ifdef ALTERA_BYTEBLASTER
/* 
 * Collected from OpenWinCE Project
 * Altera ByteBlaster/ByteBlaster II/ByteBlasterMV Parallel Port Download Cable Driver
 * Written by Marcel Telka <marcel@telka.sk>, 2002, 2003.
 *
 * The cabel bus driver is powered from the target device
 * visite http://www.altera.com for more informations
 *
 * Documentation at Altera Corporation:
 * [1] ByteBlaster Parallel Port Download Cable Data Sheet
 * [2] ByteBlaster II Parallel Port Download Cable Data Sheet
 * [3] ByteBlasterMV Parallel Port Download Cable Data Sheet
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - TMS
 *      D[1] - TCLK
 *      D[6] - TDTO (TDI)
 *      D[7] - in loop with pin 12 (PE) e.g. to check if cable is present
 */
#define	TMS	0x01
#define	TCLK	0x02
#define	TDTO	0x40 /*to device:   at the target device this is the TDI - Pin*/

#define TRESET	0x100 // pseudo 

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY  (/pin 11) - TDFROM (TDO)
 * 6 - ACK   (pin 10)
 * 5 - PE    (pin 12)  - loop driven by pin 9 D[7]
 * 4 - SEL   (pin 13)  - input e.g. to check VCC power good
 * 3 - ERROR (pin 15)
 */
#define TDFROM	0x80 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT);
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_DATA_PORT, TMS | TCLK | TDTO );
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x07;
	outb(LPT_DATA_PORT, outbyte);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 0;
	else
		return 1;
}

#endif //ALTERA_BYTEBLASTER

#ifdef LATTICE_ISPDLC // Lattice ispDownload Cable
/*
 * Collected from OpenWinCE Project
 *
 * The cabel bus driver is powered from the target device
 * visite http://www.latticesemi.com for more informations
 *
 * Documentation at Lattice:
 * ispDownload Cable
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - TDTO (TDI)
 *      D[1] - TCLK
 *      D[2] - TMS
 *      D[3] - ISP_DIS
 *      D[4] - ISP_RESET
 *      D[5] - JTAG_EN
 */
#define	TDTO		0x01 /*to device:   at the target device this is the TDI - Pin*/
#define	TCLK		0x02
#define	TMS		0x04
#define ISP_DIS		0x08
#define ISP_RESET	0x10
#define JTAG_EN		0x20

#define TRESET		0x100 // pseudo 

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY  (/pin 11)
 * 6 - ACK   (pin 10) - TDFROM (TDO)
 * 5 - PE    (pin 12) 
 * 4 - SEL   (pin 13)
 * 3 - ERROR (pin 15)
 */
#define TDFROM	0x40 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT);
	outb(LPT_DATA_PORT, ISP_DIS | JTAG_EN );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_DATA_PORT, 0 );
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_DATA_PORT, 0 );
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x07;
	outbyte |= (ISP_DIS | JTAG_EN);
	outb(LPT_DATA_PORT, outbyte);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 1;
	else
		return 0;
}

#endif //LATTICE_ISPDLC

#ifdef XILINX_DLC
/*
 * Collected from OpenWinCE Project
 * Xilinx DLC5 JTAG Parallel Cable III Driver
 *
 * The cabel bus driver is powered from the target device
 * visite http://toolbox.xilinx.com/docsan/3_1i/pdf/docs/jtg/jtg.pdf for more informations
 */

/*
 * The Data Signals are used to transfer data to the device
 * PORT_BASE
 * data D[7:0] (pins 9:2)
 *      D[0] - TDTO (TDI)
 *      D[1] - TCLK
 *      D[2] - TMS
 *      D[3] - DIS_OUTPUT
 *      D[4] - ENA_INPUT
 */
#define	TDTO		0x01 /*to device:   at the target device this is the TDI - Pin*/
#define	TCLK		0x02
#define	TMS		0x04
#define DIS_OUTPUT	0x08
#define ENA_INPUT	0x10

#define TRESET		0x100 // pseudo 

#define RESET_OFF	0x0

/*
 * The Status Signals are used to collect data received from the device.
 * PORT_BASE + 1
 *
 * 7 - BUSY  (/pin 11)
 * 6 - ACK   (pin 10)
 * 5 - PE    (pin 12) 
 * 4 - SEL   (pin 13) - TDFROM (TDO)
 * 3 - ERROR (pin 15)
 */
#define TDFROM	0x10 /*from device: at the target device this is the TDO - Pin*/

/*
 * low level functions to access the Hardware
 */
static inline void raw_device_on(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_CONTROL_PORT, LPTCTR_DEFAULT);
	outb(LPT_DATA_PORT, ENA_INPUT );
	return;
}

static inline void raw_device_off(void)
{
	outb(LPT_DATA_PORT, 0 );
#if defined(HAVE_IOPERM) || defined(HAVE_I386_SET_IOPERM)
	ioperm( port_base, 3, 0 );
#endif
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	
	outb(LPT_DATA_PORT, 0 );
	return;
}

static inline void raw_Tout(int outbyte)
{
	outbyte &= 0x07;
	outbyte |= ENA_INPUT;
	outb(LPT_DATA_PORT, outbyte);
	return;
}

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = inb(LPT_STATUS_PORT);
	if((inbyte & TDFROM) == TDFROM)
		return 1;
	else
		return 0;
}


#endif //XILINX_DLC

#ifdef AMONTEC_EPP_ACCELERATOR
/*
 *  Added by Radoslaw Szczygiel.
 *
 *  Amontec Chameleon POD with Epp Amontec JTAG-Accelerator configuration (www.amontec.com)
 *  Command driven device (uses reconfiguratable Xilinx logic).
 *  Expected to be nine or more times faster than devices using standard line drivers only.
 * 
 *  see http://www.amontec.com
 */

#ifndef LPT_EPP_MODE
#error "must run in LPT_EPP_MODE"
#endif
//pseudo pins  
#define SRESET		0x01
#define	TMS		0x02
#define	TCLK		0x04
#define	TDTO		0x08 
#define TRESET		0x10
#define RESET_OFF	0x40

//Standard Bi-Di Parallel Port Definition
/* 
 * The Ctr Signals are used to transfer data to and from the device in Simulated EPP mode
 * and must be properly setup before real EPP mode
 * PORT_BASE + 2
 * 0 - STROBE (/pin 1)   - Write
 * 1 - AUTOFD (/pin 14)  - Data strobe
 * 2 - /INIT  (pin 16)   - Reset
 * 3 - SELECT (/pin 17)  - Address Strobe
 * 
 * 5 - Tristate outputs
 */
//Mask 
#define EPP_WRITE_HIGH		0x00
#define EPP_WRITE_LOW		0x01
#define EPP_DATA_STROBE_HIGH 	0x00
#define EPP_DATA_STROBE_LOW 	0x02
#define EPP_RESET_LOW	 	0x00
#define EPP_RESET_HIGH		0x04
#define EPP_ADDR_STROBE_HIGH	0x00
#define EPP_ADDR_STROBE_LOW	0x08
#define BIDI_DIRECTION_INPUT	0x20
#define BIDI_DIRECTION_OUTPUT   0x00


//###################
//# CONTROL0 register Write only
//###################
//# epp_addr_wr with data(7) = '0'
//# |7|6|5|4|3|2|1|0|
//# '0'----- | | | |
//#      |   | | | --- srst
//#      |   | | ----- trst
//#      |   | ------- tdi_value_when_tms_scan
//#      |   --------- tdi_ntms_scan
//#      ------------- nbits_scan
//###################
// Mask
#define C0_SRST_INACTIVE 0x00 /* Open drain to target */
#define C0_SRST_ACTIVE   0x01 /* 0 to target */

#define C0_TRST_INACTIVE 0x00 /* 1 to target */
#define C0_TRST_ACTIVE   0x02 /* 0 to target */

#define C0_TDI0_WHEN_TMS 0x00
#define C0_TDI1_WHEN_TMS 0x04

#define C0_TMS_SCAN	0x00
#define C0_TDI_SCAN	0x08

#define C0_1BIT_SCAN	0x00
#define C0_2BIT_SCAN	0x10
#define C0_3BIT_SCAN	0x20
#define C0_4BIT_SCAN	0x30
#define C0_5BIT_SCAN	0x40
#define C0_6BIT_SCAN	0x50
#define C0_7BIT_SCAN	0x60
#define C0_8BIT_SCAN	0x70

#define C0               0x00

//###################
//# CONTROL1 register Write only
//###################
//# epp_addr_wr with data(7) = '1'
//# |7|6|5|4|3|2|1|0|
//# '1'| | | --------
//#    | | |     |
//#    | | |     ----- baudrate
//#    | | ----------- rtck_en
//#    | ------------  jtag_port_oe
//#    --------------- test_mode
//#################
// Mask
#define C1_BAUD_16MHz		0x00
#define C1_BAUD_8MHz		0x01
#define C1_BAUD_4MHz		0x02
#define C1_BAUD_2MHz		0x03
#define C1_BAUD_1MHz		0x04
#define C1_BAUD_500kHz		0x05
#define C1_BAUD_250kHz		0x06
#define C1_BAUD_125kHz		0x07
#define C1_BAUD_62_5kHz		0x08
#define C1_BAUD_31_25kHz	0x09
#define C1_BAUD_15_62kHz	0x0A
#define C1_BAUD_7_81kHz		0x0B
#define C1_BAUD_3_90kHz		0x0C
#define C1_BAUD_1_95kHz		0x0D
#define C1_BAUD_976Hz		0x0E
#define C1_BAUD_488Hz		0x0F

#define C1_RTCK_DISABLED	0x00
#define C1_RTCK_ENABLED		0x10

#define C1_OUTPUT_DISABLED	0x00
#define C1_OUTPUT_ENABLED	0x20

#define C1_TEST_DISABLED	0x00
#define C1_TEST_ENABLED		0x40


#define C1                 0x80

//#################
//# STATUS register Read Only
//#################
//# epp_addr_rd
//# |7|6|5|4|3|2|1|0|
//# |
//# ----------------- busy_scan
//#Note: busy_scan is only used
//# - if baudrate is equal or smaller than 1MHz,
//# or
//# - if rtck_en = ‘1’
//##################
// Mask
#define STATUS_BUSY   0x80

//##################
//# SCAN-IN register Write Only
//##################
//# epp_data_wr
//# |7|6|5|4|3|2|1|0|
//###################
//# SCAN-OUT register Read Only
//###################
//# epp_data_rd
//# |7|6|5|4|3|2|1|0|

#define C1_BAUD C1_BAUD_16MHz

#define SEQ_selDR_selIR_captureIR_shiftIR	0x03 // 0011
#define SEQ_selDR_captureDR_shiftDR		0x01 // 001

// Epp Data register
static inline void raw_Dout(int outbyte)
{
	outb(EPP_DATA_PORT, outbyte);
	return;
}

static inline char raw_Din(void)
{
	int inbyte;

	inbyte = inb(EPP_DATA_PORT);
	return inbyte;
}

// Epp Address register
static inline void raw_Aout(int outbyte)
{
	outb(EPP_ADDR_PORT, outbyte);
	return;
}

static inline char raw_Ain(void)
{
	int inbyte;

	inbyte = inb(EPP_ADDR_PORT);
	return inbyte;
}

// Wrapper to emulate the simple changes
#define raw_Tout(_outbyte_) \
do {\
	if((_outbyte_) == 0)\
		raw_Aout( C0 | C0_1BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);\
	else if((_outbyte_) == TDTO)\
		raw_Aout( C0 | C0_1BIT_SCAN | C0_TMS_SCAN | C0_TDI1_WHEN_TMS);\
	else if((_outbyte_) == (TDTO | TCLK))\
		raw_Dout( 0 );\
	else if((_outbyte_) == TCLK)\
		raw_Dout( 0 );\
	else if((_outbyte_) == TMS)\
		raw_Aout( C0 | C0_1BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);\
	else if((_outbyte_) == (TMS | TCLK))\
		raw_Dout( 1 );\
	else if((_outbyte_) == (TRESET | SRESET))\
		raw_Aout(C0 | C0_SRST_ACTIVE | C0_TRST_ACTIVE);\
	else if((_outbyte_) == TRESET)\
		raw_Aout(C0 | C0_SRST_INACTIVE | C0_TRST_ACTIVE);\
	else if((_outbyte_) == (TRESET | TCLK))\
		raw_Aout(C0 | C0_SRST_INACTIVE | C0_TRST_INACTIVE);\
	else if((_outbyte_) == RESET_OFF)\
		raw_Aout(C0 | C0_SRST_INACTIVE | C0_TRST_INACTIVE);\
} while(0)

static inline char raw_Tin(void)
{
	int inbyte;

	inbyte = raw_Din();
	return inbyte & 1;
}

static inline void raw_device_on(void)
{
	ENTER_IOPERM ;	
	outb(LPT_CONTROL_PORT, EPP_RESET_LOW);
	outb(LPT_CONTROL_PORT, EPP_RESET_HIGH);
	// config outputs to High prepare to work in EPP mode
	outb(LPT_CONTROL_PORT, EPP_RESET_HIGH | EPP_ADDR_STROBE_HIGH | EPP_DATA_STROBE_HIGH | EPP_WRITE_HIGH);
	raw_Aout(C0 | C0_SRST_INACTIVE | C0_TRST_INACTIVE);
	raw_Aout(C1 | C1_BAUD  | C1_OUTPUT_ENABLED);
	return;
}

static inline void raw_device_off(void)
{
	raw_Aout(C1 | C1_BAUD | C1_OUTPUT_DISABLED);
	LEAVE_IOPERM;
	return;
}

static inline void raw_device_disable(void)
{
	ENTER_IOPERM ;
	raw_Aout(C1 | C1_BAUD | C1_OUTPUT_DISABLED);
	return;
}

#endif //AMONTEC_EPP_ACCELERATOR



