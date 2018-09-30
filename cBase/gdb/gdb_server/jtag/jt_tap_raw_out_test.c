/*
 * jt_raw_out_test 
 *
 * Copyright (C) 2005,2006
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA. 
 * 
 * Written by Thomas Klein <ThRKlein@users.sf.net>, 2005.
 */


#if defined(HAVE_IOPERM)
#include <unistd.h>
#elif defined(HAVE_I386_SET_IOPERM)
#include <sys/types.h>
#include <machine/sysarch.h>
#include <machine/cpufunc.h>
#include <unistd.h>
#endif
#include <stdio.h>
#include <sysexits.h>
#include <stdlib.h>
#include <string.h>
#include "dbg_msg.h"
#include "jt_io.h"
#include "jt_tap.h"

int DRV_IO_FNCT(tap_raw_io_test)(void);

int DRV_IO_FNCT(tap_raw_io_test)(void)
{
#ifdef	AMONTEC_EPP_ACCELERATOR
	printf("Test not yet supported\n");
	return -1;
#else
	int i;

	printf(	"Testing Cable Driver\n\n"
		"make sure that you have setup your envioment correctly\n"
		"- Only the Cable Driver is connected with a Prallel LPT Port\n"
		"  so no other Parallel Port devices are connected\n"
		"- all Cable Driver output pins are NOT connected to any target\n"
		"  so they are all open and we can mesure its Voltage\n"
		"- You have a seperate 3.3 Voltage to power the Cable\n\n"
		"Now ready to start [y/n]?\n"
		);

	i = getc(stdin);
	if(i != 'y' && i != 'Y')
		return -1;

	printf("start disable ?\n");getc(stdin);
	raw_device_disable();
	printf("disable");getc(stdin);printf("\n");
	
	printf("next on ?\n");getc(stdin);
	raw_device_on();
	printf("on");getc(stdin);printf("\n");
	
	printf("next TMS ?\n");getc(stdin);
	raw_Tout(TMS);
	printf("TMS");getc(stdin);printf("\n");
	
	printf("next TCLK ?\n");getc(stdin);
	raw_Tout(TCLK);
	printf("TCLK");getc(stdin);printf("\n");

	printf("next TDTO ?\n");getc(stdin);
	raw_Tout(TDTO);
	printf("TDTO");getc(stdin);printf("\n");
#ifdef SRESET
	printf("next SRESET ?\n");getc(stdin);
	raw_Tout(SRESET);
	printf("SRESET");getc(stdin);printf("\n");
#endif
	printf("next TRESET ?\n");getc(stdin);
	raw_Tout(TRESET);
	printf("TRESET");getc(stdin);printf("\n");

	printf("read TDFROM ?\n");getc(stdin);
	if(raw_Tin())
		printf("input of target TDO = 1\n");
	else
		printf("input of target TDO = 0\n");
	
	printf("next TMS TCLK ?\n");getc(stdin);
	raw_Tout(TMS|TCLK);
	printf("TMS TCLK");getc(stdin);printf("\n");
	
	printf("next TMS TCLK TDTO ?\n");getc(stdin);
	raw_Tout(TMS|TCLK|TDTO);
	printf("TMS TCLK TDTO");getc(stdin);printf("\n");
	
	printf("next TCLK TDTO ?\n");getc(stdin);
	raw_Tout(TCLK|TDTO);
	printf("TCLK TDTO");getc(stdin);printf("\n");
	
	printf("next TMS TDTO ?\n");getc(stdin);
	raw_Tout(TMS|TDTO);
	printf("TMS TDTO");getc(stdin);printf("\n");
	
	printf("next (all LOW) ?\n");getc(stdin);
	raw_Tout(0);
	printf("(all LOW)");getc(stdin);printf("\n");

	printf("next TMS TCLK TDTO ?\n");getc(stdin);
	raw_Tout(TMS|TCLK|TDTO);
	printf("TMS TCLK TDTO");getc(stdin);printf("\n");
	
	printf("next TDTO ?\n");getc(stdin);
	raw_Tout(TDTO);
	printf("TDTO");getc(stdin);printf("\n");

	printf("next OFF ?\n");getc(stdin);
	raw_device_off();
	printf("OFF");getc(stdin);printf("\nbye\n");

	return 0;
#endif
}

