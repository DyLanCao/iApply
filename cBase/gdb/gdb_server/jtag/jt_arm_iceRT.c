/*
 *
 * ARM7TDMI Interface
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
 * For additional info see ARM7TDMI (rev4) Technical Reference Manual
 * document ARM DDI 0210A somewere at www.arm.com
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "dbg_msg.h"
#include "jt_arm.h"
#include "jt_instr.h"


/*
 * Note: We support ARM7TDMI only.
 * At chain 2 ARM7TDMI and ARM9TDMI cores boths have an embeddedICE programing unit.
 * The length is 38 bits and the layout is similare but the behaviour is not exactly equal.
 * The ARM10TDMI core does not have this kind of unit at chain 2. (It's using a different mechanism.)
 */

/*
 *
 */
unsigned int jtag_arm_IceRT_RegRead(int nreg)
{
	char instr[38];                    /* 38 bit collecting from the device */
	char outstr[38];                   /* 38 bit senting to device */
	long ret_val;

	if(scan_chain != 2 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(2);		/* Select IceRT Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr.         */
		scan_mode = INTEST;
	}
	
	bzero(&outstr[6],32);   /* clear 32 data bit -- not required but I feel much better doing this */
	
	/* bits[32..36] = nreg, bit[37]= 0 => read */
	jtag_supp_int2bitstr_MSB_First(6, nreg, &outstr[0]);
	
	/* Enter 32+5+1 bits in two phases: */
	jtag_exchange_data(38, &outstr[0], NULL); /*send read cmd*/
	/* Now repeat the same but this time read out first 32 bits: */
	// but since we don't have to read out the comm data (reg 5) twice 
	// we send a save default read command (reg 4) instead
	if((ice_state.is_debugrequest & 4) == 4) // monitor mode => read reg 4 (dcc status)
		jtag_supp_int2bitstr_MSB_First(6, ICERT_REG_DCC_CONTROL, &outstr[0]);
	else // read reg 1 (debug status)
		jtag_supp_int2bitstr_MSB_First(6, ICERT_REG_DEBUG_STATUS, &outstr[0]);
	jtag_exchange_data(38, &outstr[0], &instr[0]); /*read 32 bit*/
	
	jtag_supp_bitstr2int_MSB_First(32, &ret_val, &instr[6]); /* Only use 32 LSB bits out of 38 */

	dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"IceRT_RegRead[%d] = %x\n",nreg,ret_val);
	return ret_val;  /* Return 32 bits readout */
}

/*
 *
 */
unsigned int jtag_arm_IceRT_RegRead_Once(int nreg)
{
	char instr[38];                    /* 38 bit collecting from the device */
	char outstr[38];                   /* 38 bit senting to device */
	long ret_val;

	if(scan_chain != 2 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(2);		/* Select IceRT Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr.         */
		scan_mode = INTEST;
	}
	bzero(&outstr[6],32);   /* clear 32 data bit -- not required but I feel much better doing this */
	
	/* make sure bit 5 is cleared (read command) */
	nreg &= ((1<<5)-1); // &= 0x0F
	/* bits[32..36] = nreg, bit[37]= 0 => read */
	jtag_supp_int2bitstr_MSB_First(6, nreg, &outstr[0]);
	
	/* Enter 32+5+1 bits and send next read cmd and collect data of previous read cmd*/
	jtag_exchange_data(38, &outstr[0], &instr[0]); 
	
	jtag_supp_bitstr2int_MSB_First(32, &ret_val, &instr[6]); /* Only use 32 LSB bits out of 38 */

	dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"IceRT_RegRead_Once[%d] = %x\n",nreg,ret_val);
	return ret_val;  /* Return 32 bits readout */
}

/*
 *
 */
void jtag_arm_IceRT_RegWrite(int nreg, unsigned int regdata)
{
	char outstr[38];                   /* 38 bit senting to device */

	if(scan_chain != 2 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(2);		/* Select IceRT Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr.         */
		scan_mode = INTEST;
	}
	bzero(&outstr[6],32);   /* clear 32 data bit -- not required but I feel much better doing this */
	
	nreg = nreg | (1<<5);                  /* bits[32..36] = nreg, bit[37]= 1 => write*/
	jtag_supp_int2bitstr_MSB_First(6,  nreg, &outstr[0]);
	jtag_supp_int2bitstr_MSB_First(32, regdata, &outstr[6]);

	jtag_exchange_data(38, &outstr[0], NULL);   /* send write cmd & read out data */
	return;
}

