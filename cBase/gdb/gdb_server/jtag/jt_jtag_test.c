/*
 * jt_jtag_test 
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


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "dbg_msg.h"
#include "jt_tap.h"
#include "jt_arm.h"
#include "jt_flash.h"

#include "jt_test_pattern.h"


//#define	SRAM_BASE 0x0c000000 //s3c44b0x (sdram)
//#define	SRAM_BASE 0x10001000 //s3c44b0x (internal ram -- if cache disabled)
//#define	SRAM_BASE 0x40000000 // lpc2106
//#define	SRAM_BASE 0x00200000 // sam7 sram
uint32_t ramBase = 0;
int RAM_BASE_def = 0;

uint32_t flashBase = 0;
int FLASH_BASE_def = 0;

static int word_pattern[64];


/*
 *
 */
int jtag_test(uint32_t level)
{
	int i;
	unsigned int addr;
	unsigned short wdata;

	tap_start();
	if(jtag_arm_verify() == 0)
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Unknown JTAG Device\n");
		goto terminate;
	}
	//tap_idle();
	
	level &= ~TEST_JTAG_GETID; // we have seen the ID now remove Flag
	
	if(level & TEST_JTAG_MODIFY_ICE) // ICE modify
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"modify ICE Regs - Test\n");

		dbgPrintf (DBG_LEVEL_GDB_ARM_INFO,"IceB WatchP #0 Addr.mask  = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_ADDRMASK));
		jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRMASK,  0xdeadffff);
		dbgPrintf (DBG_LEVEL_GDB_ARM_INFO,"IceB WatchP #0 Addr.mask  = 0x%08X\n", jtag_arm_IceRT_RegRead(ICERT_REG_WATCHPOINT_0_ADDRMASK));
		jtag_arm_IceRT_RegWrite(ICERT_REG_WATCHPOINT_0_ADDRMASK,  0);
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Show ICE Regs\n");
		jtag_arm_ShowAllIceRT_Regs();
	}
	
	if(level) // common -
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"set halt on Any Breakpoint\n");
		jtag_arm_PutAnyBreakPoint();
		while(jtag_arm_PollDbgState() == 0)
			;
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Show ICE Regs\n");
		jtag_arm_ShowAllIceRT_Regs();
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register\n");
		jtag_arm_ReadCpuRegs(0);
		jtag_arm_DumpCPUregs();
#if 0
		jtag_arm_ReadCP15();
