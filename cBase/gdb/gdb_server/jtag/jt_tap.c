/*
 * jt_tap.c
 *
 * Test Access Port Controller Interface
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
 * Support for Amontec Chameleon POD added by Radoslaw Szczygiel. 2005
 *
 */


/* 
 * general behaviour
 * The tap controller changes its internal states on TMS at rising edge off TCK
 * All Inputs (TDFROM) are sampled after TCLK rising
 * All Outputs (TDTO) are propageted after TCLK falling 
 */
#ifdef __GNUC__
#ifdef HAVE_I386_SET_IOPERM
#include <sys/types.h>
#include <machine/sysarch.h>
#ifndef WRAPPER
#include <machine/cpufunc.h>
#endif
#include <err.h>
#endif /*HAVE_I386_SET_IOPERM*/
#include <unistd.h>
#include <stdio.h>
#endif /*__GNUC__*/
#include <sysexits.h>

#include <stdlib.h>
#include <string.h>
#include "dbg_msg.h"

#ifdef WRAPPER
#define DO_NOT_CREATE_IO_FUNCTIONS
#include "jt_io.h"
#include "jt_tap.h"

#ifndef GRANT_SRESET_TIMEOUT 
#define GRANT_SRESET_TIMEOUT 250000  /*0.25 sec*/
#endif
int tap_grant_sreset_timout = GRANT_SRESET_TIMEOUT;

#ifndef SETTLE_SYSTEM_TIMEOUT
#define SETTLE_SYSTEM_TIMEOUT 100000 /*0.1 sec*/
#endif
int tap_settle_system_timeout = SETTLE_SYSTEM_TIMEOUT;

struct chain_head chain_head = { 0, 0, NULL, NULL};

static void (*call_tap_probe)(void) = NULL;
static void (*call_tap_delay)(void) = NULL;
static void (*call_tap_hard_reset)(void) = NULL;
static void (*call_tap_reset)(void) = NULL;
static void (*call_tap_start)(void) = NULL;
static void (*call_tap_stop)(void) = NULL;
static void (*call_tap_idle)(void) = NULL;
static void (*call_tap_instr)(int num_bits, char *to_dev, char *from_dev) = NULL;
static void (*call_tap_data)(int num_bits, char *to_dev, char *from_dev) = NULL;
static void (*call_tap_discover_chain)(void) = NULL;
static int (*call_tap_raw_io_test)(void) = NULL;

void tap_probe(void)		{ call_tap_probe(); return;}
void tap_delay(void) 		{ call_tap_delay(); return;}
void tap_hard_reset(void)	{ call_tap_hard_reset(); return;}
void tap_reset(void) 		{ call_tap_reset(); return;}
void tap_start(void)		{ call_tap_start(); return;}
void tap_stop(void)		{ call_tap_stop(); return;}
void tap_idle(void)		{ call_tap_idle(); return;}
void tap_instr(int num_bits, char *to_dev, char *from_dev)	{ call_tap_instr(num_bits, to_dev, from_dev); return;}
void tap_data(int num_bits, char *to_dev, char *from_dev)	{ call_tap_data(num_bits, to_dev, from_dev); return;}
void tap_discover_chain(void)	{ call_tap_discover_chain(); return;}
int  tap_raw_io_test(void)	{ return call_tap_raw_io_test();}