/*
 *
 */
unsigned int jtag_arm_IceRT_RegWrite_getPrevData(int nreg, unsigned int regdata)
{
	char instr[38];                    /* 38 bit collecting from the device */
	char outstr[38];                   /* 38 bit senting to device */
	long ret_val;

	if(scan_chain != 2 || scan_mode != INTEST)
	{
		jtag_arm_set_chain(2);		/* Select IceRT Scan chain */
		jtag_send_instr(instr_intest);	/* Enter INTEST instr.         */
		scan_mode = INTEST;
	}
	
	bzero(&outstr[6],32);   /* clear 32 data bit -- not required but I feel much better doing this */
	
	nreg = nreg | (1<<5);                  /* bits[32..36] = nreg, bit[37]= 1 => write*/
	jtag_supp_int2bitstr_MSB_First(6,  nreg, &outstr[0]);
	jtag_supp_int2bitstr_MSB_First(32, regdata, &outstr[6]);

	/* Enter 32+5+1 bits in two phases: */
	jtag_exchange_data(38, &outstr[0], instr);   /* send write cmd & read out data */
	jtag_supp_bitstr2int_MSB_First(32, &ret_val, &instr[6]); /* Only use 32 LSB bits out of 38 */

	return ret_val;  /* Return 32 bits readout */
}

/*
 *
 */