#endif
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register -- again now forced to be ARM\n");
		jtag_arm_ReadCpuRegs(1);
		jtag_arm_DumpCPUregs();
	}
	
	if(FLASH_BASE_def && (level & TEST_JTAG_FLASH_READ)) // Flash info
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"get Intel or AMD Flash Info\n");
		jt_intelflashGetInfoHalfword(flashBase, NULL);
		jt_amdflashGetInfoHalfword( flashBase, NULL);
	}

	if(level & TEST_JTAG_MODIFY_CPU_REG) // Write CPU Regs
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Write CPU Regs\n");
		for(i=15; i>=0;i--)
			CPU.regs.r[i] = 15 - i;
		jtag_arm_WriteCpuRegs();

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register -- again\n");
		jtag_arm_ReadCpuRegs(1);
		jtag_arm_DumpCPUregs();
	}
	
	if(FLASH_BASE_def && (level & TEST_JTAG_FLASH_READ)) // Flash info
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Data at Flash\n");
		jtag_arm_ReadWordMemory(flashBase, 64, NULL );
		addr = flashBase;
		for(; addr<(4*32); addr+=4)
		{
			i = jtag_arm_ReadWord( addr );
			printf("0x%8.8X : 0x%8.8X \n",addr,i);
		}
	}
	
	if(RAM_BASE_def && (level & TEST_JTAG_RAM_WRITE)) // RAM read/write test
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Data at RAM\n");
		addr = ramBase;
		for(; addr<(ramBase + 4*32); addr+=4)
		{
			i = jtag_arm_ReadWord( addr );
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X : 0x%8.8X \n",addr,i);
		}
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Write Data to RAM (4 Word pattern and 2 Halfword pattern)\n");
		addr = ramBase;
		for(; addr<(ramBase + 4*20); )
		{
			jtag_arm_WriteWord(addr,0x0000FFFF);addr+=4;
			jtag_arm_WriteWord(addr,0xFFFF0000);addr+=4;
			jtag_arm_WriteWord(addr,0xAAAA5555);addr+=4;
			jtag_arm_WriteWord(addr,0xdeadbeaf);addr+=4;
			jtag_arm_WriteHalfword(addr,0x1111);addr+=2;
			jtag_arm_WriteHalfword(addr,0x5555);addr+=2;
		}
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Word Data at RAM\n");
		addr = ramBase;
		for(; addr<(ramBase + 4*20); addr+=4)
		{
			i = jtag_arm_ReadWord( addr );
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X : 0x%8.8X \n",addr,i);
		}

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Halfword Data at RAM\n");
		addr = ramBase;
		for(; addr<(ramBase + 4*32); addr+=2)
		{
			i = jtag_arm_ReadHalfword( addr );
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X : 0x%8.8X \n",addr,i);
		}
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Data (Buffer) at RAM\n");
		jtag_arm_ReadWordMemory(ramBase, 64, NULL );
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Write Pattern (Buffer) to RAM\n");
		for(i=63;i>=0;i--)
			word_pattern[i] = 64 - i;
		jtag_arm_WriteMemoryBuf(ramBase,64, (uint32_t *)word_pattern);
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Pattern (Buffer) at RAM\n");
		jtag_arm_ReadWordMemory(ramBase, 64, NULL );
	}
	
	if(RAM_BASE_def && (level & TEST_JTAG_RAM_PROGRAM)) // RAM prog test
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Put Testprog at SRAM\n");
		jtag_arm_WriteMemoryBuf(ramBase, sizeof(text_buf) / 4, (uint32_t *)text_buf);
		addr = ramBase;
		for(; addr<(ramBase + sizeof(text_buf)); addr+=4)
		{
			i = jtag_arm_ReadWord( addr );
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X : 0x%8.8X \n",addr,i);
		}
		/*set ltorg to thumb start*/
		jtag_arm_WriteWord(ramBase + 0x150, ramBase + 0x154);
		jtag_arm_ReadWordMemory(ramBase, 24, NULL );
		
		for(i=0; i<15;i++)
			CPU.regs.r[i] = i;
	
		CPU.regs.r[15] = ramBase + 0x54;
		CPU.CPSR = 0xd3;
		CPU.CPSR &= ~0x20L; // switch into ARM
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Run @ 0x%X\n (b .)",CPU.regs.r[15]);
		
		jtag_arm_PrepareExitDebug();
		jtag_arm_ClearAnyBreakPoint();
		jtag_arm_FinalExitDebug();
		
		for(i=0;jtag_arm_PollDbgState() != 0 && i<25;i++)
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"not yet started\n");

		if(i>24)
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"give up\n");
		else
		{
			usleep(1000); // wait 1 msec
			//printf("Press key to stop\n");getc(stdin);
		}
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"set halt on Any Breakpoint\n");
		jtag_arm_PutAnyBreakPoint();
		while(jtag_arm_PollDbgState() == 0)
			;

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Show ICE Regs\n");
		jtag_arm_ShowAllIceRT_Regs();
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register\n");
		jtag_arm_ReadCpuRegs(1);
		jtag_arm_DumpCPUregs();

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Data (Buffer) at RAM\n");
		jtag_arm_ReadWordMemory(ramBase, 24, NULL );

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Do some singel ARM steps ");
		
		//CPU.regs.r[15] = ramBase + 0x54;	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(do -> b .)\n");
		CPU.regs.r[15] = ramBase + 0x90;	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(nop's 0x8.8%X till\n", ramBase+0xa4);

		for(wdata=0; wdata<16; wdata++)
		{
			i = jtag_arm_ReadWord(CPU.regs.r[15]);
			printf("0x%X\n",i);
			jtag_arm_Step(i);
			for(i=0;jtag_arm_PollDbgState() == 0 && i<25;i++)
				dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"not yet finished\n");
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register\n");
			jtag_arm_ReadCpuRegs(1);
			jtag_arm_DumpCPUregs();
		}
		
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO
			,"instr at 0x%X -> 0x%X\n"
			,ramBase + 0x17e
			,jtag_arm_ReadHalfword( ramBase + 0x17e) );
		
		for(i=0; i<15;i++)
			CPU.regs.r[i] = i;
		
		CPU.regs.r[15] = ramBase + 0x17e;	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"run THUMB b .");
		//CPU.regs.r[15] = ramBase + 0x180;	dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"run THUMB nop's till RAM_BASE + 0x1A8");
		CPU.CPSR |= 0x20L; // switch into THUMB
		
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"\t @ 0x%X\n",CPU.regs.r[15]);
		
		jtag_arm_PrepareExitDebug();
		jtag_arm_ClearAnyBreakPoint();
		jtag_arm_FinalExitDebug();

		for(i=0;jtag_arm_PollDbgState() != 0 && i<25;i++)
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"not yet started\n");

		if(i>24)
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"give up\n");
		else
		{
			usleep(1000); // wait 1 msec
			//printf("Press key to stop\n");getc(stdin);
		}

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"set halt on Any Breakpoint\n");
		jtag_arm_PutAnyBreakPoint();
		while(jtag_arm_PollDbgState() == 0)
			;
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Show ICE Regs\n");
		jtag_arm_ShowAllIceRT_Regs();
	
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register\n");
		jtag_arm_ReadCpuRegs(1);
		jtag_arm_DumpCPUregs();
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"instr at PC 0x%X\n",jtag_arm_ReadHalfword(CPU.regs.r[15]) );
		
		if(FLASH_BASE_def && (level & TEST_JTAG_FLASH_READ)) // Flash info
		{
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Flash Info (after \"running\" programm)\n");
			jt_intelflashGetInfoHalfword(flashBase, NULL);
			jt_amdflashGetInfoHalfword(flashBase, NULL);
		}

		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Do some singel THUMB steps\n");
		CPU.regs.r[15] = ramBase + 0x194;	
		for(i=0; i<16; i++)
		{
			wdata = jtag_arm_ReadHalfword(CPU.regs.r[15]);
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%X\n",wdata);
			jtag_arm_Step(wdata);
			
			while(jtag_arm_PollDbgState() == 0)
				;
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register\n");
			jtag_arm_ReadCpuRegs(1);
			jtag_arm_DumpCPUregs();
		}
	}
	
	if(RAM_BASE_def && (level & TEST_JTAG_RAM_READ)) // read RAM
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Data at RAM\n");
		addr = ramBase;
		for(; addr<(ramBase + 4*32); addr+=4)
		{
			i = jtag_arm_ReadWord( addr );
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X : 0x%8.8X \n",addr,i);
		}
	}
	
	if(FLASH_BASE_def && (level & TEST_JTAG_FLASH_READ)) // read Flash
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Flash Info (again)\n");
		jt_intelflashGetInfoHalfword(flashBase, NULL);
		jt_amdflashGetInfoHalfword(flashBase, NULL);
		
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read Data at Flash\n");
		addr = 0;
		for(; addr<(4*32); addr+=4)
		{
			i = jtag_arm_ReadWord( addr );
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"0x%8.8X : 0x%8.8X \n",addr,i);
		}
	}

	if(FLASH_BASE_def && (level & TEST_JTAG_START_STEPS)) // step 15
	{
		dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Steping 15 instr from Start\n");
		CPU.CPSR = 0xd3;
		CPU.CPSR &= ~0x20L; // switch into ARM
		CPU.regs.r[15] = 0x0;	
		for(i=0; i<16; i++)
		{
			addr = jtag_arm_ReadWord(CPU.regs.r[15]);
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"(0x%8.8X) instr 0x%8.8X ->",CPU.regs.r[15],addr);
			jtag_arm_Step(addr);
			while(jtag_arm_PollDbgState() == 0)
				;
			dbgPrintf(DBG_LEVEL_GDB_ARM_INFO,"Read CPU Register\n");
			jtag_arm_ReadCpuRegs(1);
			jtag_arm_DumpCPUregs();
		}
	}
	
	if(FLASH_BASE_def && (level & TEST_JTAG_RELEASE)) // release
	{
		jtag_arm_PrepareExitDebug();
		jtag_arm_ClearAnyBreakPoint();
		jtag_arm_FinalExitDebug();
	}

terminate:
	tap_stop();
	return 0;
}