extern void tap_probe_tls(void);
extern void tap_delay_tls(void);
extern void tap_hard_reset_tls(void);
extern void tap_reset_tls(void);
extern void tap_start_tls(void);
extern void tap_stop_tls(void);
extern void tap_idle_tls(void);
extern void tap_instr_tls(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_tls(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_tls(void);
extern int  tap_raw_io_test_tls(void);

extern void tap_probe_tbdm(void);
extern void tap_delay_tbdm(void);
extern void tap_hard_reset_tbdm(void);
extern void tap_reset_tbdm(void);
extern void tap_start_tbdm(void);
extern void tap_stop_tbdm(void);
extern void tap_idle_tbdm(void);
extern void tap_instr_tbdm(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_tbdm(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_tbdm(void);
extern int  tap_raw_io_test_tbdm(void);

extern void tap_probe_tlongo(void);
extern void tap_delay_tlongo(void);
extern void tap_hard_reset_tlongo(void);
extern void tap_reset_tlongo(void);
extern void tap_start_tlongo(void);
extern void tap_stop_tlongo(void);
extern void tap_idle_tlongo(void);
extern void tap_instr_tlongo(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_tlongo(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_tlongo(void);
extern int  tap_raw_io_test_tlongo(void);

extern void tap_probe_omsp(void);
extern void tap_delay_omsp(void);
extern void tap_hard_reset_omsp(void);
extern void tap_reset_omsp(void);
extern void tap_start_omsp(void);
extern void tap_stop_omsp(void);
extern void tap_idle_omsp(void);
extern void tap_instr_omsp(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_omsp(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_omsp(void);
extern int  tap_raw_io_test_omsp(void);

extern void tap_probe_wiggler(void);
extern void tap_delay_wiggler(void);
extern void tap_hard_reset_wiggler(void);
extern void tap_reset_wiggler(void);
extern void tap_start_wiggler(void);
extern void tap_stop_wiggler(void);
extern void tap_idle_wiggler(void);
extern void tap_instr_wiggler(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_wiggler(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_wiggler(void);
extern int  tap_raw_io_test_wiggler(void);

extern void tap_probe_bblst(void);
extern void tap_delay_bblst(void);
extern void tap_hard_reset_bblst(void);
extern void tap_reset_bblst(void);
extern void tap_start_bblst(void);
extern void tap_stop_bblst(void);
extern void tap_idle_bblst(void);
extern void tap_instr_bblst(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_bblst(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_bblst(void);
extern int  tap_raw_io_test_bblst(void);

extern void tap_probe_ispl(void);
extern void tap_delay_ispl(void);
extern void tap_hard_reset_ispl(void);
extern void tap_reset_ispl(void);
extern void tap_start_ispl(void);
extern void tap_stop_ispl(void);
extern void tap_idle_ispl(void);
extern void tap_instr_ispl(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_ispl(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_ispl(void);
extern int  tap_raw_io_test_ispl(void);

extern void tap_probe_dlc(void);
extern void tap_delay_dlc(void);
extern void tap_hard_reset_dlc(void);
extern void tap_reset_dlc(void);
extern void tap_start_dlc(void);
extern void tap_stop_dlc(void);
extern void tap_idle_dlc(void);
extern void tap_instr_dlc(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_dlc(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_dlc(void);
extern int  tap_raw_io_test_dlc(void);

extern void tap_probe_apod(void);
extern void tap_delay_apod(void);
extern void tap_hard_reset_apod(void);
extern void tap_reset_apod(void);
extern void tap_start_apod(void);
extern void tap_stop_apod(void);
extern void tap_idle_apod(void);
extern void tap_instr_apod(int num_bits, char *to_dev, char *from_dev);
extern void tap_data_apod(int num_bits, char *to_dev, char *from_dev);
extern void tap_discover_chain_apod(void);
extern int  tap_raw_io_test_apod(void);

int tap_driver_init(enum DriverId driverId)
{
	switch(driverId)
	{
	case DRIVER_TINKER_LEVEL_SHIFTER:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver tls\n");
		call_tap_probe = tap_probe_tls;
		call_tap_delay = tap_delay_tls;
		call_tap_hard_reset = tap_hard_reset_tls;
		call_tap_reset = tap_reset_tls;
		call_tap_start = tap_start_tls;
		call_tap_stop = tap_stop_tls;
		call_tap_idle = tap_idle_tls;
		call_tap_instr = tap_instr_tls;
		call_tap_data = tap_data_tls;
		call_tap_discover_chain = tap_discover_chain_tls;
		call_tap_raw_io_test = tap_raw_io_test_tls;
		break;
	case DRIVER_TINKER_BDM:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver tbdm\n");
		call_tap_probe = tap_probe_tbdm;
		call_tap_delay = tap_delay_tbdm;
		call_tap_hard_reset = tap_hard_reset_tbdm;
		call_tap_reset = tap_reset_tbdm;
		call_tap_start = tap_start_tbdm;
		call_tap_stop = tap_stop_tbdm;
		call_tap_idle = tap_idle_tbdm;
		call_tap_instr = tap_instr_tbdm;
		call_tap_data = tap_data_tbdm;
		call_tap_discover_chain = tap_discover_chain_tbdm;
		call_tap_raw_io_test = tap_raw_io_test_tbdm;
		break;
	case DRIVER_TINKER_LONGO:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver tlongo\n");
		call_tap_probe = tap_probe_tlongo;
		call_tap_delay = tap_delay_tlongo;
		call_tap_hard_reset = tap_hard_reset_tlongo;
		call_tap_reset = tap_reset_tlongo;
		call_tap_start = tap_start_tlongo;
		call_tap_stop = tap_stop_tlongo;
		call_tap_idle = tap_idle_tlongo;
		call_tap_instr = tap_instr_tlongo;
		call_tap_data = tap_data_tlongo;
		call_tap_discover_chain = tap_discover_chain_tlongo;
		call_tap_raw_io_test = tap_raw_io_test_tlongo;
		break;
	case DRIVER_OLIMEX_MSP430:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver omsp\n");
		call_tap_probe = tap_probe_omsp;
		call_tap_delay = tap_delay_omsp;
		call_tap_hard_reset = tap_hard_reset_omsp;
		call_tap_reset = tap_reset_omsp;
		call_tap_start = tap_start_omsp;
		call_tap_stop = tap_stop_omsp;
		call_tap_idle = tap_idle_omsp;
		call_tap_instr = tap_instr_omsp;
		call_tap_data = tap_data_omsp;
		call_tap_discover_chain = tap_discover_chain_omsp;
		call_tap_raw_io_test = tap_raw_io_test_omsp;
		break;
	case DRIVER_OCDEMON_WIGGLER:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver wiggler\n");
		call_tap_probe = tap_probe_wiggler;
		call_tap_delay = tap_delay_wiggler;
		call_tap_hard_reset = tap_hard_reset_wiggler;
		call_tap_reset = tap_reset_wiggler;
		call_tap_start = tap_start_wiggler;
		call_tap_stop = tap_stop_wiggler;
		call_tap_idle = tap_idle_wiggler;
		call_tap_instr = tap_instr_wiggler;
		call_tap_data = tap_data_wiggler;
		call_tap_discover_chain = tap_discover_chain_wiggler;
		call_tap_raw_io_test = tap_raw_io_test_wiggler;
		break;
	case DRIVER_ALTERA_BYTEBLASTER:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver bblst\n");
		call_tap_probe = tap_probe_bblst;
		call_tap_delay = tap_delay_bblst;
		call_tap_hard_reset = tap_hard_reset_bblst;
		call_tap_reset = tap_reset_bblst;
		call_tap_start = tap_start_bblst;
		call_tap_stop = tap_stop_bblst;
		call_tap_idle = tap_idle_bblst;
		call_tap_instr = tap_instr_bblst;
		call_tap_data = tap_data_bblst;
		call_tap_discover_chain = tap_discover_chain_bblst;
		call_tap_raw_io_test = tap_raw_io_test_bblst;
		break;
	case DRIVER_LATTICE_ISPDLC:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver ispl\n");
		call_tap_probe = tap_probe_ispl;
		call_tap_delay = tap_delay_ispl;
		call_tap_hard_reset = tap_hard_reset_ispl;
		call_tap_reset = tap_reset_ispl;
		call_tap_start = tap_start_ispl;
		call_tap_stop = tap_stop_ispl;
		call_tap_idle = tap_idle_ispl;
		call_tap_instr = tap_instr_ispl;
		call_tap_data = tap_data_ispl;
		call_tap_discover_chain = tap_discover_chain_ispl;
		call_tap_raw_io_test = tap_raw_io_test_ispl;
		break;
	case DRIVER_XILINX_DLC:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver dlc\n");
		call_tap_probe = tap_probe_dlc;
		call_tap_delay = tap_delay_dlc;
		call_tap_hard_reset = tap_hard_reset_dlc;
		call_tap_reset = tap_reset_dlc;
		call_tap_start = tap_start_dlc;
		call_tap_stop = tap_stop_dlc;
		call_tap_idle = tap_idle_dlc;
		call_tap_instr = tap_instr_dlc;
		call_tap_data = tap_data_dlc;
		call_tap_discover_chain = tap_discover_chain_dlc;
		call_tap_raw_io_test = tap_raw_io_test_dlc;
		break;
	case DRIVER_AMONTEC_EPP_ACCELERATOR:
		dbgPrintf ( DBG_LEVEL_GDB_ARM_INFO, "Driver apod\n");
		call_tap_probe = tap_probe_apod;
		call_tap_delay = tap_delay_apod;
		call_tap_hard_reset = tap_hard_reset_apod;
		call_tap_reset = tap_reset_apod;
		call_tap_start = tap_start_apod;
		call_tap_stop = tap_stop_apod;
		call_tap_idle = tap_idle_apod;
		call_tap_instr = tap_instr_apod;
		call_tap_data = tap_data_apod;
		call_tap_discover_chain = tap_discover_chain_apod;
		call_tap_raw_io_test = tap_raw_io_test_apod;
		break;
	case NO_DRIVER:
	default:
		return 1;
	}
	return 0;
}

#else /*Not the WRAPPER it is a line driver*/
#include "jt_io.h"
#include "jt_tap.h"

void DRV_IO_FNCT(tap_probe)(void);
void DRV_IO_FNCT(tap_delay)(void);
void DRV_IO_FNCT(tap_hard_reset)(void);
void DRV_IO_FNCT(tap_reset)(void);
void DRV_IO_FNCT(tap_start)(void);
void DRV_IO_FNCT(tap_stop)(void);
void DRV_IO_FNCT(tap_idle)(void);
void DRV_IO_FNCT(tap_instr)(int num_bits, char *to_dev, char *from_dev);
void DRV_IO_FNCT(tap_data)(int num_bits, char *to_dev, char *from_dev);
void DRV_IO_FNCT(tap_discover_chain)(void);


void DRV_IO_FNCT(tap_probe)(void)
{
	ENTER_IOPERM;
	LEAVE_IOPERM;
}

void DRV_IO_FNCT(tap_delay)(void)
{
	inb(LPT_STATUS_PORT);
}

#ifdef WITH_TAP_LINGER
static void tap_tick(void);

#ifndef TAP_DELAY_TIMOUT
#define TAP_DELAY_TIMOUT	10
#endif

static void tap_tick(void)
{
	usleep(TAP_DELAY_TIMEOUT);
}
#else
#define tap_tick() /*void*/
#endif


void DRV_IO_FNCT(tap_hard_reset)(void)
{
#ifndef WITHOUT_RESET
	/*raise Reset line*/
	raw_Tout(RESET_OFF);	

#ifdef SRESET
	raw_Tout(TRESET | SRESET);
#else
	raw_Tout(TRESET);
#endif
	/*stay in reset*/
	usleep(tap_grant_sreset_timout); // e.g. grant Sytem reset
#ifdef SRESET
	raw_Tout(TRESET | TCLK);	 /*mhm ugly, but TCLK should be set*/
	
	/*wait after reset*/
	usleep(tap_settle_system_timeout); // e.g. release Sytem reset
	raw_Tout(TCLK);	tap_tick();
#else
	/*wait after reset*/
	raw_Tout(TCLK);	
	usleep(tap_settle_system_timeout); // e.g. release Sytem reset
#endif
#else  /*so it is WITHOUT_RESET*/
	raw_Tout(TCLK);	tap_tick();
#endif /*WITHOUT_RESET*/
	return;
}

void DRV_IO_FNCT(tap_reset)(void)
{
	/*since Reset line need not to be present we generate the Reset sequence 5 times TMS = 1*/
#ifdef AMONTEC_EPP_ACCELERATOR
	raw_Aout(C0 | C0_5BIT_SCAN | C0_TMS_SCAN | C0_TDI1_WHEN_TMS);
	raw_Dout(0x1F);
#else
	int i;

	for(i=0;i<5;i++)
	{
		raw_Tout(TMS);			tap_tick();
		raw_Tout(TMS|TCLK);		tap_tick();
	}
#endif
	return;
}

void DRV_IO_FNCT(tap_idle)(void)
{
	/*set machine into Idle state*/
	raw_Tout(0);		tap_tick();
	raw_Tout(TCLK);		tap_tick();

	return;
}

void DRV_IO_FNCT(tap_start)(void)
{
	raw_device_disable();	tap_tick();
	raw_device_on();	tap_tick();

	DRV_IO_FNCT(tap_reset)();
	DRV_IO_FNCT(tap_idle)();
	return;
}

void DRV_IO_FNCT(tap_stop)(void)
{
	raw_device_off();

	return;
}

#ifdef AMONTEC_EPP_ACCELERATOR
/*
 *
 * The first char of to_dev is shifted as first bit into jtag chain.
 * The first char of from_dev is the first shifted out of the jtag chain
 */
void DRV_IO_FNCT(tap_instr)(int num_bits, char *to_dev, char *from_dev) // Allways 4 bits and to_dev only
{
	char tdto, nb_unaligned_bits, temp;
	int  i,j, nb_writes;

	raw_Aout( C0 | C0_4BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);
	raw_Dout( SEQ_selDR_selIR_captureIR_shiftIR );

	nb_unaligned_bits = (num_bits-1) % 8;
	if (nb_unaligned_bits) 
	{
		temp = 0;
		//write
		for(i=0; i < nb_unaligned_bits; i++)
		{
			/*calculate if TDI to the device is to be set*/
			if(to_dev != NULL && *to_dev != 0)
			{
				tdto = 1;
				to_dev++;
			}
			else
			{
				tdto = 0;
				if(to_dev != NULL) 
					to_dev++;
			}
			temp |= (tdto << i);
		}
		raw_Aout( C0 | ((nb_unaligned_bits-1) << 4) | C0_TDI_SCAN );
		raw_Dout( temp );
		//read
		if(from_dev != NULL )
		{
			temp = raw_Din();
			for (i = nb_unaligned_bits-1; i >= 0 ; i--) 
			{
				*from_dev = (temp >> i) & 1;
				from_dev++;
			}
		}
	}

	nb_writes = (num_bits-1) / 8;

	for (j = 0; j < nb_writes; j++) 
	{
		temp = 0;
		for(i=0; i < 8; i++)
		{
			/*calculate if TDI to the device is to be set*/
			if(to_dev != NULL && *to_dev != 0)
			{
				tdto = 1;
				to_dev++;
			}
			else
			{
				tdto = 0;
				if(to_dev != NULL) 
					to_dev++;
			}
			temp |= (tdto << i);
		}
		if( (j == 0) && (nb_unaligned_bits == 0) ) 
			raw_Aout( C0 | ((8-1) << 4) | C0_TDI_SCAN );
		raw_Dout( temp );		
		//read
		if(from_dev != NULL )
		{
			temp = raw_Din();
			for (i = 7; i >= 0; --i) 
			{
				*from_dev = (temp >> i) & 1;
				from_dev++;
			}
		}
	}

	/*set state (Exit1-IR) and set state (Update-IR)*/
	if(*to_dev != 0)
		raw_Aout( C0 | C0_2BIT_SCAN | C0_TMS_SCAN | C0_TDI1_WHEN_TMS);
	else
		raw_Aout( C0 | C0_2BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);
	raw_Dout( 0x03 );	// 11

	/*collect TDO from device*/
	if(from_dev != NULL)
	{
		*from_dev = raw_Din() & 1;
	}

	return;
}


void DRV_IO_FNCT(tap_data)(int num_bits, char *to_dev, char *from_dev)
{
	char tdto, nb_unaligned_bits, temp;
	int  i,j, nb_writes;

	raw_Aout( C0 | C0_3BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);
	raw_Dout( SEQ_selDR_captureDR_shiftDR );

	nb_unaligned_bits = (num_bits-1) % 8;
	if (nb_unaligned_bits) 
	{
		temp = 0;
		//write
		for(i=0; i < nb_unaligned_bits; i++)
		{
			/*calculate if TDI to the device is to be set*/
			if(to_dev != NULL && *to_dev != 0)
			{
				tdto = 1;
				to_dev++;
			}
			else
			{
				tdto = 0;
				if(to_dev != NULL) 
					to_dev++;
			}
			temp |= (tdto << i);
		}
		raw_Aout( C0 | ((nb_unaligned_bits-1) << 4) | C0_TDI_SCAN );
		raw_Dout( temp );
		//read
		if(from_dev != NULL )
		{
			temp = raw_Din();
			for (i = nb_unaligned_bits-1; i >= 0 ; i--) 
			{
				*from_dev = (temp >> i) & 1;
				from_dev++;
			}
		}
	}

	nb_writes = (num_bits-1) / 8;

	for (j = 0; j < nb_writes; j++) 
	{
		temp = 0;
		for(i=0; i < 8; i++)
		{
			/*calculate if TDI to the device is to be set*/
			if(to_dev != NULL && *to_dev != 0)
			{
				tdto = 1;
				to_dev++;
			}
			else
			{
				tdto = 0;
				if(to_dev != NULL) 
					to_dev++;
			}
			temp |= (tdto << i);
		}
		if( (j == 0) && (nb_unaligned_bits == 0) ) 
 			raw_Aout( C0 | ((8-1) << 4) | C0_TDI_SCAN );
		raw_Dout( temp );		
		//read
		if(from_dev != NULL )
		{
			temp = raw_Din();
			for (i = 7; i >= 0; --i) 
			{
				*from_dev = (temp >> i) & 1;
				from_dev++;
			}
		}
	}

#if 0
	/*set state (Exit1-DR) and set state (Update-DR) and  (Run-Test/Idle) */
	if(*to_dev != 0)
		raw_Aout( C0 | C0_3BIT_SCAN | C0_TMS_SCAN | C0_TDI1_WHEN_TMS);
	else
		raw_Aout( C0 | C0_3BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);
	raw_Dout( 0x03 );	// 011	
#else
	/*set state (Exit1-DR) and set state (Update-DR)*/
	if(*to_dev != 0)
		raw_Aout( C0 | C0_2BIT_SCAN | C0_TMS_SCAN | C0_TDI1_WHEN_TMS);
	else
		raw_Aout( C0 | C0_2BIT_SCAN | C0_TMS_SCAN | C0_TDI0_WHEN_TMS);
	raw_Dout( 0x03 );	// 11
#endif

	/*collect last TDO from device*/
	if(from_dev != NULL)
	{
		*from_dev = raw_Din() & 1;
	}
	return;
}

#else /*normal parallel port devices*/
/*
 *
 * The first char of to_dev is shifted as first bit into jtag chain.
 * The first char of from_dev is the first shifted out of the jtag chain
 */
void DRV_IO_FNCT(tap_instr)(int num_bits, char *to_dev, char *from_dev)
{
	char tdto;
	int  i;

	/*set state (select DR-Scan)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	/*set state (select IR-Scan)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	/*set state (Capture IR-Scan)*/
	raw_Tout(0);			tap_tick();
	raw_Tout(TCLK);			tap_tick();
	/*set state (shift IR)*/
	raw_Tout(0);			tap_tick();
	raw_Tout(TCLK);	tap_tick();
	
	for(i=0; i<num_bits; i++)
	{
		/*calculate if TDI to the device is to be set*/
		if(to_dev != NULL)
		{
			if(*to_dev != 0)
				tdto = TDTO;
			else
				tdto = 0; 
			to_dev++;
		}
		else	/* send Bytpass instruction to all Devices */
			tdto = TDTO;
		
		/* send TDI to device */
		if( i == num_bits - 1)
		{
			/*set state (Exit1-IR)*/
			raw_Tout(TMS|tdto); 		tap_tick();
			raw_Tout(TMS|TCLK|tdto);	tap_tick();
			/*collect TDO from device*/
			if(from_dev != NULL)
			{
				*from_dev = raw_Tin();
				from_dev++;
			}
			//else
			//	raw_Tin();

		}
		else
		{
			/*set state (shift IR)*/
			raw_Tout(tdto); 	tap_tick();
			raw_Tout(tdto|TCLK);	tap_tick();

			/*collect TDO from device*/
			if(from_dev != NULL)
			{
				*from_dev = raw_Tin();
				from_dev++;
			}
			//else
			//	raw_Tin();
			
		}
	}
	
	/*set state (Update-IR)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	return;
}

void DRV_IO_FNCT(tap_data)(int num_bits, char *to_dev, char *from_dev)
{
	char tdto;
	int  i;

	/*set state (select DR-Scan)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	/*set state (Capture DR-Scan)*/
	raw_Tout(0);			tap_tick();
	raw_Tout(TCLK);			tap_tick();
	/*set state (shift DR)*/
	raw_Tout(0);			tap_tick();
	raw_Tout(TCLK);			tap_tick();

	for(i=0; i<num_bits; i++)
	{
		/*calculate if TDI to the device is to be set*/
		if(to_dev != NULL && *to_dev != 0)
		{
			tdto = TDTO;
			to_dev++;
		}
		else
		{
			tdto = 0;
			if(to_dev != NULL) 
				to_dev++;
		}
		/*collect TDO from device (after we exchanged TDI)*/
		if(from_dev != NULL && i > 0)
		{
			*from_dev = raw_Tin();
			from_dev++;
		}
		/*write TDI to device*/
		if( i == num_bits - 1)
		{
			/*set state (Exit1-DR)*/
			raw_Tout(TMS|tdto);		tap_tick();
			raw_Tout(TMS|TCLK|tdto);	tap_tick();
			/*collect last TDO from device*/
			if(from_dev != NULL)
			{
				*from_dev = raw_Tin();
				from_dev++;
			}
		}
		else
		{
			/*set state (shift DR)*/
			raw_Tout(tdto); 	tap_tick();
			raw_Tout(tdto|TCLK);	tap_tick();
		}
	}

	/*set state (Update-DR)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	return;
}
#endif

/* 
 * function uses struct chain_head chain_head 
 */
void DRV_IO_FNCT(tap_discover_chain)(void) 
{
	char tdto;
	char tdfrom, tdfrom_prev;
	int  i, num_dev, is_valid;
	struct chain_info *curr, *prev = NULL;

	/* get rid of old chain_head stuff, if any */
	if(chain_head.num_of_devices > 0 && chain_head.first && chain_head.last)
	{
		curr = chain_head.first;
		for(i=0;i<chain_head.num_of_devices;i++)
		{
			prev = curr;
			curr = curr->next;
			free(prev);
		}
		
		bzero(&chain_head, sizeof(chain_head)) ;
		prev = NULL;
	}
	
	/*if the device supports IDCODE its intruction is set it otherwiswe is is in BYPASS mode */
	/*make sure we are here after Tap reset*/
	DRV_IO_FNCT(tap_hard_reset)();
	DRV_IO_FNCT(tap_reset)();
	DRV_IO_FNCT(tap_idle)();

	/*set state (select DR-Scan)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	/*set state (Capture DR-Scan)*/
	raw_Tout(0);			tap_tick();
	raw_Tout(TCLK);			tap_tick();
	/*set state (shift DR)*/
	raw_Tout(TDTO);			tap_tick();
	raw_Tout(TDTO|TCLK);		tap_tick();
	

	for(num_dev=1; ; num_dev++)
	{
		curr = (struct chain_info *) malloc(sizeof(struct chain_info));

		if(curr == NULL)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"Oh shit out off memory\n");
			DRV_IO_FNCT(tap_stop)();
			exit(EX_TEMPFAIL);
		}
		
		/*TDI (and so TDO) is set to 1 since the IDCODE set to all ones is known to be wrong*/
		tdto = TDTO;
		
		/*set state (shift DR)*/
		raw_Tout(tdto);		tap_tick();
		raw_Tout(tdto|TCLK);	tap_tick();
		/*collect TDO from device*/
		tdfrom = raw_Tin();

		if(tdfrom != 0)
		{
			/*device is in IDCODE mode*/
			curr->has_idcode = 1;
			curr->idcode[31] = 1;

			is_valid = 0;
			
			/*read out following 31 Bits*/
			for(i=30; i>=0; i--)
			{
				/*set state (shift DR)*/
				raw_Tout(tdto);		tap_tick();
				raw_Tout(tdto|TCLK);	tap_tick();
				/*collect TDO from device*/
				tdfrom = raw_Tin();

				curr->idcode[i] = tdfrom;
				if(tdfrom == 0) // so at least one 0 within the idcode -> we have a valid one
					is_valid = 1;
			}
			if(!is_valid) // not valid so it is our "all 1 end" marker
			{
				/*now we kown the number of devices within the chain*/
				num_dev--;
				free(curr);

				/*put it to chain_head*/
				chain_head.num_of_devices = num_dev;
				chain_head.first = prev;

				curr = prev;
				/*we must run through the chain to find the last*/
				while(curr)
				{
					prev = curr;
					curr = curr->next;
				}
				chain_head.last = prev;
				break;
			}
		}
		else
		{
			/*device is in Bypass mode*/
			curr->has_idcode = 0;
		}
		curr->next = prev; // sound's silly but we we are reading data from last to first 
				   // but we want to send data from first to last so we swap here
		curr->prev = NULL; // we can't assign yet since we will only know after the next loop
		if(prev != NULL)
			prev->prev = curr;
		prev = curr;
		if(num_dev>128)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"too many devices in chain. give up\n");
			DRV_IO_FNCT(tap_stop)();
			exit(EX_CONFIG);
		}
	}
	/*did we found the device(s)*/
	if(num_dev == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"device not found\n");
		DRV_IO_FNCT(tap_stop)();
		exit(EX_CONFIG);
	}

	/*OK we have our info but we've forgotten to exit, so reset to start again*/
	DRV_IO_FNCT(tap_reset)();
	DRV_IO_FNCT(tap_idle)();

	/*
	 * from now on we know the number of devices within the chain,
	 * still missing the intruction length of each device
	 *
	 * valid values that are read during capture phase are:
	 *    0..01
	 *    10..01
	 *    110..01 and so on
	 *    up to
	 *    1..101
	 */
	/*set state (select DR-Scan)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	/*set state (select IR-Scan)*/
	raw_Tout(TMS);			tap_tick();
	raw_Tout(TMS|TCLK);		tap_tick();
	/*set state (Capture IR-Scan)*/
	raw_Tout(0);			tap_tick();
	raw_Tout(TCLK);			tap_tick();
	/*set state (shift IR)*/
	raw_Tout(TDTO);			tap_tick();
	raw_Tout(TDTO|TCLK);		tap_tick();
	
	/* send 1 and 0 to be sure that we see the end*/
	tdto = TDTO;
		
	/*set state (shift IR)*/
	raw_Tout(tdto);			tap_tick();
	raw_Tout(tdto|TCLK);		tap_tick();
	/*collect TDO from device*/
	tdfrom = raw_Tin();

	/*the first bit must be 1*/;
	if(tdfrom == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"not possible to collect the instruction length\n");
		DRV_IO_FNCT(tap_stop)();
		exit(EX_PROTOCOL);
	}
	
	/*now the 0*/
	tdto = 0;
		
	/*set state (shift IR)*/
	raw_Tout(tdto);			tap_tick();
	raw_Tout(tdto|TCLK);		tap_tick();
	/*collect TDO from device*/
	tdfrom = raw_Tin();

	/*the second bit must be 0*/;
	if(tdfrom != 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR
			,"not possible to collect the instruction length(missing 01)\n");
		DRV_IO_FNCT(tap_stop)();
		exit(EX_PROTOCOL);
	}
	
	tdfrom_prev = tdfrom;
	
	curr = chain_head.last;
	
	for(curr = chain_head.last; curr ; curr = curr->prev)
	{
		for(i=1;;i++)
		{
			/* send Bypass instruction to all Devices */
			tdto = TDTO;
		
			/*set state (shift IR)*/
			raw_Tout(tdto);		tap_tick();
			raw_Tout(tdto|TCLK);	tap_tick();
			/*collect TDO from device*/
			tdfrom = raw_Tin();

			/*we are waiting on a change from 1 to 0*/
			if(tdfrom_prev != 0 && tdfrom == 0)
			{
				tdfrom_prev = tdfrom;
				curr->instr_len = i; // trust me the smallest value is 2
				break;
			}

			tdfrom_prev = tdfrom;
			/*check for nonsence*/
			if(i>64)
			{
				dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"instuction length much to big\n");
				DRV_IO_FNCT(tap_stop)();
				exit(EX_PROTOCOL);
			}
		}
	}

	chain_head.total_length = 0;
	curr = chain_head.first;
	for(/*curr = chain_head.last*/; curr ; curr = curr->next)
		chain_head.total_length += curr->instr_len;
	
	/*we have filled up all chain info - verify that only Bypass instruction placed in the Jtag chain*/
	for(i=0; i<chain_head.total_length; i++)
	{
		/* send Bypass instruction to all Devices */
		tdto = TDTO;
		
		/*set state (shift IR)*/
		raw_Tout(tdto);		tap_tick();
		raw_Tout(tdto|TCLK);	tap_tick();
		/*collect TDO from device*/
		tdfrom = raw_Tin();

		if(tdfrom == 0)
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_ERROR,"more instructions than expected\n");
			DRV_IO_FNCT(tap_stop)();
			exit(EX_PROTOCOL);
		}
	}

	/*OK we have our info but we've forgotten to exit, so reset to start again*/
	DRV_IO_FNCT(tap_reset)();
	DRV_IO_FNCT(tap_idle)();

	return;
}
#endif /*WRAPPER*/