void jtag_arm_ShowAllIceRT_Regs(void)
{
	unsigned val;

	IF_DBG(DBG_LEVEL_JTAG_ICERT)
	{
		// Debug Control - REG 0
		val = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_CONTROL);
		printf ("IceB Debug Control        = 0x%08X\t", val);
		if(val & 0x20uL) printf("Disable-EICE-RT ");
		if(val & 0x10uL) printf("Monitor-Mode "); else printf("Debug-Mode ");
		if(val & 0x04uL) printf("Disable-Intr. ");
		if(val & 0x02uL) printf("Set-manual-DBGRQ ");
		if(val & 0x01uL) printf("Set-manual-DBGACK\n"); else printf("\n");
	
		// Debug Status - REG 1
		val = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);
		printf ("IceB Debug Status         = 0x%08X\t", val);
		if(val & 0x10uL) printf("T-bit ");
		if(val & 0x08uL) printf("cgenL ");
		if(val & 0x04uL) printf("IFEN ");
		if(val & 0x02uL) printf("DBGRQ ");
		if(val & 0x01uL) printf("DBGACK\n"); else printf("\n");
	
		// Abort Status - REG 2
		val = jtag_arm_IceRT_RegRead(ICERT_REG_ABORT_STATUS);
		printf ("IceB Abort Status         = 0x%08X\t", val);
		if(val & 0x01uL) printf("DBGABT\n"); else printf("\n");
	
		// DCC - REG 4 and 5
		val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
		printf ("IceB Debug Comms Control  = 0x%08X\t", val);
		printf ("ICE version %d\n", (val>>28)&0x0f);
		// we should not read Data if not there
		if(val & 0x2) // data is written from the cpu into the comm. channel
			printf ("IceB Debug Comms Data     = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA));

		// Watchpoint 0 - REG[8 up to 13]
		printf ("IceB WatchP #0 Address    = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_ADDRESS));
		printf ("IceB WatchP #0 Addr.mask  = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_ADDRMASK));
		printf ("IceB WatchP #0 Data       = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_DATA));
		printf ("IceB WatchP #0 Data mask  = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_DATAMASK));
		val = jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_CONTROL);
		printf ("IceB WatchP #0 Control    = 0x%08X\t", val);
		if(val & 0x100uL) printf("ENABLE ");
		if(val & 0x080uL) printf("RANGE ");
		if(val & 0x040uL) printf("CHAIN ");
		if(val & 0x020uL) printf("EXTERN ");
		if(val & 0x010uL) printf("/TRANS=1(priveleg)"); else  printf("/TRANS=1(user mode)");
		if(val & 0x008uL) printf("/OPC=1(data)"); else  printf("/OPC=0(instr)");
		if(val & 0x004uL) printf("MAS[1] ");
		if(val & 0x002uL) printf("MAS[0] ");
		if(val & 0x001uL) printf("write\n"); else printf("read\n");
		val = jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_CONTROLMASK);
		printf ("IceB WatchP #0 Contr.mask = 0x%08X\t", val);
		if((val & 0x080uL) == 0) printf("RANGE ");
		if((val & 0x040uL) == 0) printf("CHAIN ");
		if((val & 0x020uL) == 0) printf("EXTERN ");
		if((val & 0x010uL) == 0) printf("/TRANS ");
		if((val & 0x008uL) == 0) printf("/OPC "); 
		if((val & 0x004uL) == 0) printf("MAS[1] ");
		if((val & 0x002uL) == 0) printf("MAS[0] ");
		if((val & 0x001uL) == 0) printf("read/write\n"); else printf("\n");
	
		// Watchpoint 1 - REG[16 up to 21]
		printf ("IceB WatchP #1 Address    = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_1_ADDRESS));
		printf ("IceB WatchP #1 Addr.mask  = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_1_ADDRMASK));
		printf ("IceB WatchP #1 Data       = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_1_DATA));
		printf ("IceB WatchP #1 Data mask  = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_1_DATAMASK));
		val = jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_1_CONTROL);
		printf ("IceB WatchP #1 Control    = 0x%08X\t", val);
		if(val & 0x100uL) printf("ENABLE ");
		if(val & 0x080uL) printf("RANGE ");
		if(val & 0x040uL) printf("CHAIN ");
		if(val & 0x020uL) printf("EXTERN ");
		if(val & 0x010uL) printf("/TRANS=1(priveleg)"); else  printf("/TRANS=1(user mode)");
		if(val & 0x008uL) printf("/OPC=1(data)"); else  printf("/OPC=0(instr)");
		if(val & 0x004uL) printf("MAS[1] ");
		if(val & 0x002uL) printf("MAS[0] ");
		if(val & 0x001uL) printf("write\n"); else printf("read\n");
		val = jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_1_CONTROLMASK);
		printf ("IceB WatchP #1 Contr.mask = 0x%08X\t", val);
		if((val & 0x080uL) == 0) printf("RANGE ");
		if((val & 0x040uL) == 0) printf("CHAIN ");
		if((val & 0x020uL) == 0) printf("EXTERN ");
		if((val & 0x010uL) == 0) printf("/TRANS ");
		if((val & 0x008uL) == 0) printf("/OPC "); 
		if((val & 0x004uL) == 0) printf("MAS[1] ");
		if((val & 0x002uL) == 0) printf("MAS[0] ");
		if((val & 0x001uL) == 0) printf("read/write\n"); else printf("\n");
	
		printf ("\n");
	}
	return ;
}

/*
 * Poll debug state
 */
unsigned int jtag_arm_PollDbgState(void)
{
	unsigned int ret_val;
	int reg = 0;
	
	dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"PollDbgState:\n");
	if(ice_state.is_debugrequest & 1)
	{
		dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"CPU still in debug state\n");
		ret_val = 1;
		return ret_val;
	}
	//jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_STATUS, 0x1F); 
	//jtag_eos();
	//jtag_arm_IceRT_RegRead(ICERT_REG_ABORT_STATUS);
	
	ret_val = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);
	//if(ret_val & 1)
	//	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"<%x> ",ret_val);

	if((ret_val & 0x09) == 0x09) // 0x01
	{
		reg |= (1<<0);			// DBGACK:signal that we are in debug state even on system speed access
		reg |= (1<<2);			// disable Intr
		jtag_arm_IceRT_RegWrite( ICERT_REG_DEBUG_CONTROL, reg);
		
		dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"CPU has entered debug state\n");
		ice_state.is_debugrequest |= 1;
		ret_val = 1;
	}
	else
	{
#if 0		
		/*make sure there is no message via DCC control*/
		ret_val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
		if(ret_val & 0x2) // read out data from target
		{
			ret_val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_DATA);
			dbgPrintf(DBG_LEVEL_JTAG_ARM,"unexpected message from target 0x%08X\n",ret_val);
		}
		ret_val = jtag_arm_IceRT_RegRead(ICERT_REG_ABORT_STATUS);
		if(ret_val)
		dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"abort state\n");
		else
#endif
		ret_val = 0;
	}

	return ret_val;  /* Return 32 bits readout */
}

/*
 *
 */
int jtag_arm_StopRunningProgram(void)
{
	jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, 0x02);              /* INTDIS=0 DBGRQ=1 DBGACK=0*/
	
	/* After sending data, RUN-TEST/IDLE state must be entered: */
	jtag_eos();                  /* goto Run-Test/Idle          */
	
	/* Put a flag to remember later that the debug state was entered because
	   WE requested it directly (not thru a breakpoint/watchpoint): */
	ice_state.is_debugrequest |= 2;
      	return 0;
}

/*
 *
 */
int jtag_arm_IceRT_version(void)
{
	int val;
	
	val = jtag_arm_IceRT_RegRead(ICERT_REG_DCC_CONTROL);
	val = (val>>28) & 0x0f;
	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Embedded ICE version %d\n",val);
	arm_info.ice_revision = val;
	return val;
}

/* 
 * Setup the ICE-RT to generate a breakpoint signal on any instruction address 
 */
void jtag_arm_PutAnyBreakPoint(void)
{
	int reg = 0;
	int stat;

	stat = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);

	// check if CPU is already in debug state
	if((stat&0x09)!= 0x09)
	{
		//reg = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_CONTROL);	// read Debug control register
		reg |= (1<<1);				// DBGRQ
		reg |= (1<<2);				// disable Intr
		jtag_arm_IceRT_RegWrite( ICERT_REG_DEBUG_CONTROL, reg);
		jtag_eos();

		// make sure we enter DEBUG State
		do {
			stat = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);
		} while ((stat&0x09)!= 0x09);
		ice_state.is_debugrequest |= 3;
		dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"CPU entering debug state\n");
	}
	
	reg = 0;
	reg |= (1<<0);			// DBGACK:signal that we are in debug state even on system speed access
	reg |= (1<<2);			// disable Intr
	
	jtag_arm_IceRT_RegWrite( ICERT_REG_DEBUG_CONTROL, reg);
	
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_CONTROL, 0x000);		/* WP1 contr value DISABLE */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROL, 0x000);		/* WP0 contr value DISABLE */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRESS, 0x0);		/* WP0 Addr value */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRMASK, 0xfffffffe);	/* WP0 Addr mask=ignore except bit 0*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_DATAMASK, 0xffffffff);	/* WP0 data mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROLMASK, 0x0f7);	/* WP0 contr mask=nOPC only*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROL, 0x100);		/* WP0 contr value Enable=1 nOPC=0*/
	
	/* Flag to know if the debug state was entered because of a BRKP/WATCHP or a direct debug request: */
	ice_state.watchpoint0 = 0;	// free = 0
	ice_state.watchpoint1 = 0;	// free = 0

	return ;
}

/*
 *
 */
void jtag_arm_PutHWBreakPoint0(uint32_t addr)
{
	if(ice_state.watchpoint0 != 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Watchpoint 0 already in use\ncan't insert HW Breakpoint at 0x%8.8X\n",addr);
		return;
	}
	
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRESS, addr);		/* WP0 Addr value */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRMASK, 0x00000001);	/* WP0 Addr mask=all except bit 0*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_DATAMASK, 0xffffffff);	/* WP0 data mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROLMASK, 0x0f7);	/* WP0 contr mask=nOPC only*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROL, 0x100);		/* WP0 contr value Enable=1 nOPC=0*/

	/* Flag to know if the debug state was entered because of a BRKP/WATCHP or a direct debug request: */
	ice_state.watchpoint0 = 1;	// HW = 1

	return ;
}

/*
 *
 */
void jtag_arm_PutHWBreakPoint1(uint32_t addr)
{
	if(ice_state.watchpoint1 != 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Watchpoint 1 already in use\ncan't insert HW Breakpoint at 0x%8.8X\n",addr);
		return;
	}
	
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_ADDRESS, addr);		/* WP1 Addr value */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_ADDRMASK, 0x00000001);	/* WP1 Addr mask=all except bit 0*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_DATAMASK, 0xffffffff);	/* WP1 data mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_CONTROLMASK, 0x0f7);	/* WP1 contr mask=nOPC only*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_CONTROL, 0x100);		/* WP1 contr value Enable=1 nOPC=0*/

	/* Flag to know if the debug state was entered because of a BRKP/WATCHP or a direct debug request: */
	ice_state.watchpoint1 = 1;	// HW = 1

	return ;
}

/*
 *
 */
void jtag_arm_PutSWBreakPoint0(void)
{
	if(ice_state.watchpoint0 != 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Watchpoint 0 already in use\ncan't insert SW Breakpoint\n");
		return;
	}
	
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRESS, 0xffffffff);	/* WP0 Addr mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_DATA, 0xB710B710);	/* WP0 data value=ARM BKPT */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_DATAMASK, 0x00000000);	/* WP0 data mask=fit */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROLMASK, 0x0f7);	/* WP0 contr mask=nOPC */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROL, 0x100);		/* WP0 contr value Enable=1 nOPC=0*/

	/* Flag to know if the debug state was entered because of a BRKP/WATCHP or a direct debug request: */
	ice_state.watchpoint0 = 2;	//  SW = 2

	return ;
}

/*
 *
 */
void jtag_arm_PutSWBreakPoint1(void)
{
	if(ice_state.watchpoint1 != 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_WARN,"Watchpoint 1 already in use\ncan't insert SW Breakpoint\n");
		return;
	}
	
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_ADDRMASK, 0xffffffff);	/* WP1 Addr mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_DATA, 0xB710B710);	/* WP1 data value=ARM BKPT */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_DATAMASK, 0x00000000);	/* WP1 data mask=fit */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_CONTROLMASK, 0x0f7);	/* WP1 contr mask=nOPC */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_CONTROL, 0x100);		/* WP1 contr value Enable=1 nOPC=0 */

	/* Flag to know if the debug state was entered because of a BRKP/WATCHP or a direct debug request: */
	ice_state.watchpoint1 = 2;	// SW = 2

	return ;
}


/*  
 *  Disable Interrupt using ICE-RT 
 */
void jtag_arm_disable_Intr(void)
{
	int reg;
	
	reg = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_CONTROL);	// read Debug control register
	reg |= (1<<2);				// disable Intr
	jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, reg);
	return;
}

/*  
 *  Enable Interrupt using ICE-RT 
 */
void jtag_arm_enable_Intr(void)
{
	int reg;
	
	reg = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_CONTROL);	// read Debug control register
	reg &= ~(1<<2uL);			// enable Intr
	jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, reg);
	return;
}

/* 
 * Disable any enabled breakpoint 
 */
void jtag_arm_ClearAnyBreakPoint(void)
{
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROL, 0x000);	// WP0 contr value ENABLE=0 nOPC=0
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_1_CONTROL, 0x000);	// WP1 contr value ENABLE=0 nOPC=0
	
	ice_state.is_debugrequest &= ~1uL;
	ice_state.watchpoint0 = 0;	// free = 0
	ice_state.watchpoint1 = 0;	// free = 0
	return ;
}


/* 
 * Disable any enabled breakpoint and enter monitor mode
 */
void jtag_arm_enterMonitorMode(void)
{
	int reg;
	
	reg = 0x10;
	jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, reg);
	ice_state.is_debugrequest = 4;
	dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"CPU has entered monitor state\n");
	return ;
}

/*
 * Leave Monitor mode and enter debug mode by puting the any Breakpoint
 */
void jtag_arm_Mointor2DebugMode(void)
{
	int reg = 0;
	int stat;

	stat = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);

	// check if CPU is already in debug state
	if((stat&0x09)!= 0x09)
	{
		reg |= (1<<1);				// DBGRQ
		reg |= (1<<2);				// disable Intr
		//reg |= (1<<5);			// disable ICE

		jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, reg);
		jtag_eos();

		// make sure we enter DEBUG State
		do {
			stat = jtag_arm_IceRT_RegRead(ICERT_REG_DEBUG_STATUS);
		} while ((stat&0x09)!= 0x09);
		ice_state.is_debugrequest |= 3;
		dbgPrintf(DBG_LEVEL_JTAG_ICERT_LOW,"CPU has entered debug state\n");
	}
	reg = 0;
	reg |= (1<<0);			// DBGACK:signal that we are in debug state even on system speed access
	reg |= (1<<2);			// disable Intr
	jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, reg);
	ice_state.is_debugrequest |= 1;
	ice_state.is_debugrequest &= ~4uL;
	
	//jtag_arm_IceRT_RegWrite(ICERT_REG_DEBUG_CONTROL, 0x20);	
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRMASK, 0xffffffff);	/* WP0 Addr mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_DATAMASK, 0xffffffff);	/* WP0 data mask=ignore */
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROLMASK, 0x0f7);	/* WP0 contr mask=nOPC only*/
	jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_CONTROL, 0x100);		/* WP0 contr value Enable=1 nOPC=0*/
	
	/* Flag to know if the debug state was entered because of a BRKP/WATCHP or a direct debug request: */
	ice_state.watchpoint0 = 0;	// free = 0
	ice_state.watchpoint1 = 0;	// free = 0

	return ;
}


